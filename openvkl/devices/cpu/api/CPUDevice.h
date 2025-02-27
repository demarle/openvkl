// Copyright 2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../../../api/Device.h"
#include "../common/align.h"
#include "../iterator/Iterator.h"
#include "../sampler/Sampler.h"

namespace openvkl {
  namespace cpu_device {

    /*
     * Type alias to make enable_if more concise (we cannot rely on C++14).
     */
    template <bool C, class T = void>
    using EnableIf = typename std::enable_if<C, T>::type;

    template <int W>
    struct CPUDevice : public api::Device
    {
      CPUDevice()           = default;
      ~CPUDevice() override = default;

      bool supportsWidth(int width) override;

      int getNativeSIMDWidth() override;

      void commit() override;

      void commit(VKLObject object) override;

      void release(VKLObject object) override;

      /////////////////////////////////////////////////////////////////////////
      // Data /////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

      VKLData newData(size_t numItems,
                      VKLDataType dataType,
                      const void *source,
                      VKLDataCreationFlags dataCreationFlags,
                      size_t byteStride) override;

      /////////////////////////////////////////////////////////////////////////
      // Observer /////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

      VKLObserver newObserver(VKLVolume volume, const char *type) override;
      VKLObserver newObserver(VKLSampler sampler, const char *type) override;
      const void *mapObserver(VKLObserver observer) override;
      void unmapObserver(VKLObserver observer) override;
      VKLDataType getObserverElementType(VKLObserver observer) const override;
      size_t getObserverElementSize(VKLObserver observer) const override;
      size_t getObserverNumElements(VKLObserver observer) const override;

      /////////////////////////////////////////////////////////////////////////
      // Interval iterator ////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

      VKLIntervalIteratorContext newIntervalIteratorContext(
          VKLSampler sampler) override;

      size_t getIntervalIteratorSize1(
          VKLIntervalIteratorContext context) const override
      {
        const auto &contextObject =
            referenceFromHandle<IntervalIteratorContext<W>>(context);
        const auto &samplerObject = contextObject.getSampler();
        return samplerObject.getIntervalIteratorFactory().sizeU();
      }

#define __define_getIntervalIteratorSizeN(WIDTH)                            \
  size_t getIntervalIteratorSize##WIDTH(VKLIntervalIteratorContext context) \
      const override                                                        \
  {                                                                         \
    const auto &contextObject =                                             \
        referenceFromHandle<IntervalIteratorContext<W>>(context);           \
    const auto &samplerObject = contextObject.getSampler();                 \
    return samplerObject.getIntervalIteratorFactory().sizeV();              \
  }

      __define_getIntervalIteratorSizeN(4);
      __define_getIntervalIteratorSizeN(8);
      __define_getIntervalIteratorSizeN(16);

#undef __define_getIntervalIteratorSizeN

      //////////////////////////////////////////////////////////////////////////
     public:
      VKLIntervalIterator initIntervalIterator1(
          VKLIntervalIteratorContext context,
          const vvec3fn<1> &origin,
          const vvec3fn<1> &direction,
          const vrange1fn<1> &tRange,
          float time,
          void *buffer) const override;

#define __define_initIntervalIteratorN(WIDTH)                           \
  VKLIntervalIterator##WIDTH initIntervalIterator##WIDTH(               \
      const int *valid,                                                 \
      VKLIntervalIteratorContext context,                               \
      const vvec3fn<WIDTH> &origin,                                     \
      const vvec3fn<WIDTH> &direction,                                  \
      const vrange1fn<WIDTH> &tRange,                                   \
      const float *times,                                               \
      void *buffer) const override                                      \
  {                                                                     \
    return reinterpret_cast<VKLIntervalIterator##WIDTH>(                \
        initIntervalIteratorAnyWidth<WIDTH>(                            \
            valid, context, origin, direction, tRange, times, buffer)); \
  }

