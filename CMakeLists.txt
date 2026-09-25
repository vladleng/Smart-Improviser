cmake_minimum_required(VERSION 3.22)

project(SmartImproviser
    VERSION 0.3.0
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
    src/core/model/AnalysisEvidence.h
    src/core/model/ChordModel.cpp
    src/core/model/ChordModel.h
    src/core/model/HarmonicInterpretation.h
    src/core/model/HarmonicPattern.h
    src/core/model/HarmonicSituation.cpp
    src/core/model/HarmonicSituation.h
    src/core/model/ImprovisationContracts.h
    src/core/model/KeyCenter.h
    src/core/model/KeyModel.cpp
    src/core/model/KeyModel.h
    src/core/model/Phrase.h
    src/core/model/ResolutionTarget.h
    src/core/analysis/AmbiguityAnalyzer.cpp
    src/core/analysis/AmbiguityAnalyzer.h
    src/core/analysis/DiatonicSources.cpp
    src/core/analysis/DiatonicSources.h
    src/core/analysis/SpecialSources.cpp
    src/core/analysis/SpecialSources.h
    src/core/analysis/HarmonicConcepts.cpp
    src/core/analysis/HarmonicConcepts.h
    src/core/analysis/ImprovisationEngine.cpp
    src/core/analysis/ImprovisationEngine.h
    src/core/analysis/HarmonicEngine.cpp
    src/core/analysis/HarmonicEngine.h
    src/core/analysis/HarmonicFunction.cpp
    src/core/analysis/HarmonicFunction.h
    src/core/analysis/LocalKeyCenterAnalyzer.cpp
    src/core/analysis/LocalKeyCenterAnalyzer.h
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
# Stage 1 context mapping contract. This layer converts the ARA/shared snapshot
# into host-neutral HarmonicContext / TimelineHarmonicSnapshot values and has no
# JUCE/ARA dependency, so edge cases can be regression-tested directly.
add_library(SmartImproviserContext STATIC
    src/context/SharedHarmonicContextData.h
    src/context/TimelineContextMapper.cpp
    src/context/TimelineContextMapper.h
)

target_include_directories(SmartImproviserContext
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(SmartImproviserContext PUBLIC SmartImproviserCore)
target_compile_features(SmartImproviserContext PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(SmartImproviserContext PRIVATE /utf-8)
endif()

# -----------------------------------------------------------------------------
option(SMART_IMPROVISER_BUILD_PLUGIN "Build JUCE/ARA plugin" ON)
if(SMART_IMPROVISER_BUILD_PLUGIN)
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

# ARA/Event-FX helper. It reads Studio Pro musical context and publishes it to
# the isolated Smart Improviser shared-memory bridge. During Stage 1 it also
# exposes a small diagnostics editor so live ARA/context tests are observable.
# No voicing, MIDI routing or harmonizer code is included here.
#
# User-facing package name is deliberately stable across all checkpoints:
#     Smart Improviser.vst3
# This allows each test build to replace the previous VST3 package directly.
juce_add_plugin(SmartImproviserARA
    COMPANY_NAME "Moon River Studio"
    BUNDLE_ID "com.moonriverstudio.smartimproviserara"
    PLUGIN_MANUFACTURER_CODE MNRV
    PLUGIN_CODE SmIA
    FORMATS VST3
    PRODUCT_NAME "Smart Improviser"
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
        src/context/ARAContextProvider.cpp
        src/context/ARAContextProvider.h
        src/context/SharedHarmonicContext.h
        src/ara/ARAContextDebugState.h
        src/ara/ARAContextDocumentController.cpp
        src/ara/ARAContextDocumentController.h
        src/ara/ARAPluginEditor.cpp
        src/ara/ARAPluginEditor.h
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
        SMART_IMPROVISER_BUILD_VERSION="0.3e fix1"
)

target_link_libraries(SmartImproviserARA
    PRIVATE
        SmartImproviserContext
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
endif() # SMART_IMPROVISER_BUILD_PLUGIN

# Regression tests.
add_executable(SmartImproviserCoreTests
    tests/CoreHarmonyTests.cpp
)

target_link_libraries(SmartImproviserCoreTests PRIVATE SmartImproviserCore)
target_compile_features(SmartImproviserCoreTests PRIVATE cxx_std_20)

add_executable(SmartImproviserDataModelTests
    tests/CoreDataModelTests.cpp
)

target_link_libraries(SmartImproviserDataModelTests PRIVATE SmartImproviserCore)
target_compile_features(SmartImproviserDataModelTests PRIVATE cxx_std_20)

add_executable(SmartImproviserHarmonicEngineTests
    tests/HarmonicEngineTests.cpp
)

target_link_libraries(SmartImproviserHarmonicEngineTests PRIVATE SmartImproviserCore)
target_compile_features(SmartImproviserHarmonicEngineTests PRIVATE cxx_std_20)

add_executable(SmartImproviserLocalKeyCenterTests
    tests/LocalKeyCenterTests.cpp
)

target_link_libraries(SmartImproviserLocalKeyCenterTests PRIVATE SmartImproviserCore)
target_compile_features(SmartImproviserLocalKeyCenterTests PRIVATE cxx_std_20)

add_executable(SmartImproviserAmbiguityConfidenceTests
    tests/AmbiguityConfidenceTests.cpp
)

target_link_libraries(SmartImproviserAmbiguityConfidenceTests PRIVATE SmartImproviserCore)
target_compile_features(SmartImproviserAmbiguityConfidenceTests PRIVATE cxx_std_20)

add_executable(SmartImproviserIntegrationValidationTests
    tests/IntegrationValidationTests.cpp
)

target_link_libraries(SmartImproviserIntegrationValidationTests PRIVATE SmartImproviserCore)
target_compile_features(SmartImproviserIntegrationValidationTests PRIVATE cxx_std_20)

add_executable(SmartImproviserTimelineContextTests
    tests/TimelineContextMapperTests.cpp
)

target_link_libraries(SmartImproviserTimelineContextTests PRIVATE SmartImproviserContext)
target_compile_features(SmartImproviserTimelineContextTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(SmartImproviserCoreTests PRIVATE /utf-8)
    target_compile_options(SmartImproviserDataModelTests PRIVATE /utf-8)
    target_compile_options(SmartImproviserHarmonicEngineTests PRIVATE /utf-8)
    target_compile_options(SmartImproviserLocalKeyCenterTests PRIVATE /utf-8)
    target_compile_options(SmartImproviserAmbiguityConfidenceTests PRIVATE /utf-8)
    target_compile_options(SmartImproviserIntegrationValidationTests PRIVATE /utf-8)
    target_compile_options(SmartImproviserTimelineContextTests PRIVATE /utf-8)
endif()

add_test(NAME SmartImproviserCoreTests COMMAND SmartImproviserCoreTests)
add_test(NAME SmartImproviserDataModelTests COMMAND SmartImproviserDataModelTests)
add_test(NAME SmartImproviserHarmonicEngineTests COMMAND SmartImproviserHarmonicEngineTests)
add_test(NAME SmartImproviserLocalKeyCenterTests COMMAND SmartImproviserLocalKeyCenterTests)
add_test(NAME SmartImproviserAmbiguityConfidenceTests COMMAND SmartImproviserAmbiguityConfidenceTests)
add_test(NAME SmartImproviserIntegrationValidationTests COMMAND SmartImproviserIntegrationValidationTests)
add_test(NAME SmartImproviserTimelineContextTests COMMAND SmartImproviserTimelineContextTests)

add_executable(SmartImproviserImprovisationTests tests/ImprovisationEngineTests.cpp)
target_link_libraries(SmartImproviserImprovisationTests PRIVATE SmartImproviserCore)
if(MSVC)
    target_compile_options(SmartImproviserImprovisationTests PRIVATE /utf-8)
endif()
add_test(NAME SmartImproviserImprovisationTests COMMAND SmartImproviserImprovisationTests)

add_executable(SmartImproviserDiatonicSourcesTests tests/DiatonicSourcesTests.cpp)
target_link_libraries(SmartImproviserDiatonicSourcesTests PRIVATE SmartImproviserCore)
if(MSVC)
    target_compile_options(SmartImproviserDiatonicSourcesTests PRIVATE /utf-8)
endif()
add_test(NAME SmartImproviserDiatonicSourcesTests COMMAND SmartImproviserDiatonicSourcesTests)

add_executable(SmartImproviserSpecialSourcesTests tests/SpecialSourcesTests.cpp)
target_link_libraries(SmartImproviserSpecialSourcesTests PRIVATE SmartImproviserCore)
if(MSVC)
    target_compile_options(SmartImproviserSpecialSourcesTests PRIVATE /utf-8)
endif()
add_test(NAME SmartImproviserSpecialSourcesTests COMMAND SmartImproviserSpecialSourcesTests)

add_executable(SmartImproviserHarmonicConceptsTests tests/HarmonicConceptsTests.cpp)
target_link_libraries(SmartImproviserHarmonicConceptsTests PRIVATE SmartImproviserCore)
if(MSVC)
    target_compile_options(SmartImproviserHarmonicConceptsTests PRIVATE /utf-8)
endif()
add_test(NAME SmartImproviserHarmonicConceptsTests COMMAND SmartImproviserHarmonicConceptsTests)
