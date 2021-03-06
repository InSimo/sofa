cmake_minimum_required(VERSION 2.8.12)
set(SOLUTION_NAME "SofaGeneral")
project(${SOLUTION_NAME})

#temporary until we move the sub projects
get_filename_component(SofaGeneral_PARENT_SOURCE_DIR ${SofaGeneral_SOURCE_DIR} DIRECTORY)
set(SofaGeneral_ACTUAL_SOURCE_DIR ${SofaGeneral_PARENT_SOURCE_DIR}/modules)
set(SOFAGENERAL_SOURCE_INCLUDE_DIR ${SofaGeneral_PARENT_SOURCE_DIR}/modules)

##
## SofaGeneral version
##
set( SOFAGENERAL_MAJOR_VERSION 0 )
set( SOFAGENERAL_MINOR_VERSION 0 )
set( SOFAGENERAL_BUILD_VERSION 1 )

set( SOFAGENERAL_VERSION ${SOFAGENERAL_MAJOR_VERSION}.${SOFAGENERAL_MINOR_VERSION}.${SOFAGENERAL_BUILD_VERSION} CACHE INTERNAL "Set to internal so it can be changed by sub directories.")

set(SOFAGENERAL_BUILD_LIB_DIR     ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(SOFAGENERAL_BUILD_BIN_DIR     ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(SOFAGENERAL_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SOFAGENERAL_BUILD_CMAKE_DIR   ${CMAKE_BINARY_DIR}/SofaGeneral)

# Offer the user the choice of overriding the installation directories
set(SOFAGENERAL_INSTALL_LIB_DIR     ${CMAKE_INSTALL_LIBDIR}     CACHE PATH "Installation directory for libraries")
set(SOFAGENERAL_INSTALL_BIN_DIR     ${CMAKE_INSTALL_BINDIR}     CACHE PATH "Installation directory for executables")
set(SOFAGENERAL_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")
set(SOFAGENERAL_INSTALL_CMAKE_DIR   ${SOFAGENERAL_INSTALL_LIB_DIR}/cmake/SofaGeneral )

## SofaGeneral dependencies
##
if( NOT TARGET SofaComponentCommon )
    find_package(SofaCommon REQUIRED)
endif()

if( TARGET newmat )
    set(Newmat_FOUND TRUE)
else()
    find_package(Newmat)
endif()
if( TARGET csparse )
    set(Csparse_FOUND TRUE)
else()
    find_package(Csparse)
endif()
if( TARGET metis )
    set(METIS_FOUND TRUE)
else()
    find_package(Metis)
endif()

set(SOFA_HAVE_NEWMAT ${Newmat_FOUND} CACHE INTERNAL "")
set(SOFA_HAVE_CSPARSE ${Csparse_FOUND} CACHE INTERNAL "")
set(SOFA_HAVE_METIS ${METIS_FOUND} CACHE INTERNAL "")

set(SOFAGENERAL_DEPENDENCY_INCLUDE_DIR ${SofaCommon_INCLUDE_DIRS})


configure_file( ${SofaGeneral_SOURCE_DIR}/SofaGeneral.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/sofa/SofaGeneral.h )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/sofa/SofaGeneral.h
         DESTINATION "${SOFAGENERAL_INSTALL_INCLUDE_DIR}/sofa")

set(SOFAGENERAL_TARGETS SofaValidation SofaExporter SofaGraphComponent SofaTopologyMapping SofaBoundaryCondition 
    SofaComponentGeneral SofaEngine SofaUserInteraction SofaConstraint SofaHaptics)

if(SOFA_OPTIONAL)
  set(SOFAGENERAL_TARGETS ${SOFAGENERAL_TARGETS} SofaSparseSolver SofaPreconditioner)
endif()

if(SOFA_HAVE_NEWMAT)
  list(APPEND SOFAGENERAL_DEPENDENCY_LINK newmat )
  list(APPEND SOFAGENERAL_TARGETS SofaDenseSolver)
endif()

if(SOFA_HAVE_CSPARSE)
   list(APPEND SOFAGENERAL_DEPENDENCY_LINK csparse)
endif()

if(NOT SOFA_NO_OPENGL)
   list(APPEND SOFAGENERAL_TARGETS SofaOpenglVisual)
endif()

if(SOFA_HAVE_TAUCS)
   list(APPEND SOFAGENERAL_TARGETS SofaTaucsSolver)
endif()
if(SOFA_HAVE_EIGEN2)
   list(APPEND SOFAGENERAL_TARGETS SofaEigen2Solver)
endif()

foreach(TARGET ${SOFAGENERAL_TARGETS})
  add_subdirectory(${SofaGeneral_ACTUAL_SOURCE_DIR}/${TARGET} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET})
  # set IDE project filter
  if(MSVC)
    set_target_properties(${TARGET} PROPERTIES FOLDER "SofaGeneral")
  endif()
endforeach()

include(CMakePackageConfigHelpers)

# SofaGeneralConfigVersion.cmake file (same for build tree and intall)
write_basic_package_version_file(${SOFAGENERAL_BUILD_CMAKE_DIR}/SofaGeneralConfigVersion.cmake
                                 VERSION ${SOFAGENERAL_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${SOFAGENERAL_BUILD_CMAKE_DIR}/SofaGeneralConfigVersion.cmake
        DESTINATION ${SOFAGENERAL_INSTALL_CMAKE_DIR})

# SofaGeneralConfig.cmake (build tree)
set(SOFAGENERAL_INCLUDE_DIR "${SOFAGENERAL_BUILD_INCLUDE_DIR}")
configure_package_config_file(SofaGeneralConfig.cmake.in
                              ${SOFAGENERAL_BUILD_CMAKE_DIR}/SofaGeneralConfig.cmake
                              INSTALL_DESTINATION ${SOFAGENERAL_BUILD_CMAKE_DIR}
                              INSTALL_PREFIX ${CMAKE_BINARY_DIR}
                              PATH_VARS SOFAGENERAL_INCLUDE_DIR)

# SofaGeneralConfig.cmake (installed)
set(SOFAGENERAL_INCLUDE_DIR "${SOFAGENERAL_INSTALL_INCLUDE_DIR}")
configure_package_config_file(SofaGeneralConfig.cmake.in
                              ${SOFAGENERAL_BUILD_CMAKE_DIR}/SofaGeneralConfigInstall.cmake
                              INSTALL_DESTINATION ${SOFAGENERAL_INSTALL_CMAKE_DIR}
                              PATH_VARS SOFAGENERAL_INCLUDE_DIR )

install(FILES ${SOFAGENERAL_BUILD_CMAKE_DIR}/SofaGeneralConfigInstall.cmake
        DESTINATION ${SOFAGENERAL_INSTALL_CMAKE_DIR}
        RENAME SofaGeneralConfig.cmake)

# SofaGeneralTargets.cmake (installed)
install(EXPORT SofaGeneralTargets
        DESTINATION ${SOFAGENERAL_INSTALL_CMAKE_DIR} )

# compatibility with find_package
set(SofaGeneral_LIBRARIES SofaComponentGeneral CACHE INTERNAL "")
set(SOFAGENERAL_INCLUDE_DIR "$<BUILD_INTERFACE:${SOFAGENERAL_BUILD_INCLUDE_DIR}>$<INSTALL_INTERFACE:${SOFAGENERAL_INSTALL_INCLUDE_DIR}>" CACHE INTERNAL "")
set(SofaGeneral_INCLUDE_DIRS ${SOFAGENERAL_INCLUDE_DIR} ${SofaCommon_INCLUDE_DIRS} CACHE INTERNAL "")
