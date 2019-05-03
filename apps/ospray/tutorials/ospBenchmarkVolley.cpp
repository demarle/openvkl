// ======================================================================== //
// Copyright 2018 Intel Corporation                                         //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "ospBenchmarkVolley.h"
#include "benchmark/benchmark.h"

void initializeOSPRay()
{
  static bool initialized = false;

  if (!initialized) {
    OSPError initError = ospInit(nullptr, nullptr);

    if (initError != OSP_NO_ERROR)
      throw std::runtime_error("error initializing OSPRay");

    ospDeviceSetErrorFunc(
        ospGetCurrentDevice(), [](OSPError error, const char *errorDetails) {
          std::cerr << "OSPRay error: " << errorDetails << std::endl;
          exit(error);
        });

    initialized = true;
  }
}

void initializeVolley()
{
  static bool initialized = false;

  if (!initialized) {
    vlyLoadModule("ispc_driver");

    VLYDriver driver = vlyNewDriver("ispc_driver");
    vlyCommitDriver(driver);
    vlySetCurrentDriver(driver);

    initialized = true;
  }
}

static void volley_volume_only(benchmark::State &state,
                               const std::string &rendererType,
                               const vec2i &windowSize,
                               int volumeDimension)
{
  initializeOSPRay();
  initializeVolley();

  std::shared_ptr<WaveletProceduralVolume> v(new WaveletProceduralVolume(
      vec3i(volumeDimension), vec3f(-1.f), vec3f(2.f / volumeDimension)));

  std::unique_ptr<OSPRayVolleyTestScene> ovts(
      new OSPRayVolleyTestScene(rendererType, v));

  std::unique_ptr<OSPRayWindow> ow = ovts->getOSPRayWindow(windowSize);

  for (auto _ : state) {
    ow->render();
  }
  state.SetItemsProcessed(state.iterations());

  ow->savePPM("test.ppm");
}

BENCHMARK_CAPTURE(
    volley_volume_only, simple_volley / 512, "simple_volley", vec2i(1024), 512);

BENCHMARK_CAPTURE(volley_volume_only,
                  volley_ray_iterator_volume / 512,
                  "volley_ray_iterator_volume",
                  vec2i(1024),
                  512);
BENCHMARK_CAPTURE(volley_volume_only,
                  volley_ray_iterator / 512,
                  "volley_ray_iterator",
                  vec2i(1024),
                  512);

BENCHMARK_MAIN();
