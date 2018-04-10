
cmake_minimum_required(VERSION 3.6.0)

# ---------------------------------------
# Import ARCore
# ---------------------------------------
add_library(arcore SHARED IMPORTED)

set_target_properties(arcore PROPERTIES IMPORTED_LOCATION ${EXTERNAL_LIBPATH}/${ANDROID_ABI}/libarcore_sdk.so)

# ---------------------------------------
# Native App GLUE
# ---------------------------------------
add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# ---------------------------------------
# Project & setup
# ---------------------------------------
set( ExternDir  ../../../../extern/ )
set( InternDir  ../../../../intern/ )
set( SourceDir  ../../../../intern/src/ )
set( IncludeDir ../../../../intern/src/ )

set( CMAKE_INCLUDE_CURRENT_DIR ON )

# ---------------------------------------
# Base
# ---------------------------------------
include_directories( ${IncludeDir} )

file( GLOB TargetSources ${SourceDir}base/*.c* ${SourceDir}base/*.h*)

add_library( base STATIC ${TargetSources} )

target_include_directories( base PRIVATE ${ExternDir}/glm/include )
target_include_directories( base PRIVATE ${ExternDir}/json/include )

# ---------------------------------------
# Engine
# ---------------------------------------
set( MyLibraries camera graphic logic script core data gui )

include_directories( ${IncludeDir} )

link_directories ( ${ExternDir}/assimp/lib/${ANDROID_ABI} )
link_directories ( ${ExternDir}/devil/lib/${ANDROID_ABI} )
link_directories ( ${ExternDir}/tinyxml2/lib/${ANDROID_ABI} )

file( GLOB TargetSourcesCamera ${SourceDir}camera/*.c* ${SourceDir}camera/*.h* )
file( GLOB TargetSourcesGraphic ${SourceDir}graphic/*.c* ${SourceDir}graphic/*.h* )
file( GLOB TargetSourcesLogic ${SourceDir}logic/*.c* ${SourceDir}logic/*.h* )
file( GLOB TargetSourcesScript ${SourceDir}script/*.c* ${SourceDir}script/*.h* )
file( GLOB TargetSourcesCore ${SourceDir}core/*.c* ${SourceDir}core/*.h* )
file( GLOB TargetSourcesData ${SourceDir}data/*.c* ${SourceDir}data/*.h* )
file( GLOB TargetSourcesGUI ${SourceDir}gui/*.c* ${SourceDir}gui/*.h* )
file( GLOB TargetSourcesEngine ${SourceDir}engine/*.c* ${SourceDir}engine/*.h* )

add_library( engine SHARED ${TargetSourcesCamera} ${TargetSourcesGraphic} ${TargetSourcesLogic} ${TargetSourcesScript} ${TargetSourcesCore} ${TargetSourcesData} ${TargetSourcesGUI} ${TargetSourcesEngine} )

target_include_directories( engine PRIVATE ${ExternDir}/glm/include )
target_include_directories( engine PRIVATE ${ExternDir}/json/include )
target_include_directories( engine PRIVATE ${ExternDir}/assimp/include )
target_include_directories( engine PRIVATE ${ExternDir}/devil/include )
target_include_directories( engine PRIVATE ${ExternDir}/tinyxml2/include )

target_link_libraries( engine log android m EGL GLESv3 )
target_link_libraries( engine assimp IL ILU tinyxml2 )
target_link_libraries( engine base)

# ---------------------------------------
# Plugins
# 01. plugin_arcore
# ---------------------------------------
set( MyLibraries plugin_arcore )

include_directories( ${IncludeDir} )

link_directories ( ${ExternDir}/assimp/lib/${ANDROID_ABI} )
link_directories ( ${ExternDir}/devil/lib/${ANDROID_ABI} )
link_directories ( ${ExternDir}/tinyxml2/lib/${ANDROID_ABI} )

foreach( MyLibrary IN ITEMS ${MyLibraries} )

    set( TargetName ${MyLibrary} )
    set( TargetSourceDir ${SourceDir}${TargetName} )

    file(
        GLOB_RECURSE TargetSources
        LIST_DIRECTORIES false
        ${TargetSourceDir}/*.c*
        ${TargetSourceDir}/*.h*
    )

    add_library( ${TargetName} SHARED ${TargetSources} )

    target_include_directories( ${TargetName} PRIVATE ${ExternDir}/glm/include )
    target_include_directories( ${TargetName} PRIVATE ${ExternDir}/json/include )

    target_link_libraries( ${TargetName} log android m )
    target_link_libraries( ${TargetName} tinyxml2 )
    target_link_libraries( ${TargetName} base engine )

endforeach()

target_include_directories( plugin_arcore PRIVATE ${ExternDir}/arcore/include )

target_link_libraries( plugin_arcore arcore)

# ---------------------------------------
# Java Interface
# ---------------------------------------
link_directories ( ${ExternDir}/assimp/lib/${ANDROID_ABI} )
link_directories ( ${ExternDir}/devil/lib/${ANDROID_ABI} )
link_directories ( ${ExternDir}/tinyxml2/lib/${ANDROID_ABI} )

file(
     GLOB_RECURSE TargetSources
     LIST_DIRECTORIES false
     ${SourceDir}app_droid/*.c*
     ${SourceDir}app_droid/*.h*
)

add_library( app_droid SHARED ${TargetSources} )

target_include_directories( app_droid PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue )
target_include_directories( app_droid PRIVATE ${ExternDir}/glm/include )
target_include_directories( app_droid PRIVATE ${ExternDir}/json/include )

target_link_libraries( app_droid log android native_app_glue )
target_link_libraries( app_droid tinyxml2 )
target_link_libraries( app_droid base engine )