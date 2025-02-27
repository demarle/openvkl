// Copyright 2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "StructuredRegularVolume.h"
#include "../common/export_util.h"

namespace openvkl {
  namespace cpu_device {

    template <int W>
    void StructuredRegularVolume<W>::commit()
    {
      StructuredVolume<W>::commit();

      if (!this->ispcEquivalent) {
        this->ispcEquivalent = CALL_ISPC(SharedStructuredVolume_Constructor);

        if (!this->ispcEquivalent) {
          throw std::runtime_error(
              "could not create ISPC-side object for StructuredRegularVolume");
        }
      }

      std::vector<const ispc::Data1D *> ispcAttributesData =
          ispcs(this->attributesData);

      bool success = CALL_ISPC(SharedStructuredVolume_set,
                               this->ispcEquivalent,
                               ispcAttributesData.size(),
                               ispcAttributesData.data(),
                               this->temporallyStructuredNumTimesteps,
                               ispc(this->temporallyUnstructuredIndices),
                               ispc(this->temporallyUnstructuredTimes),
                               (const ispc::vec3i &)this->dimensions,
                               ispc::structured_regular,
                               (const ispc::vec3f &)this->gridOrigin,
                               (const ispc::vec3f &)this->gridSpacing,
                               (ispc::VKLFilter)this->filter);

      if (!success) {
        CALL_ISPC(SharedStructuredVolume_Destructor, this->ispcEquivalent);
        this->ispcEquivalent = nullptr;

        throw std::runtime_error("failed to commit StructuredRegularVolume");
      }

      CALL_ISPC(
          Volume_setBackground, this->ispcEquivalent, this->background->data());

      // must be last
      this->buildAccelerator();
    }

    //VKL_REGISTER_VOLUME(StructuredRegularVolume<VKL_TARGET_WIDTH>,
    //                    CONCAT1(internal_structuredRegular_, VKL_TARGET_WIDTH))

  }  // namespace cpu_device
}  // namespace openvkl