      __define_initIntervalIteratorN(4);
      __define_initIntervalIteratorN(8);
      __define_initIntervalIteratorN(16);

#undef __define_initIntervalIteratorN

     private:
      template <int OW>
      EnableIf<(W == OW), IntervalIterator<W> *> initIntervalIteratorAnyWidth(
          const int *valid,
          VKLIntervalIteratorContext context,
          const vvec3fn<OW> &origin,
          const vvec3fn<OW> &direction,
          const vrange1fn<OW> &tRange,
          const float *times,
          void *buffer) const;

      template <int OW>
      EnableIf<(W != OW), IntervalIterator<W> *> initIntervalIteratorAnyWidth(
          const int *valid,
          VKLIntervalIteratorContext context,
          const vvec3fn<OW> &origin,
          const vvec3fn<OW> &direction,
          const vrange1fn<OW> &tRange,
          const float *times,
          void *buffer) const;

      //////////////////////////////////////////////////////////////////////////

     public:
      void iterateInterval1(const VKLIntervalIterator iterator,
                            vVKLIntervalN<1> &interval,
                            int *result) const override;

#define __define_iterateIntervalN(WIDTH)                           \
  void iterateInterval##WIDTH(const int *valid,                    \
                              VKLIntervalIterator##WIDTH iterator, \
                              vVKLIntervalN<WIDTH> &interval,      \
                              int *result) const override          \
  {                                                                \
    iterateIntervalAnyWidth<WIDTH>(                                \
        valid,                                                     \
        referenceFromHandle<IntervalIterator<WIDTH>>(iterator),    \
        interval,                                                  \
        result);                                                   \
  }

      __define_iterateIntervalN(4);
      __define_iterateIntervalN(8);
      __define_iterateIntervalN(16);

#undef __define_iterateIntervalN

     private:
      template <int OW>
      EnableIf<(W == OW)> iterateIntervalAnyWidth(
          const int *valid,
          IntervalIterator<OW> &iterator,
          vVKLIntervalN<OW> &interval,
          int *result) const;

      template <int OW>
      EnableIf<(W != OW)> iterateIntervalAnyWidth(
          const int *valid,
          IntervalIterator<OW> &iterator,
          vVKLIntervalN<OW> &interval,
          int *result) const;

      /////////////////////////////////////////////////////////////////////////
      // Hit iterator /////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

     public:
      VKLHitIteratorContext newHitIteratorContext(VKLSampler sampler) override;

      size_t getHitIteratorSize1(VKLHitIteratorContext context) const override
      {
        const auto &contextObject =
            referenceFromHandle<HitIteratorContext<W>>(context);
        const auto &samplerObject = contextObject.getSampler();
        return samplerObject.getHitIteratorFactory().sizeU();
      }

#define __define_getHitIteratorSizeN(WIDTH)                       \
  size_t getHitIteratorSize##WIDTH(VKLHitIteratorContext context) \
      const override                                              \
  {                                                               \
    const auto &contextObject =                                   \
        referenceFromHandle<HitIteratorContext<W>>(context);      \
    const auto &samplerObject = contextObject.getSampler();       \
    return samplerObject.getHitIteratorFactory().sizeV();         \
  }

      __define_getHitIteratorSizeN(4);
      __define_getHitIteratorSizeN(8);
      __define_getHitIteratorSizeN(16);

#undef __define_getIntervalIteratorSizeN

      //////////////////////////////////////////////////////////////////////////

     public:
      VKLHitIterator initHitIterator1(VKLHitIteratorContext context,
                                      const vvec3fn<1> &origin,
                                      const vvec3fn<1> &direction,
                                      const vrange1fn<1> &tRange,
                                      float time,
                                      void *buffer) const override;

