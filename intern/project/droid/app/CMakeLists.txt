
cmake_minimum_required(VERSION 3.6.0)

# ---------------------------------------
# Native App GLUE
# ---------------------------------------
add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# ---------------------------------------
# Project & setup
# ---------------------------------------
set( InternDir  ../../../../intern/ )
set( SourceDir  ../../../../intern/src/ )
set( IncludeDir ../../../../intern/src/ )

set( CMAKE_INCLUDE_CURRENT_DIR ON )

# ---------------------------------------
# Internal libaries
# 01. base
# 02. camera
# 03. core
# 04. data
# 05. graphic
# 06. gui
# 07. logic
# ---------------------------------------
set( MyLibraries camera graphic logic core data gui base )

include_directories( ${IncludeDir} )

foreach( MyLibrary IN ITEMS ${MyLibraries} )

    set( TargetName ${MyLibrary} )
    set( TargetSourceDir ${SourceDir}${TargetName} )

    file(
        GLOB_RECURSE TargetSources
        LIST_DIRECTORIES false
        ${TargetSourceDir}/*.c*
        ${TargetSourceDir}/*.h*
    )

    add_library( ${TargetName} STATIC ${TargetSources} )

endforeach()

# ---------------------------------------
# Java Interface
# ---------------------------------------
file(
     GLOB_RECURSE TargetSources
     LIST_DIRECTORIES false
     ${SourceDir}app_droid/*.c*
     ${SourceDir}app_droid/*.h*
)

add_library( app_droid SHARED ${TargetSources} )

target_include_directories( app_droid PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue )

target_link_libraries( app_droid EGL GLESv3 log android m native_app_glue)

target_link_libraries( app_droid ${MyLibraries} )