// Copyright 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "create_voxels.h"
#include "framebuffer.h"

// We must include the openvkl header.
#include <openvkl/openvkl.h>

int main(int argc, char **argv)
{
  vklLoadModule("cpu_device");
  VKLDevice device = vklNewDevice("cpu");
  vklCommitDevice(device);
  
  // "Load data from disk". (We generate the array procedurally).
  constexpr size_t res      = 128;
  std::vector<float> voxels = createVoxels(res);

  // Note that Open VKL uses a C99 API for maximum compatibility.
  // So we will have to wrap the array we just created so that
  // we can pass it to Open VKL.

  // Create a new volume. Volume objects are created on a device.
  // We create a structured regular grid here, which is essentially
  // a dense 3D array.
  VKLVolume volume = vklNewVolume(device, "structuredRegular");

  // We have to set a few parameters on the volume.
  // First, Open VKL needs to know the extent of the volume:
  vklSetVec3i(volume, "dimensions", res, res, res);

  // By default, the volume assumes a voxel size of 1. Scale it so the
  // domain is [0, 1].
  const float spacing = 1.f / static_cast<float>(res);
  vklSetVec3f(volume, "gridSpacing", spacing, spacing, spacing);

  // Open VKL has a concept of typed Data objects. That's how we pass data
  // buffers to a device. We create a shared buffer here to avoid copying
  // the voxels.
  VKLData voxelData = vklNewData(
      device, voxels.size(), VKL_FLOAT, voxels.data(), VKL_DATA_SHARED_BUFFER);

  // Set the data parameter. We can release the data directly afterwards
  // as Open VKL has a reference counting mechanism and will keep track
  // internally.
  // Note this is a shared buffer, so we have to keep voxels around.
  vklSetData(volume, "data", voxelData);
  vklRelease(voxelData);

  // Finally, commit. This may build acceleration structures, etc.
  vklCommit(volume);

  // Instead of drawing the field directly into our framebuffer, we will instead
  // sample the volume we just created. To do that, we need a sampler object.
  VKLSampler sampler = vklNewSampler(volume);
  vklCommit(sampler);

  Framebuffer fb(64, 32);

  fb.generate([&](float fx, float fy) {
    // To sample, we call vklComputeSample on our sampler object.
    const vkl_vec3f p = {fx, fy, 0.f};
    return transferFunction(vklComputeSample(sampler, &p));
  });

  fb.drawToTerminal();

  // Release the volume to clean up!
  vklRelease(sampler);
  vklRelease(volume);
  vklReleaseDevice(device);

  return 0;
}