#define __define_initHitIteratorN(WIDTH)                                \
  VKLHitIterator##WIDTH initHitIterator##WIDTH(                         \
      const int *valid,                                                 \
      VKLHitIteratorContext context,                                    \
      const vvec3fn<WIDTH> &origin,                                     \
      const vvec3fn<WIDTH> &direction,                                  \
      const vrange1fn<WIDTH> &tRange,                                   \
      const float *times,                                               \
      void *buffer) const override                                      \
  {                                                                     \
    return reinterpret_cast<VKLHitIterator##WIDTH>(                     \
        initHitIteratorAnyWidth<WIDTH>(                                 \
            valid, context, origin, direction, tRange, times, buffer)); \
  }

      __define_initHitIteratorN(4);
      __define_initHitIteratorN(8);
      __define_initHitIteratorN(16);

#undef __define_initHitIteratorN

     private:
      template <int OW>
      EnableIf<(W == OW), HitIterator<W> *> initHitIteratorAnyWidth(
          const int *valid,
          VKLHitIteratorContext context,
          const vvec3fn<OW> &origin,
          const vvec3fn<OW> &direction,
          const vrange1fn<OW> &tRange,
          const float *times,
          void *buffer) const;

      template <int OW>
      EnableIf<(W != OW), HitIterator<W> *> initHitIteratorAnyWidth(
          const int *valid,
          VKLHitIteratorContext context,
          const vvec3fn<OW> &origin,
          const vvec3fn<OW> &direction,
          const vrange1fn<OW> &tRange,
          const float *times,
          void *buffer) const;

      //////////////////////////////////////////////////////////////////////////

     public:
      void iterateHit1(VKLHitIterator iterator,
                       vVKLHitN<1> &hit,
                       int *result) const override;

#define __define_iterateHitN(WIDTH)                        \
  void iterateHit##WIDTH(const int *valid,                 \
                         VKLHitIterator##WIDTH iterator,   \
                         vVKLHitN<WIDTH> &hit,             \
                         int *result) const override       \
  {                                                        \
    iterateHitAnyWidth<WIDTH>(                             \
        valid,                                             \
        referenceFromHandle<HitIterator<WIDTH>>(iterator), \
        hit,                                               \
        result);                                           \
  }

      __define_iterateHitN(4);
      __define_iterateHitN(8);
      __define_iterateHitN(16);

#undef __define_iterateHitN

     private:
      template <int OW>
      EnableIf<(W == OW)> iterateHitAnyWidth(const int *valid,
                                             HitIterator<OW> &iterator,
                                             vVKLHitN<OW> &interval,
                                             int *result) const;

      template <int OW>
      EnableIf<(W != OW)> iterateHitAnyWidth(const int *valid,
                                             HitIterator<OW> &iterator,
                                             vVKLHitN<OW> &interval,
                                             int *result) const;

      /////////////////////////////////////////////////////////////////////////
      // Parameters ///////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

      void setBool(VKLObject object, const char *name, const bool b) override;
      void set1f(VKLObject object, const char *name, const float x) override;
      void set1i(VKLObject object, const char *name, const int x) override;
      void setVec3f(VKLObject object,
                    const char *name,
                    const vec3f &v) override;
      void setVec3i(VKLObject object,
                    const char *name,
                    const vec3i &v) override;
      void setObject(VKLObject object,
                     const char *name,
                     VKLObject setObject) override;
      void setString(VKLObject object,
                     const char *name,
                     const std::string &s) override;
      void setVoidPtr(VKLObject object, const char *name, void *v) override;

      void setObjectParam(VKLObject object,
                          const char *name,
                          VKLDataType dataType,
                          const void *mem) override;

      /////////////////////////////////////////////////////////////////////////
      // Sampler //////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

      VKLSampler newSampler(VKLVolume volume) override;

#define __define_computeSampleN(WIDTH)                               \
  void computeSample##WIDTH(const int *valid,                        \
                            VKLSampler sampler,                      \
                            const vvec3fn<WIDTH> &objectCoordinates, \
                            float *samples,                          \
                            unsigned int attributeIndex,             \
                            const float *times) override;

      __define_computeSampleN(1);
      __define_computeSampleN(4);
      __define_computeSampleN(8);
      __define_computeSampleN(16);

