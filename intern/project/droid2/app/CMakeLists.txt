
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
file(
    GLOB_RECURSE TargetSources
    LIST_DIRECTORIES false
    ${SourceDir}app_droid/*.c*
    ${SourceDir}app_droid/*.h*
)

add_library( app_droid SHARED ${TargetSources} )

target_link_libraries( app_droid ${MyLibraries} log GLESv3 EGL android )