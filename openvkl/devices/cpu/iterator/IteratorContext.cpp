// Copyright 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "IteratorContext.h"
#include "../common/Data.h"
#include "../common/export_util.h"
#include "../sampler/Sampler.h"
#include "../volume/Volume.h"
#include "../volume/vdb/VdbVolume.h"
#include "../volume/vdb/DenseVdbVolume.h"
#include "IteratorContext_ispc.h"
#include "rkcommon/math/range.h"
#include "../volume/UnstructuredVolume.h"
#include "../volume/amr/AMRVolume.h"
#include "../volume/particle/ParticleVolume.h"

namespace openvkl {
  namespace cpu_device {

    ///////////////////////////////////////////////////////////////////////////
    // Helpers ////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    // this function is somewhat expensive, but only occurs on commit. also,
    // generating the map on on commit handles cases where the volume may change
    // or be committed again after construction of the context.
    template <int W>
    inline int mapToMaxIteratorDepth(IteratorContext<W> &iteratorContext,
                                     const float intervalResolutionHint)
    {
      // each pair in this vector maps a intervalResolutionHint value to a
      // maxIteratorDepth value. the mapping depends on the volume type.
      std::vector<std::pair<float, int>> hintToDepth;

      const Volume<W> &volume = iteratorContext.getSampler().getVolume();

      if (dynamic_cast<const VdbVolume<W> *>(&volume)) {
        // handles both DenseVdbVolume (structuredRegular) and VdbVolume

        hintToDepth.emplace_back(0.f, 0);
        hintToDepth.emplace_back(0.2f, 1);
        hintToDepth.emplace_back(0.4f, 2);

        // max iterator depth is VKL_VDB_NUM_LEVELS - 1
        assert(VKL_VDB_NUM_LEVELS == 4);
        hintToDepth.emplace_back(0.8f, 3);

      } else if (dynamic_cast<const AMRVolume<W> *>(&volume) ||
                 dynamic_cast<const ParticleVolume<W> *>(&volume) ||
                 dynamic_cast<const UnstructuredVolume<W> *>(&volume)) {
        // these volume types all use a BVH-based iterator

        // we should have these volume types inherit from a common base
        int bvhDepth = 0;

        if (dynamic_cast<const AMRVolume<W> *>(&volume)) {
          const auto *v = dynamic_cast<const AMRVolume<W> *>(&volume);
          bvhDepth      = v->getBvhDepth();
        } else if (dynamic_cast<const ParticleVolume<W> *>(&volume)) {
          const auto *v = dynamic_cast<const ParticleVolume<W> *>(&volume);
          bvhDepth      = v->getBvhDepth();
        } else if (dynamic_cast<const UnstructuredVolume<W> *>(&volume)) {
          const auto *v = dynamic_cast<const UnstructuredVolume<W> *>(&volume);
          bvhDepth      = v->getBvhDepth();
        } else {
          throw std::runtime_error(
              "could not map intervalResolutionHint value");
        }

        int defaultDepth              = bvhDepth > 6 ? 6 : bvhDepth / 2;
        const float defaultRangeBegin = 0.45f;
        const float defaultRangeEnd   = 0.55f;

        // mapping defined for intervalResolutionHint in [0, defaultRangeBegin]
        if (defaultDepth == 0) {
          hintToDepth.emplace_back(0.f, 0);
        } else {
          for (int i = 0; i <= defaultDepth; i++) {
            float f = float(i) / float(defaultDepth) * defaultRangeBegin;
            hintToDepth.emplace_back(f, i);
          }
        }

        // mapping defined for intervalResolutionHint in [defaultRangeEnd, 1)
        for (int i = defaultDepth + 1; i <= bvhDepth - 1; i++) {
          float f = defaultRangeEnd + float(i - (defaultDepth + 1)) /
                                          float(bvhDepth - (defaultDepth + 1)) *
                                          (1.f - defaultRangeEnd);

          hintToDepth.emplace_back(f, i);
        }

        // mapping defined for intervalResolutionHint == 1
        hintToDepth.emplace_back(1.f, bvhDepth);
      }

      else {
        // volume type does not support maxIteratorDepth parameter (result will
        // go unused)
        return 0;
      }

      // sanity check populated hintToDepth map
      if (!hintToDepth.size() || hintToDepth.front().first != 0.f ||
          hintToDepth.back().first > 1.f) {
        throw std::runtime_error("could not map intervalResolutionHint value");
      }

      // find the mapped value
      int maxIteratorDepth = -1;

      for (const auto &m : hintToDepth) {
        // selects value for the [i, i+1) interval
        if (intervalResolutionHint >= m.first) {
          maxIteratorDepth = m.second;
        } else {
          break;
        }
      }

      if (maxIteratorDepth == -1) {
        throw std::runtime_error("could not map intervalResolutionHint value");
      }

      return maxIteratorDepth;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Interval iterator context //////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    template <int W>
    IntervalIteratorContext<W>::~IntervalIteratorContext()
    {
      if (this->ispcEquivalent) {
        CALL_ISPC(IntervalIteratorContext_Destructor, this->ispcEquivalent);
        this->ispcEquivalent = nullptr;
      }
    }

    template <int W>
    void IntervalIteratorContext<W>::commit()
    {
      // attribute index
      this->attributeIndex = this->template getParam<int>("attributeIndex", 0);

      throwOnIllegalAttributeIndex(&this->getSampler().getVolume(),
                                   this->attributeIndex);

      // value ranges
      Ref<const DataT<box1f>> valueRangesData =
          this->template getParamDataT<box1f>("valueRanges", nullptr);

      std::vector<range1f> valueRanges;

      if (valueRangesData) {
        for (const auto &r : *valueRangesData) {
          valueRanges.push_back(r);
        }
      }

      // interval resolution hint
      float intervalResolutionHint =
          this->template getParam<float>("intervalResolutionHint", 0.5f);

      // clamp to range [0, 1]
      intervalResolutionHint =
          std::max(std::min(1.f, intervalResolutionHint), 0.f);

      // map interval resolution hint into internally used values
      const int maxIteratorDepth =
          mapToMaxIteratorDepth(*this, intervalResolutionHint);
      const bool elementaryCellIteration = (intervalResolutionHint == 1.f);

      if (this->ispcEquivalent) {
        CALL_ISPC(IntervalIteratorContext_Destructor, this->ispcEquivalent);
      }

      this->ispcEquivalent = CALL_ISPC(IntervalIteratorContext_Constructor,
                                       this->getSampler().getISPCEquivalent(),
                                       this->attributeIndex,
                                       valueRanges.size(),
                                       (const ispc::box1f *)valueRanges.data(),
                                       maxIteratorDepth,
                                       elementaryCellIteration);
    }

    template struct IntervalIteratorContext<VKL_TARGET_WIDTH>;

    ///////////////////////////////////////////////////////////////////////////
    // Hit iterator context ///////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    template <int W>
    HitIteratorContext<W>::~HitIteratorContext()
    {
      if (this->ispcEquivalent) {
        CALL_ISPC(HitIteratorContext_Destructor, this->ispcEquivalent);
        this->ispcEquivalent = nullptr;
      }
    }

    template <int W>
    void HitIteratorContext<W>::commit()
    {
      this->attributeIndex = this->template getParam<int>("attributeIndex", 0);

      throwOnIllegalAttributeIndex(&this->getSampler().getVolume(),
                                   this->attributeIndex);

      Ref<const DataT<float>> valuesData =
          this->template getParamDataT<float>("values", nullptr);

      std::vector<float> values;

      if (valuesData) {
        for (const auto &r : *valuesData) {
          values.push_back(r);
        }
      }

      // default interval iterator depth used for hit iteration
      int maxIteratorDepth;

      const Volume<W> &volume = this->getSampler().getVolume();

      if (dynamic_cast<const VdbVolume<W> *>(&volume) &&
          !dynamic_cast<const DenseVdbVolume<W> *>(&volume)) {
        // VdbVolume, but not DenseVdbVolume.
        // For sparse VDB volumes (constant cell data), we use elementary cell
        // iteration to avoid hit artifacts near boundaries.
        maxIteratorDepth = mapToMaxIteratorDepth(*this, 1.f);
      } else {
        maxIteratorDepth = mapToMaxIteratorDepth(*this, 0.5f);
      }

      if (this->ispcEquivalent) {
        CALL_ISPC(HitIteratorContext_Destructor, this->ispcEquivalent);
      }

      this->ispcEquivalent = CALL_ISPC(HitIteratorContext_Constructor,
                                       this->getSampler().getISPCEquivalent(),
                                       this->attributeIndex,
                                       values.size(),
                                       (const float *)values.data(),
                                       maxIteratorDepth);
    }

    template struct HitIteratorContext<VKL_TARGET_WIDTH>;

  }  // namespace cpu_device
}  // namespace openvkl
