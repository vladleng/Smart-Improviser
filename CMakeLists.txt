cmake_minimum_required(VERSION 3.22)

project(SmartImproviser
    VERSION 0.0.1
    DESCRIPTION "Smart Improviser - context-aware jazz improvisation assistant"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(CTest)

# -----------------------------------------------------------------------------
# Host-neutral harmony core. This layer must remain independent from JUCE/ARA.
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

# -----------------------------------------------------------------------------
# JUCE / ARA infrastructure reused from Smart Voicing, with a new isolated ABI.
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

FetchContent_Declare(
    ARA_SDK
    GIT_REPOSITORY https://github.com/Celemony/ARA_SDK.git
    GIT_TAG releases/2.3.0
    GIT_SUBMODULES_RECURSE TRUE
)

FetchContent_GetProperties(ARA_SDK)
if(NOT ara_sdk_POPULATED)
    FetchContent_Populate(ARA_SDK)
endif()

juce_set_ara_sdk_path("${ara_sdk_SOURCE_DIR}")

# Context reader library used later by the main Smart Improviser plug-in.
add_library(SmartImproviserContext STATIC
    src/context/SharedHarmonicContext.h
    src/context/ARAContextProvider.cpp
    src/context/ARAContextProvider.h
)

juce_generate_juce_header(SmartImproviserContext)

target_include_directories(SmartImproviserContext
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(SmartImproviserContext
    PUBLIC
        SmartImproviserCore
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(SmartImproviserContext
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

if(MSVC)
    target_compile_options(SmartImproviserContext PRIVATE /utf-8)
endif()

# Headless ARA/Event-FX helper. It reads Studio Pro musical context and publishes
# it to the isolated Smart Improviser shared-memory bridge. No voicing, MIDI
# routing, harmonizer or UI code is included here.
juce_add_plugin(SmartImproviserARA
    COMPANY_NAME "Moon River Studio"
    BUNDLE_ID "com.moonriverstudio.smartimproviserara"
    PLUGIN_MANUFACTURER_CODE MNRV
    PLUGIN_CODE SmIA
    FORMATS VST3
    PRODUCT_NAME "Smart Improviser ARA"
    IS_SYNTH FALSE
    VST3_CATEGORIES Fx
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_ARA_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(SmartImproviserARA)

target_sources(SmartImproviserARA
    PRIVATE
        src/context/SharedHarmonicContext.h
        src/ara/ARAContextDebugState.h
        src/ara/ARAContextDocumentController.cpp
        src/ara/ARAContextDocumentController.h
        src/ara/ARAPluginProcessor.cpp
        src/ara/ARAPluginProcessor.h
)

target_include_directories(SmartImproviserARA
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(SmartImproviserARA
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SmartImproviserARA
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(MSVC)
    target_compile_options(SmartImproviserARA PRIVATE /utf-8)
endif()

# -----------------------------------------------------------------------------
# Host-neutral regression tests.
add_executable(SmartImproviserCoreTests
    tests/CoreHarmonyTests.cpp
)

target_link_libraries(SmartImproviserCoreTests PRIVATE SmartImproviserCore)
target_compile_features(SmartImproviserCoreTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(SmartImproviserCoreTests PRIVATE /utf-8)
endif()

add_test(NAME SmartImproviserCoreTests COMMAND SmartImproviserCoreTests)
