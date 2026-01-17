# Tests CMakeLists.txt

# Unit tests
add_executable(choir_v2_tests
    unit/test_formant_resonator.cpp
    unit/test_linear_smoother.cpp
    unit/test_subharmonic_generator.cpp
    unit/test_reverb_effect.cpp
    unit/test_voice_allocator.cpp
    unit/test_g2p_engine.cpp
    unit/test_preset_manager.cpp
)

target_link_libraries(choir_v2_tests
    ChoirV2Core
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(choir_v2_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/unit
)

include(GoogleTest)
gtest_discover_tests(choir_v2_tests)

# Integration tests (commented out until implemented)
# add_executable(choir_v2_integration_tests
#     integration/test_synthesis_pipeline.cpp
#     integration/test_voice_manager.cpp
# )
#
# target_link_libraries(choir_v2_integration_tests
#     ChoirV2Core
#     GTest::gtest
#     GTest::gtest_main
# )
#
# gtest_discover_tests(choir_v2_integration_tests)
