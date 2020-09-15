// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../../common/export_util.h"
#include "../../iterator/DefaultIterator.h"
#include "../../iterator/Iterator.h"
#include "../../sampler/Sampler.h"
#include "AMRVolume.h"
#include "AMRVolume_ispc.h"
#include "Sampler_ispc.h"
#include "Volume_ispc.h"

namespace openvkl {
  namespace ispc_driver {

    template <int W>
    using AMRIntervalIteratorFactory =
        ConcreteIteratorFactory<W, IntervalIterator, DefaultIntervalIterator>;

    template <int W>
    using AMRHitIterator = DefaultHitIterator<W, DefaultIntervalIterator<W>>;

    template <int W>
    using AMRHitIteratorFactory =
        ConcreteIteratorFactory<W, HitIterator, AMRHitIterator>;

    template <int W>
    using AMRSamplerBase = SamplerBase<W,
                                       AMRVolume,
                                       AMRIntervalIteratorFactory,
                                       AMRHitIteratorFactory>;

    template <int W>
    struct AMRSampler : public AMRSamplerBase<W>
    {
      AMRSampler(AMRVolume<W> *volume);
      ~AMRSampler() override;

      void commit() override {}

      void computeSampleV(const vintn<W> &valid,
                          const vvec3fn<W> &objectCoordinates,
                          vfloatn<W> &samples,
                          unsigned int attributeIndex) const override final;

      void computeSampleN(unsigned int N,
                          const vvec3fn<1> *objectCoordinates,
                          float *samples,
                          unsigned int attributeIndex) const override final;

      void computeGradientV(const vintn<W> &valid,
                            const vvec3fn<W> &objectCoordinates,
                            vvec3fn<W> &gradients,
                            unsigned int attributeIndex) const override final;

      void computeGradientN(unsigned int N,
                            const vvec3fn<1> *objectCoordinates,
                            vvec3fn<1> *gradients,
                            unsigned int attributeIndex) const override final;

     protected:
      using Sampler<W>::ispcEquivalent;
      using AMRSamplerBase<W>::volume;
    };

    // Inlined definitions ////////////////////////////////////////////////////

    template <int W>
    inline AMRSampler<W>::AMRSampler(AMRVolume<W> *volume)
        : AMRSamplerBase<W>(*volume)
    {
      assert(volume);
      ispcEquivalent =
          CALL_ISPC(AMRSampler_create, volume->getISPCEquivalent());
    }

    template <int W>
    inline AMRSampler<W>::~AMRSampler()
    {
      CALL_ISPC(AMRSampler_destroy, ispcEquivalent);
      ispcEquivalent = nullptr;
    }

    template <int W>
    inline void AMRSampler<W>::computeSampleV(
        const vintn<W> &valid,
        const vvec3fn<W> &objectCoordinates,
        vfloatn<W> &samples,
        unsigned int attributeIndex) const
    {
      assert(attributeIndex < volume->getNumAttributes());
      CALL_ISPC(AMRVolume_sample_export,
                static_cast<const int *>(valid),
                ispcEquivalent,
                &objectCoordinates,
                &samples);
    }

    template <int W>
    inline void AMRSampler<W>::computeSampleN(
        unsigned int N,
        const vvec3fn<1> *objectCoordinates,
        float *samples,
        unsigned int attributeIndex) const
    {
      assert(attributeIndex < volume->getNumAttributes());
      CALL_ISPC(Sampler_sample_N_export,
                ispcEquivalent,
                N,
                (ispc::vec3f *)objectCoordinates,
                samples);
    }

    template <int W>
    inline void AMRSampler<W>::computeGradientV(
        const vintn<W> &valid,
        const vvec3fn<W> &objectCoordinates,
        vvec3fn<W> &gradients,
        unsigned int attributeIndex) const
    {
      assert(attributeIndex < volume->getNumAttributes());
      CALL_ISPC(AMRVolume_gradient_export,
                static_cast<const int *>(valid),
                ispcEquivalent,
                &objectCoordinates,
                &gradients);
    }

    template <int W>
    inline void AMRSampler<W>::computeGradientN(
        unsigned int N,
        const vvec3fn<1> *objectCoordinates,
        vvec3fn<1> *gradients,
        unsigned int attributeIndex) const
    {
      assert(attributeIndex < volume->getNumAttributes());
      CALL_ISPC(Sampler_gradient_N_export,
                ispcEquivalent,
                N,
                (ispc::vec3f *)objectCoordinates,
                (ispc::vec3f *)gradients);
    }

  }  // namespace ispc_driver
}  // namespace openvkl
