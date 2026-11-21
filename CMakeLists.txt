cmake_minimum_required(VERSION 3.22)

project(Chioccia VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ------------------------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------------------------
option(CHIOCCIA_BUILD_PLUGIN   "Build the JUCE plug-in (VST3, AU on macOS)"            ON)
option(CHIOCCIA_BUILD_TESTS    "Build automatic tests and the measurement tool"       ON)
option(CHIOCCIA_BUILD_STANDALONE "Also build a Standalone app (handy for GUI checks)"  OFF)
option(CHIOCCIA_ENABLE_VST2    "Build VST2 (.dll) — ONLY with a legally obtained VST2 SDK" OFF)
set(CHIOCCIA_VST2_SDK_DIR "" CACHE PATH "Folder of a legally obtained VST2 SDK (contains pluginterfaces/vst2.x)")
set(CHIOCCIA_JUCE_DIR "" CACHE PATH "Local JUCE checkout. If empty, JUCE is fetched with FetchContent")
set(CHIOCCIA_JUCE_TAG "9.0.4" CACHE STRING "JUCE git tag used by FetchContent")
set(CHIOCCIA_JUCE_URL "https://github.com/juce-framework/JUCE/archive/refs/tags/${CHIOCCIA_JUCE_TAG}.tar.gz" CACHE STRING "JUCE archive used by FetchContent when Git is not installed")
option(CHIOCCIA_LINUX_MINIMAL_X11 "Linux only: disable optional X11 extensions (XInput2, Xrandr, Xinerama, Xcursor) when their -dev packages are missing" OFF)

# Never use fast-math: NaN/Inf guards rely on std::isfinite.
if(MSVC)
    add_compile_options(/fp:precise)
    set(CHIOCCIA_WARNINGS /W4 /permissive-)
else()
    add_compile_options(-fno-fast-math)
    set(CHIOCCIA_WARNINGS -Wall -Wextra -Wshadow -Wfloat-conversion)
endif()

# ------------------------------------------------------------------------------------------------
# JUCE-independent DSP core
# ------------------------------------------------------------------------------------------------
add_library(chioccia_dsp STATIC
    source/dsp/HalfBandOversampler.cpp
    source/dsp/OpticalCell.cpp
    source/dsp/Detector.cpp
    source/dsp/ColourStage.cpp
    source/dsp/OptoEngine.cpp)
target_include_directories(chioccia_dsp PUBLIC source/dsp)
target_compile_features(chioccia_dsp PUBLIC cxx_std_17)
target_compile_options(chioccia_dsp PRIVATE ${CHIOCCIA_WARNINGS})

# ------------------------------------------------------------------------------------------------
# Tests and measurement tool (no JUCE needed)
# ------------------------------------------------------------------------------------------------
if(CHIOCCIA_BUILD_TESTS)
    enable_testing()
    add_executable(chioccia_core_tests tests/core/CoreTests.cpp)
    target_link_libraries(chioccia_core_tests PRIVATE chioccia_dsp)
    target_compile_options(chioccia_core_tests PRIVATE ${CHIOCCIA_WARNINGS})
    add_test(NAME core_tests COMMAND chioccia_core_tests)

    add_executable(chioccia_measure tools/measure/Measure.cpp)
    target_link_libraries(chioccia_measure PRIVATE chioccia_dsp)
    target_compile_options(chioccia_measure PRIVATE ${CHIOCCIA_WARNINGS})
endif()

# ------------------------------------------------------------------------------------------------
# JUCE plug-in
# ------------------------------------------------------------------------------------------------
if(CHIOCCIA_BUILD_PLUGIN)
    if(CHIOCCIA_JUCE_DIR)
        add_subdirectory(${CHIOCCIA_JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE EXCLUDE_FROM_ALL)
    else()
        include(FetchContent)
        find_package(Git QUIET)
        if(GIT_FOUND)
            FetchContent_Declare(JUCE
                GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
                GIT_TAG        ${CHIOCCIA_JUCE_TAG}
                GIT_SHALLOW    ON)
        else()
            # No Git installed (e.g. a fresh Visual Studio setup): download the release archive.
            message(STATUS "Git not found: downloading JUCE ${CHIOCCIA_JUCE_TAG} as an archive")
            FetchContent_Declare(JUCE URL ${CHIOCCIA_JUCE_URL})
        endif()
        FetchContent_MakeAvailable(JUCE)
    endif()

    set(CHIOCCIA_FORMATS VST3)
    if(APPLE)
        list(APPEND CHIOCCIA_FORMATS AU)
    endif()
    if(CHIOCCIA_BUILD_STANDALONE)
        list(APPEND CHIOCCIA_FORMATS Standalone)
    endif()
    if(CHIOCCIA_ENABLE_VST2)
        if(NOT CHIOCCIA_VST2_SDK_DIR OR NOT EXISTS "${CHIOCCIA_VST2_SDK_DIR}/pluginterfaces/vst2.x/aeffect.h")
            message(FATAL_ERROR "CHIOCCIA_ENABLE_VST2=ON requires CHIOCCIA_VST2_SDK_DIR pointing to a legally obtained VST2 SDK.")
        endif()
        juce_set_vst2_sdk_path("${CHIOCCIA_VST2_SDK_DIR}")
        list(APPEND CHIOCCIA_FORMATS VST)
    endif()

    # NOTE: company name, bundle id and the 4-char codes are placeholders. Change them to values you
    # own before distributing (the plug-in code must be unique per product).
    juce_add_plugin(Chioccia
        PRODUCT_NAME "Chioccia"
        COMPANY_NAME "Pollaio Audio"
        BUNDLE_ID "com.pollaioaudio.chioccia"
        PLUGIN_MANUFACTURER_CODE Plla
        PLUGIN_CODE Chcc
        FORMATS ${CHIOCCIA_FORMATS}
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_CATEGORIES Fx Dynamics
        AU_MAIN_TYPE kAudioUnitType_Effect
        DESCRIPTION "Opto leveling amplifier with parallel-first dry/wet path"
        VERSION ${PROJECT_VERSION})

    juce_add_binary_data(ChiocciaAssets
        NAMESPACE ChiocciaBinary
        SOURCES assets/fonts/LilitaOne-Regular.ttf)

    # Plug-in code shared by the plug-in target and the JUCE-based tests.
    add_library(chioccia_plugin_code INTERFACE)
    target_sources(chioccia_plugin_code INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/plugin/Parameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/plugin/Presets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/plugin/PluginProcessor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/plugin/PluginEditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/gui/ChiocciaLookAndFeel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/gui/FacePlate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/gui/VuMeter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/gui/Controls.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/gui/AdvancedPanel.cpp)
    target_include_directories(chioccia_plugin_code INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/plugin
        ${CMAKE_CURRENT_SOURCE_DIR}/source/gui)
    target_compile_definitions(chioccia_plugin_code INTERFACE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        CHIOCCIA_VERSION_STRING="${PROJECT_VERSION}")
    if(UNIX AND NOT APPLE AND CHIOCCIA_LINUX_MINIMAL_X11)
        target_compile_definitions(chioccia_plugin_code INTERFACE
            JUCE_USE_XINPUT=0 JUCE_USE_XRANDR=0 JUCE_USE_XINERAMA=0 JUCE_USE_XCURSOR=0)
    endif()
    target_link_libraries(chioccia_plugin_code INTERFACE
        chioccia_dsp
        ChiocciaAssets
        juce::juce_audio_processors
        juce::juce_gui_basics)
    if(CHIOCCIA_BUILD_STANDALONE)
        # The standalone wrapper needs audio devices (ALSA headers on Linux).
        target_link_libraries(Chioccia PRIVATE juce::juce_audio_utils)
    endif()

    target_link_libraries(Chioccia PRIVATE
        chioccia_plugin_code
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

    if(CHIOCCIA_BUILD_TESTS)
        juce_add_console_app(chioccia_plugin_tests PRODUCT_NAME "ChiocciaPluginTests")
        target_sources(chioccia_plugin_tests PRIVATE tests/plugin/PluginTests.cpp)
        target_link_libraries(chioccia_plugin_tests PRIVATE
            chioccia_plugin_code
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
        add_test(NAME plugin_tests COMMAND chioccia_plugin_tests)
    endif()
endif()
