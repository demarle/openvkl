## Copyright 2019 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

# Benchmarks
if (BUILD_BENCHMARKS)
  # Structured volumes
  add_executable(vklBenchmarkStructuredVolume
    vklBenchmarkStructuredVolume.cpp
    ${VKL_RESOURCE}
  )

  target_link_libraries(vklBenchmarkStructuredVolume
    benchmark
    openvkl_testing
  )

  install(TARGETS vklBenchmarkStructuredVolume
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  # Structured volumes (multi-attribute)
  add_executable(vklBenchmarkStructuredVolumeMulti
    vklBenchmarkStructuredVolumeMulti.cpp
    ${VKL_RESOURCE}
  )

  target_link_libraries(vklBenchmarkStructuredVolumeMulti
    benchmark
    openvkl_testing
  )

  install(TARGETS vklBenchmarkStructuredVolumeMulti
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  # Unstructured volumes
  add_executable(vklBenchmarkUnstructuredVolume
    vklBenchmarkUnstructuredVolume.cpp
    ${VKL_RESOURCE}
  )

  target_link_libraries(vklBenchmarkUnstructuredVolume
    benchmark
    openvkl_testing
  )

  install(TARGETS vklBenchmarkUnstructuredVolume
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  # VDB volumes
  add_executable(vklBenchmarkVdbVolume
    vklBenchmarkVdbVolume.cpp
    ${VKL_RESOURCE}
  )

  target_link_libraries(vklBenchmarkVdbVolume
    benchmark
    openvkl_testing
  )

  install(TARGETS vklBenchmarkVdbVolume
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  # VDBvolumes (multi-attribute)
    add_executable(vklBenchmarkVdbVolumeMulti
    vklBenchmarkVdbVolumeMulti.cpp
    ${VKL_RESOURCE}
  )

  target_link_libraries(vklBenchmarkVdbVolumeMulti
    benchmark
    openvkl_testing
  )

  install(TARGETS vklBenchmarkVdbVolumeMulti
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  # Particle volumes
  add_executable(vklBenchmarkParticleVolume
    vklBenchmarkParticleVolume.cpp
    ${VKL_RESOURCE}
  )

  target_link_libraries(vklBenchmarkParticleVolume
    benchmark
    openvkl_testing
  )

  install(TARGETS vklBenchmarkParticleVolume
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

# Functional tests
if (BUILD_TESTING)
  include_directories_ispc(
    ${PROJECT_SOURCE_DIR}/openvkl/include
    ${PROJECT_SOURCE_DIR}/openvkl/devices/cpu
    ${RKCOMMON_INCLUDE_DIRS}
  )

  openvkl_add_executable_ispc(vklTests
    vklTests.cpp
    tests/alignment.cpp
    tests/background_undefined.cpp
    tests/hit_iterator.cpp
    tests/hit_iterator_epsilon.cpp
    tests/interval_iterator.cpp
    tests/simd_conformance.cpp
    tests/simd_conformance.ispc
    tests/simd_type_conversion.cpp
    tests/structured_volume_gradients.cpp
    tests/structured_regular_volume_sampling.cpp
    tests/structured_regular_volume_sampling_motion_blur.cpp
    tests/structured_regular_volume_gradients_motion_blur.cpp
    tests/structured_regular_volume_strides.cpp
    tests/structured_regular_volume_multi.cpp
    tests/structured_spherical_volume_sampling.cpp
    tests/structured_spherical_volume_bounding_box.cpp
    tests/structured_volume_value_range.cpp
    tests/unstructured_volume_gradients.cpp
    tests/unstructured_volume_sampling.cpp
    tests/unstructured_volume_strides.cpp
    tests/unstructured_volume_value_range.cpp
    tests/vectorized_gradients.cpp
    tests/stream_gradients.cpp
    tests/vectorized_hit_iterator.cpp
    tests/vectorized_interval_iterator.cpp
    tests/vectorized_sampling.cpp
    tests/stream_sampling.cpp
    tests/amr_volume_sampling.cpp
    tests/amr_volume_value_range.cpp
    tests/vdb_volume.cpp
    tests/vdb_volume_multi.cpp
    tests/vdb_volume_motion_blur.cpp
    tests/vdb_volume_inner_node_observer.cpp
    tests/vdb_volume_dense.cpp
    tests/particle_volume_sampling.cpp
    tests/particle_volume_gradients.cpp
    tests/particle_volume_value_range.cpp
    tests/particle_volume_radius.cpp
    tests/particle_volume_interval_iterator.cpp
    tests/multi_device.cpp
  )

  target_include_directories(vklTests PRIVATE ${ISPC_TARGET_DIR})

  target_link_libraries(vklTests PRIVATE openvkl_testing openvkl_module_cpu_device)

  # Needed for SIMD conformance tests
  foreach(TARGET_NAME "openvkl_module_cpu_device_4"
                      "openvkl_module_cpu_device_8"
                      "openvkl_module_cpu_device_16")
    if (TARGET ${TARGET_NAME})
      target_link_libraries(vklTests PRIVATE ${TARGET_NAME})
    endif()
  endforeach()

  install(TARGETS vklTests
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  ## Expose tests to CTest ##

  add_test(NAME "simd_conformance"    COMMAND vklTests "[simd_conformance]")
  add_test(NAME "interval_iterators"  COMMAND vklTests "[interval_iterators]")
  add_test(NAME "hit_iterators"       COMMAND vklTests "[hit_iterators]")
  add_test(NAME "volume_bounding_box" COMMAND vklTests "[volume_bounding_box]")
  add_test(NAME "volume_gradients"    COMMAND vklTests "[volume_gradients]")
  add_test(NAME "volume_sampling"     COMMAND vklTests "[volume_sampling]")
  add_test(NAME "volume_value_range"  COMMAND vklTests "[volume_value_range]")
endif()
