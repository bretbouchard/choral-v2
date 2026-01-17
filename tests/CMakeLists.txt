# Tests CMakeLists.txt

# Unit tests
add_executable(choir_v2_tests
    unit/test_phoneme_database.cpp
    unit/test_language_loader.cpp
    unit/test_g2p_engine.cpp
    unit/test_formant_resonator.cpp
    unit/test_glottal_source.cpp
    unit/test_linear_smoother.cpp
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

discover_tests(choir_v2_tests)

# Integration tests
add_executable(choir_v2_integration_tests
    integration/test_synthesis_pipeline.cpp
    integration/test_voice_manager.cpp
)

target_link_libraries(choir_v2_integration_tests
    ChoirV2Core
    GTest::gtest
    GTest::gtest_main
)

discover_tests(choir_v2_integration_tests)
