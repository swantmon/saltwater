
cmake_minimum_required(VERSION 3.6.0)

# ---------------------------------------
# Project & setup
# ---------------------------------------
set(InternDir  ../../../../intern/)
set(SourceDir  ../../../../intern/src/)
set(IncludeDir ../../../../intern/src/)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
set(MyLibraries base camera core data graphic gui logic)

include_directories(${IncludeDir})

foreach(MyLibrary IN ITEMS ${MyLibraries})

    set(TargetName ${MyLibrary})
    set(TargetSourceDir ${SourceDir}${TargetName})

    file(
        GLOB_RECURSE TargetSources
        LIST_DIRECTORIES false
        ${TargetSourceDir}/*.c*
        ${TargetSourceDir}/*.h*
    )

    add_library(${TargetName} STATIC ${TargetSources})

endforeach()


# ---------------------------------------
# Java Interface
# ---------------------------------------
include_directories(${CMAKE_BINARY_DIR})
message(STATUS "TEST OUTPUT VAR: ${CMAKE_BINARY_DIR}")

file(
     GLOB_RECURSE TargetSources
     LIST_DIRECTORIES false
     ${SourceDir}app_droid/*.c*
     ${SourceDir}app_droid/*.h*
)

add_library( app_droid SHARED ${SourceDir}app_droid/app_droid.cpp )

target_link_libraries( app_droid
                       ${CMAKE_BINARY_DIR}/libbase.a
                       ${CMAKE_BINARY_DIR}/libcamera.a
                       ${CMAKE_BINARY_DIR}/libcore.a
                       ${CMAKE_BINARY_DIR}/libdata.a
                       ${CMAKE_BINARY_DIR}/libgraphic.a
                       ${CMAKE_BINARY_DIR}/libgui.a
                       ${CMAKE_BINARY_DIR}/liblogic.a
                       log EGL GLESv3 android)


# ---------------------------------------
# Example
# ---------------------------------------
#add_library(native-lib SHARED src/main/cpp/native-lib.cpp)

#find_library(log-lib log)

#target_link_libraries( # Specifies the target library.
#                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
#                       ${log-lib} ${CMAKE_BINARY_DIR}/libbase.a ${CMAKE_BINARY_DIR}/libcore.a)