#undef __define_computeSampleN

      void computeSampleN(VKLSampler sampler,
                          unsigned int N,
                          const vvec3fn<1> *objectCoordinates,
                          float *samples,
                          unsigned int attributeIndex,
                          const float *times) override;

#define __define_computeSampleMN(WIDTH)                               \
  void computeSampleM##WIDTH(const int *valid,                        \
                             VKLSampler sampler,                      \
                             const vvec3fn<WIDTH> &objectCoordinates, \
                             float *samples,                          \
                             unsigned int M,                          \
                             const unsigned int *attributeIndices,    \
                             const float *times) override;

      __define_computeSampleMN(1);
      __define_computeSampleMN(4);
      __define_computeSampleMN(8);
      __define_computeSampleMN(16);

#undef __define_computeSampleMN

      void computeSampleMN(VKLSampler sampler,
                           unsigned int N,
                           const vvec3fn<1> *objectCoordinates,
                           float *samples,
                           unsigned int M,
                           const unsigned int *attributeIndices,
                           const float *times) override;

#define __define_computeGradientN(WIDTH)                               \
  void computeGradient##WIDTH(const int *valid,                        \
                              VKLSampler sampler,                      \
                              const vvec3fn<WIDTH> &objectCoordinates, \
                              vvec3fn<WIDTH> &gradients,               \
                              unsigned int attributeIndex,             \
                              const float *times) override;

      __define_computeGradientN(1);
      __define_computeGradientN(4);
      __define_computeGradientN(8);
      __define_computeGradientN(16);

