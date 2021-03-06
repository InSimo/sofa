#-------------------------------------------------------------------------------------------
# Copyright (C) Electronic Arts Inc.  All rights reserved.
#-------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)
project(EABase CXX)

#-------------------------------------------------------------------------------------------
# Source files
#-------------------------------------------------------------------------------------------
file(GLOB EABASE_SOURCES "include/Common/EABase/*.h" "include/Common/EABase/config/*.h")

#-------------------------------------------------------------------------------------------
# Header only library
#-------------------------------------------------------------------------------------------
add_library(EABase INTERFACE)

#-------------------------------------------------------------------------------------------
# exported include directories 
#-------------------------------------------------------------------------------------------
target_include_directories(EABase INTERFACE include/Common)

# HACK: Workaround CMake limitations to have files of interface / header-only
#       libaries show up in IDEs
# http://stackoverflow.com/questions/27039019/how-to-have-cmake-show-headers-that-are-not-part-of-any-binary-target-in-the-ide/29218394#29218394
add_custom_target(EABase_ide SOURCES ${EABASE_SOURCES})

install(TARGETS "${PROJECT_NAME}"
        EXPORT  SofaFrameworkTargets
        RUNTIME DESTINATION "${EASTL_INSTALL_BIN_DIR}"
        LIBRARY DESTINATION "${EASTL_INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${EASTL_INSTALL_LIB_DIR}"
    )
