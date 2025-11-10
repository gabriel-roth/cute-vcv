cmake_minimum_required(VERSION 3.22)

# Set the path to the Plugin SDK using a cmake variable

if(NOT "${METAMODULE_SDK_DIR}" STREQUAL "")
	message("METAMODULE_SDK_DIR set by CMake variable ${METAMODULE_SDK_DIR}")
elseif (DEFINED ENV{METAMODULE_SDK_DIR})
    set(METAMODULE_SDK_DIR "$ENV{METAMODULE_SDK_DIR}")
	message("METAMODULE_SDK_DIR set by environment variable ${METAMODULE_SDK_DIR}")
else()
    set(METAMODULE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../metamodule-plugin-sdk")
	message("METAMODULE_SDK_DIR set to default: ${METAMODULE_SDK_DIR}")
endif()

include(${METAMODULE_SDK_DIR}/plugin.cmake)


project(cute-vcv VERSION 1.0 DESCRIPTION "CuteLab Plugin for MetaModule" LANGUAGES C CXX ASM)

# Add sources from the Rack source repo
set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

add_library(cute-vcv STATIC ${CUTE_VCV_SOURCES})

target_sources(cute-vcv PRIVATE
    ${SOURCE_DIR}/src/plugin.cpp 
    ${SOURCE_DIR}/src/mom-jeans.cpp
    ${SOURCE_DIR}/cores/oscillator/lut/lut_saw_uint8.c
    ${SOURCE_DIR}/cores/oscillator/lut/lut_saw_uint16.c
    ${SOURCE_DIR}/cores/oscillator/lut/lut_sin_uint8.c
    ${SOURCE_DIR}/cores/oscillator/lut/lut_sin_uint16.c
    ${SOURCE_DIR}/cores/oscillator/lut/lut_square_uint8.c
    ${SOURCE_DIR}/cores/oscillator/lut/lut_square_uint16.c
    ${SOURCE_DIR}/cores/oscillator/lut/lut_tri_uint8.c
    ${SOURCE_DIR}/cores/oscillator/lut/lut_tri_uint16.c
    ${SOURCE_DIR}/cores/oscillator/lut/lut.h
    ${SOURCE_DIR}/cores/oscillator/oscillator.c
    ${SOURCE_DIR}/cores/oscillator/oscillator.h
    ${SOURCE_DIR}/cores/phasor/phasor.c
    ${SOURCE_DIR}/cores/phasor/phasor.h
    ${SOURCE_DIR}/cores/pulsar/antialiased_oscillator.c
    ${SOURCE_DIR}/cores/pulsar/antialiased_oscillator.h
    ${SOURCE_DIR}/cores/pulsar/biquad.c
    ${SOURCE_DIR}/cores/pulsar/biquad.h
    ${SOURCE_DIR}/cores/pulsar/minblep_lut.c
    ${SOURCE_DIR}/cores/pulsar/minblep_lut.h
    ${SOURCE_DIR}/cores/pulsar/minblep.c
    ${SOURCE_DIR}/cores/pulsar/minblep.h
    ${SOURCE_DIR}/cores/pulsar/pulsar_lut.c
    ${SOURCE_DIR}/cores/pulsar/pulsar_lut.h
    ${SOURCE_DIR}/cores/pulsar/pulsar_oscillator.c
    ${SOURCE_DIR}/cores/pulsar/pulsar_oscillator.h
    ${SOURCE_DIR}/cores/pulsar/pulsar.c
    ${SOURCE_DIR}/cores/pulsar/pulsar.h
    ${SOURCE_DIR}/cores/utils.h  
    )
	
# Add includes and compile options for source repo
target_include_directories(cute-vcv PRIVATE ${SOURCE_DIR}/src)
target_include_directories(cute-vcv PRIVATE ${SOURCE_DIR}/cores/pulsar)
target_include_directories(cute-vcv PRIVATE ${SOURCE_DIR}/cores/phasor)
target_include_directories(cute-vcv PRIVATE ${SOURCE_DIR}/cores/oscillator)
target_include_directories(cute-vcv PRIVATE ${SOURCE_DIR}/cores/oscillator/lut)

# Call this to link and create the plugin file
create_plugin(
    SOURCE_LIB      cute-vcv                                        # The cmake target name (defined in add_target)
    PLUGIN_NAME     CuteLab                                         # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${SOURCE_DIR}/plugin.json                       # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets                # Path to the assets/ dir containing the PNGs
    DESTINATION     ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugins    # Path to where you want the plugin file output
)