#undef __define_computeGradientN

      void computeGradientN(VKLSampler sampler,
                            unsigned int N,
                            const vvec3fn<1> *objectCoordinates,
                            vvec3fn<1> *gradients,
                            unsigned int attributeIndex,
                            const float *times) override;

      /////////////////////////////////////////////////////////////////////////
      // Volume ///////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

      VKLVolume newVolume(const char *type) override;

      box3f getBoundingBox(VKLVolume volume) override;

      unsigned int getNumAttributes(VKLVolume volume) override;

      range1f getValueRange(VKLVolume volume,
                            unsigned int attributeIndex) override;

     private:
      template <int OW>
      typename std::enable_if<(OW < W), void>::type computeSampleAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          float *samples,
          unsigned int attributeIndex,
          const float *times);

      template <int OW>
      typename std::enable_if<(OW == W), void>::type computeSampleAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          float *samples,
          unsigned int attributeIndex,
          const float *times);

      template <int OW>
      typename std::enable_if<(OW > W), void>::type computeSampleAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          float *samples,
          unsigned int attributeIndex,
          const float *times);

      template <int OW>
      typename std::enable_if<(OW < W), void>::type computeSampleMAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          float *samples,
          unsigned int M,
          const unsigned int *attributeIndices,
          const float *times);

      template <int OW>
      typename std::enable_if<(OW == W), void>::type computeSampleMAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          float *samples,
          unsigned int M,
          const unsigned int *attributeIndices,
          const float *times);

      template <int OW>
      typename std::enable_if<(OW > W), void>::type computeSampleMAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          float *samples,
          unsigned int M,
          const unsigned int *attributeIndices,
          const float *times);

      template <int OW>
      typename std::enable_if<(OW < W), void>::type computeGradientAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          vvec3fn<OW> &gradients,
          unsigned int attributeIndex,
          const float *times);

      template <int OW>
      typename std::enable_if<(OW == W), void>::type computeGradientAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          vvec3fn<OW> &gradients,
          unsigned int attributeIndex,
          const float *times);

      template <int OW>
      typename std::enable_if<(OW > W), void>::type computeGradientAnyWidth(
          const int *valid,
          VKLSampler sampler,
          const vvec3fn<OW> &objectCoordinates,
          vvec3fn<OW> &gradients,
          unsigned int attributeIndex,
          const float *times);
    };

    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Interval iterators
    ////////////////////////////////////////////////////////////////////////////

    template <int W>
    inline VKLIntervalIterator CPUDevice<W>::initIntervalIterator1(
        VKLIntervalIteratorContext context,
        const vvec3fn<1> &origin,
        const vvec3fn<1> &direction,
        const vrange1fn<1> &tRange,
        float time,
        void *buffer) const
    {
      const auto &ctx =
          referenceFromHandle<IntervalIteratorContext<W>>(context);
      const auto &spl     = ctx.getSampler();
      const auto &factory = spl.getIntervalIteratorFactory();

      assertValidTime(time);

      IntervalIterator<W> *it = factory.constructU(ctx, buffer);
      it->initializeIntervalU(origin, direction, tRange, time);

      return reinterpret_cast<VKLIntervalIterator>(it);
    }

    template <int W>
    template <int OW>
    inline EnableIf<(W == OW), IntervalIterator<W> *>
    CPUDevice<W>::initIntervalIteratorAnyWidth(
        const int *valid,
        VKLIntervalIteratorContext context,
        const vvec3fn<OW> &origin,
        const vvec3fn<OW> &direction,
        const vrange1fn<OW> &tRange,
        const float *times,
        void *buffer) const
    {
      const auto &ctx =
          referenceFromHandle<IntervalIteratorContext<W>>(context);
      const auto &spl     = ctx.getSampler();
      const auto &factory = spl.getIntervalIteratorFactory();

      IntervalIterator<W> *it = factory.constructV(ctx, buffer);

      vintn<W> validW;
      for (int i = 0; i < W; ++i)
        validW[i] = valid[i];

      const vfloatn<W> timesW(times, OW);
      assertValidTimes(validW, timesW);

      it->initializeIntervalV(validW, origin, direction, tRange, timesW);

      return it;
    }

    template <int W>
    template <int OW>
    inline EnableIf<(W != OW), IntervalIterator<W> *>
    CPUDevice<W>::initIntervalIteratorAnyWidth(
        const int *valid,
        VKLIntervalIteratorContext context,
        const vvec3fn<OW> &origin,
        const vvec3fn<OW> &direction,
        const vrange1fn<OW> &tRange,
        const float *times,
        void *buffer) const
    {
      throw std::runtime_error(
          "interval iterators are only supported for the "
          "native vector width");
    }

    ////////////////////////////////////////////////////////////////////////////

    template <int W>
    inline void CPUDevice<W>::iterateInterval1(
        const VKLIntervalIterator iterator,
        vVKLIntervalN<1> &interval,
        int *result) const
    {
      auto &it = referenceFromHandle<IntervalIterator<W>>(iterator);
      it.iterateIntervalU(*reinterpret_cast<vVKLIntervalN<1> *>(&interval),
                          reinterpret_cast<vintn<1> &>(*result));
    }

    template <int W>
    template <int OW>
    inline EnableIf<(W == OW)> CPUDevice<W>::iterateIntervalAnyWidth(
        const int *valid,
        IntervalIterator<OW> &iterator,
        vVKLIntervalN<OW> &interval,
        int *result) const
    {
      vintn<W> validW;
      for (int i = 0; i < W; i++)
        validW[i] = valid[i];

      vintn<W> resultW;

      iterator.iterateIntervalV(
          validW, *reinterpret_cast<vVKLIntervalN<W> *>(&interval), resultW);

      for (int i = 0; i < W; i++)
        result[i] = resultW[i];
    }

    template <int W>
    template <int OW>
    inline EnableIf<(W != OW)> CPUDevice<W>::iterateIntervalAnyWidth(
        const int *valid,
        IntervalIterator<OW> &iterator,
        vVKLIntervalN<OW> &interval,
        int *result) const
    {
      throw std::runtime_error(
          "interval iterators are only supported for the "
          "native vector width");
    }

    ////////////////////////////////////////////////////////////////////////////
    // Hit iterators
    ////////////////////////////////////////////////////////////////////////////

    template <int W>
    inline VKLHitIterator CPUDevice<W>::initHitIterator1(
        VKLHitIteratorContext context,
        const vvec3fn<1> &origin,
        const vvec3fn<1> &direction,
        const vrange1fn<1> &tRange,
        float time,
        void *buffer) const
    {
      const auto &ctx     = referenceFromHandle<HitIteratorContext<W>>(context);
      const auto &spl     = ctx.getSampler();
      const auto &factory = spl.getHitIteratorFactory();

      assertValidTime(time);

      HitIterator<W> *it = factory.constructU(ctx, buffer);
      it->initializeHitU(origin, direction, tRange, time);

      return reinterpret_cast<VKLHitIterator>(it);
    }

    template <int W>
    template <int OW>
    inline EnableIf<(W == OW), HitIterator<W> *>
    CPUDevice<W>::initHitIteratorAnyWidth(const int *valid,
                                          VKLHitIteratorContext context,
                                          const vvec3fn<OW> &origin,
                                          const vvec3fn<OW> &direction,
                                          const vrange1fn<OW> &tRange,
                                          const float *times,
                                          void *buffer) const
    {
      const auto &ctx     = referenceFromHandle<HitIteratorContext<W>>(context);
      const auto &spl     = ctx.getSampler();
      const auto &factory = spl.getHitIteratorFactory();

      HitIterator<W> *it = factory.constructV(ctx, buffer);

      vintn<W> validW;
      for (int i = 0; i < W; ++i)
        validW[i] = valid[i];

      const vfloatn<W> timesW(times, OW);
      assertValidTimes(validW, timesW);

      it->initializeHitV(validW, origin, direction, tRange, timesW);

      return it;
    }

    template <int W>
    template <int OW>
    inline EnableIf<(W != OW), HitIterator<W> *>
    CPUDevice<W>::initHitIteratorAnyWidth(const int *valid,
                                          VKLHitIteratorContext context,
                                          const vvec3fn<OW> &origin,
                                          const vvec3fn<OW> &direction,
                                          const vrange1fn<OW> &tRange,
                                          const float *times,
                                          void *buffer) const
    {
      throw std::runtime_error(
          "hit iterators are only supported for the "
          "native vector width");
    }

    ////////////////////////////////////////////////////////////////////////////

    template <int W>
    inline void CPUDevice<W>::iterateHit1(const VKLHitIterator iterator,
                                          vVKLHitN<1> &hit,
                                          int *result) const
    {
      auto &it = referenceFromHandle<HitIterator<W>>(iterator);
      it.iterateHitU(*reinterpret_cast<vVKLHitN<1> *>(&hit),
                     reinterpret_cast<vintn<1> &>(*result));
    }

    template <int W>
    template <int OW>
    inline EnableIf<(W == OW)> CPUDevice<W>::iterateHitAnyWidth(
        const int *valid,
        HitIterator<OW> &iterator,
        vVKLHitN<OW> &hit,
        int *result) const
    {
      vintn<W> validW;
      for (int i = 0; i < W; i++)
        validW[i] = valid[i];

      vintn<W> resultW;

      iterator.iterateHitV(
          validW, *reinterpret_cast<vVKLHitN<W> *>(&hit), resultW);

      for (int i = 0; i < W; i++)
        result[i] = resultW[i];
    }

    template <int W>
    template <int OW>
    inline EnableIf<(W != OW)> CPUDevice<W>::iterateHitAnyWidth(
        const int *valid,
        HitIterator<OW> &iterator,
        vVKLHitN<OW> &hit,
        int *result) const
    {
      throw std::runtime_error(
          "hit iterators are only supported for the "
          "native vector width");
    }

    ////////////////////////////////////////////////////////////////////////////

  }  // namespace cpu_device
}  // namespace openvkl
