cmake_minimum_required(VERSION 3.22)

project(SmartImproviser
    VERSION 0.0.1
    DESCRIPTION "Smart Improviser - context-aware jazz improvisation assistant"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(SmartImproviserCore STATIC
    src/core/context/HarmonicContext.h
    src/core/model/ChordModel.cpp
    src/core/model/ChordModel.h
    src/core/model/KeyModel.cpp
    src/core/model/KeyModel.h
    src/core/analysis/HarmonicFunction.cpp
    src/core/analysis/HarmonicFunction.h
)

target_include_directories(SmartImproviserCore
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(SmartImproviserCore PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(SmartImproviserCore PRIVATE /utf-8)
endif()

add_executable(SmartImproviserCoreTests
    tests/CoreHarmonyTests.cpp
)

target_link_libraries(SmartImproviserCoreTests PRIVATE SmartImproviserCore)
target_compile_features(SmartImproviserCoreTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(SmartImproviserCoreTests PRIVATE /utf-8)
endif()

add_test(NAME SmartImproviserCoreTests COMMAND SmartImproviserCoreTests)
