// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <algorithm>
#include "../common/export_util.h"
#include "../iterator/UnstructuredIterator.h"
#include "../sampler/Sampler.h"
#include "Sampler_ispc.h"
#include "UnstructuredVolume.h"
#include "UnstructuredVolume_ispc.h"
#include "Volume_ispc.h"

namespace openvkl {
  namespace cpu_device {

    template <int W>
    using UnstructuredSamplerBase = SamplerBase<W,
                             UnstructuredVolume,
                             UnstructuredIntervalIteratorFactory,
                             UnstructuredHitIteratorFactory>;

    template <int W>
    struct UnstructuredSampler
        : public UnstructuredSamplerBase<W>
    {
      UnstructuredSampler(UnstructuredVolume<W> *volume);
      ~UnstructuredSampler() override;

      void computeSampleV(const vintn<W> &valid,
                          const vvec3fn<W> &objectCoordinates,
                          vfloatn<W> &samples,
                          unsigned int attributeIndex,
                          const vfloatn<W> &time) const override final;

      void computeSampleN(unsigned int N,
                          const vvec3fn<1> *objectCoordinates,
                          float *samples,
                          unsigned int attributeIndex,
                          const float *times) const override final;

      void computeGradientV(const vintn<W> &valid,
                            const vvec3fn<W> &objectCoordinates,
                            vvec3fn<W> &gradients,
                            unsigned int attributeIndex,
                            const vfloatn<W> &time) const override final;

      void computeGradientN(unsigned int N,
                            const vvec3fn<1> *objectCoordinates,
                            vvec3fn<1> *gradients,
                            unsigned int attributeIndex,
                            const float *times) const override final;

     private:
      using Sampler<W>::ispcEquivalent;
      using UnstructuredSamplerBase<W>::volume;
    };

    // Inlined definitions ////////////////////////////////////////////////////

    template <int W>
    inline UnstructuredSampler<W>::UnstructuredSampler(
        UnstructuredVolume<W> *volume)
        : UnstructuredSamplerBase<W>(*volume)
    {
      assert(volume);
      ispcEquivalent = CALL_ISPC(VKLUnstructuredSampler_Constructor,
                                 volume->getISPCEquivalent());
    }

    template <int W>
    inline UnstructuredSampler<W>::~UnstructuredSampler()
    {
      CALL_ISPC(VKLUnstructuredSampler_Destructor, ispcEquivalent);
      ispcEquivalent = nullptr;
    }

    template <int W>
    inline void UnstructuredSampler<W>::computeSampleV(
        const vintn<W> &valid,
        const vvec3fn<W> &objectCoordinates,
        vfloatn<W> &samples,
        unsigned int attributeIndex,
        const vfloatn<W> &time) const
    {
      assert(attributeIndex < volume->getNumAttributes());
      assertValidTimes(valid, time);
      CALL_ISPC(VKLUnstructuredVolume_sample_export,
                static_cast<const int *>(valid),
                ispcEquivalent,
                &objectCoordinates,
                &samples);
    }

    template <int W>
    inline void UnstructuredSampler<W>::computeSampleN(
        unsigned int N,
        const vvec3fn<1> *objectCoordinates,
        float *samples,
        unsigned int attributeIndex,
        const float *times) const
    {
      assert(attributeIndex < volume->getNumAttributes());
      assertAllValidTimes(N, times);
      CALL_ISPC(Sampler_sample_N_export,
                ispcEquivalent,
                N,
                (ispc::vec3f *)objectCoordinates,
                samples);
    }

    template <int W>
    inline void UnstructuredSampler<W>::computeGradientV(
        const vintn<W> &valid,
        const vvec3fn<W> &objectCoordinates,
        vvec3fn<W> &gradients,
        unsigned int attributeIndex,
        const vfloatn<W> &time) const
    {
      assert(attributeIndex < volume->getNumAttributes());
      assertValidTimes(valid, time);
      CALL_ISPC(VKLUnstructuredVolume_gradient_export,
                static_cast<const int *>(valid),
                ispcEquivalent,
                &objectCoordinates,
                &gradients);
    }

    template <int W>
    inline void UnstructuredSampler<W>::computeGradientN(
        unsigned int N,
        const vvec3fn<1> *objectCoordinates,
        vvec3fn<1> *gradients,
        unsigned int attributeIndex,
        const float *times) const
    {
      assert(attributeIndex < volume->getNumAttributes());
      assertAllValidTimes(N, times);
      CALL_ISPC(Sampler_gradient_N_export,
                ispcEquivalent,
                N,
                (ispc::vec3f *)objectCoordinates,
                (ispc::vec3f *)gradients);
    }

  }  // namespace cpu_device
}  // namespace openvkl
