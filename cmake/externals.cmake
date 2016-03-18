cmake_policy(SET CMP0015 OLD)

# lib dir
link_directories("${SOFA_LIB_DIR}")
link_directories("${SOFA_LIB_OS_DIR}")
if (NOT SOFA-EXTERNAL_LIBRARY_DIR STREQUAL "")
    link_directories("${SOFA-EXTERNAL_LIBRARY_DIR}")
endif()

# include dirs
if(WIN32)
    list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${SOFA_INC_DIR}")
endif()
if(NOT SOFA-EXTERNAL_INCLUDE_DIR STREQUAL "")
    list(APPEND GLOBAL_INCLUDE_DIRECTORIES ${SOFA-EXTERNAL_INCLUDE_DIR})
endif()
list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${SOFA_FRAMEWORK_DIR}")
list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${SOFA_MODULES_DIR}")
list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${SOFA_APPLICATIONS_DIR}")
list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${SOFA_APPLICATIONS_PLUGINS_DIR}")
list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${SOFA_EXTLIBS_DIR}")
list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${SOFA_BUILD_DIR}/misc/include") # Generated files


## Boost
if(SOFA-EXTERNAL_BOOST)
    if(NOT SOFA-EXTERNAL_BOOST_PATH STREQUAL "")
        set(BOOST_ROOT ${SOFA-EXTERNAL_BOOST_PATH})
    endif()
	if(WIN32)
		if(BOOST_ROOT)
			file(GLOB ALL_BOOST_SUBDIRS "${BOOST_ROOT}/*")
			if(NOT ALL_BOOST_SUBDIRS STREQUAL "")
			
				if(CMAKE_CL_64)
					set(BOOST_ARCH "64")
				else()
					set(BOOST_ARCH "32")
				endif()
				
				## try to find the proper lib dir according to the current architecture (32 / 64 bit)
				foreach(BOOST_SUBDIR ${ALL_BOOST_SUBDIRS})
					if(IS_DIRECTORY ${BOOST_SUBDIR})
						if(BOOST_SUBDIR MATCHES ".*lib.*")
							if(BOOST_SUBDIR MATCHES ".*${BOOST_ARCH}.*")
								list(APPEND BOOST_LIBRARYDIR "${BOOST_SUBDIR}")
							endif()
						endif()
					endif()
				endforeach()
				
				## if no lib dir has been found according to the current architecture, we assume the user built only the needed one and did not set an architecture prefix
				if(NOT BOOST_LIBRARYDIR)
					foreach(BOOST_SUBDIR ${ALL_BOOST_SUBDIRS})
						if(IS_DIRECTORY ${BOOST_SUBDIR})
							if(BOOST_SUBDIR MATCHES ".*lib.*")
								list(APPEND BOOST_LIBRARYDIR "${BOOST_SUBDIR}")
							endif()
						endif()
					endforeach()
				endif()

				endif()
		endif()
	endif()

	if(SOFA-MISC_STATIC_LINK_BOOST)
		set(Boost_USE_STATIC_LIBS ON)
	else()
		set(Boost_USE_STATIC_LIBS OFF)
	endif()

	# this setting should match the standard library usage of Sofa libraries, not the value of SOFA-MISC_STATIC_LINK_BOOST 
	# ie. on Win32 it should be OFF if we link with msvcrt (/MD) or ON if we link with libcmt (/MT).
	# Sofa links with msvcrt for now (which is the recommended default on Windows).
	set(Boost_USE_STATIC_RUNTIME OFF) 
	
	if(WIN32)
		  # Setting some more suffixes for the library
		set(Boost_LIB_PREFIX "lib") #if you compile BOOST yourself then the lib is appended to the name
	endif()

    set(Boost_USE_MULTITHREADED ON)
    find_package("Boost" REQUIRED COMPONENTS thread graph system)
    set(Boost_LIB_DIAGNOSTIC_DEFINITIONS ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} CACHE INTERNAL "Boost lib diagnostic definitions" FORCE)
    set(GLOBAL_COMPILER_DEFINES ${GLOBAL_COMPILER_DEFINES} BOOST_ALL_NO_LIB CACHE INTERNAL "Global Compiler Defines" FORCE)
    list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}")
#    message( "Boost - lib path: " ${Boost_LIBRARY_DIRS} " - include path: " ${Boost_INCLUDE_DIRS} )
else()
    list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${SOFA-EXTERNAL_BOOST_PATH}")
endif()

# packages and libraries

## opengl / glew / glut
if (NOT SOFA-MISC_NO_OPENGL)
    find_package(OPENGL REQUIRED)
    unset(GLUT_LIBRARIES CACHE) # force remove glut path to be able to switch glut/freeglut
    if(WIN32)
        list(FIND OPENGL_LIBRARIES "glu32" GLU32_INDEX)
        if(GLU32_INDEX EQUAL -1)
            list(APPEND OPENGL_LIBRARIES "glu32")
        endif()
        if(SOFA-EXTERNAL_FREEGLUT)
            set(GLUT_LIBRARIES "freeglut")
        else()
            set(GLUT_LIBRARIES "glut32")
        endif()
        if(SOFA-EXTERNAL_GLEW)
            set(GLEW_LIBRARIES "glew32")
        else()
            set(GLEW_LIBRARIES "")
        endif()
    else()
        if(NOT PS3)
            if (SOFA-EXTERNAL_FREEGLUT)
                find_package(GLUT REQUIRED)
            else()
                if(APPLE)
                    find_library(GLUT_LIBRARIES "GLUT")
                else()
                    find_library(GLUT_LIBRARIES "glut")
                endif()
            endif()
        endif()
        ## GLU
        if(UNIX)
            if(NOT APPLE)
                list(FIND GLUT_LIBRARIES GLU GLU_INDEX)
                if(GLU_INDEX EQUAL -1)
                    list(APPEND GLUT_LIBRARIES GLU)
                endif()
                list(FIND GLUT_LIBRARIES X11 X11_INDEX)
                if(X11_INDEX EQUAL -1)
                    list(APPEND GLUT_LIBRARIES X11)
                endif()
            endif()
            list(FIND GLUT_LIBRARIES dl DL_INDEX)
            if(DL_INDEX EQUAL -1)
                list(APPEND GLUT_LIBRARIES dl)
            endif()
        endif()
        sofa_remove_duplicates(GLUT_LIBRARIES)
        if(SOFA-EXTERNAL_GLEW AND NOT PS3)
            find_package(GLEW REQUIRED)
        else()
            set(GLEW_LIBRARIES "")
        endif()
    endif()
else()
    set(OPENGL_LIBRARIES "")
    set(GLUT_LIBRARIES "")
    set(GLEW_LIBRARIES "")
endif()

## libpng
if (SOFA-EXTERNAL_PNG)
    if(WIN32)
        set(PNG_LIBRARIES "libpng")
    else()
        if(SOFA-EXTERNAL_PNG_SPECIFIC_VERSION)
            set(PNG_LIBRARIES "${SOFA-EXTERNAL_PNG_VERSION}")
        else()
            #find_library(PNG_LIBRARIES "png")
            find_package(PNG REQUIRED)
            list(APPEND GLOBAL_INCLUDE_DIRECTORIES "${PNG_INCLUDE_DIR}")
        endif()
    endif()
    if(PS3)
        set(PNG_LIBRARIES "${SOFA_LIB_OS_DIR}/libpng.a")
        set(PNG_INCLUDE_DIR ${SOFA_INC_DIR})
    endif(PS3)
else()
    set(PNG_LIBRARIES "")
endif()

set(OPENGL_LIBRARIES ${OPENGL_LIBRARIES} CACHE INTERNAL "OpenGL Library")
set(GLEW_LIBRARIES ${GLEW_LIBRARIES} CACHE INTERNAL "GLEW Library")
set(GLUT_LIBRARIES ${GLUT_LIBRARIES} CACHE INTERNAL "GLUT Library")
set(PNG_LIBRARIES ${PNG_LIBRARIES} CACHE INTERNAL "PNG Library")

RegisterProjects(${OPENGL_LIBRARIES})
RegisterProjects(${GLEW_LIBRARIES} OPTION SOFA-EXTERNAL_GLEW COMPILE_DEFINITIONS SOFA_HAVE_GLEW)
if(SOFA-EXTERNAL_FREEGLUT)
    RegisterProjects(${GLUT_LIBRARIES} COMPILE_DEFINITIONS SOFA_HAVE_FREEGLUT)
else()
    RegisterProjects(${GLUT_LIBRARIES})
endif()
RegisterProjects(${PNG_LIBRARIES} OPTION SOFA-EXTERNAL_PNG COMPILE_DEFINITIONS SOFA_HAVE_PNG)


## Zlib (SOFA-EXTERNAL_ZLIB)
# Note: zlib has to be registered AFTER libpng, because zlib is part of PNG_LIBRARIES; this way,
# GLOBAL_PROJECT_OPTION_COMPILERDEFINITIONS_zlib is set to SOFA_HAS_ZLIB (and not SOFA_HAVE_PNG)
# (This is quite a ugly workaround, I agree)
if(WIN32 OR XBOX OR PS3)
    if(PS3)
        set(ZLIB_LIBRARIES "${SOFA_LIB_OS_DIR}/zlib.a")
        set(ZLIB_INCLUDE_DIR ${SOFA_INC_DIR})
    else()
        set(ZLIB_LIBRARIES "zlib")
    endif(PS3)
else()
    find_library(ZLIB_LIBRARIES "z")
endif()
set(ZLIB_LIBRARIES ${ZLIB_LIBRARIES} CACHE INTERNAL "ZLib Library")
if (SOFA-EXTERNAL_ZLIB)
    set(ZLIB_LIBRARIES_OPTIONAL ${ZLIB_LIBRARIES} CACHE INTERNAL "ZLib Library")
else()
    set(ZLIB_LIBRARIES_OPTIONAL "" CACHE INTERNAL "ZLib Library (optional)")
endif()
RegisterProjects(${ZLIB_LIBRARIES} OPTION SOFA-EXTERNAL_ZLIB COMPILE_DEFINITIONS SOFA_HAVE_ZLIB)



# enable unit tests
if(SOFA-MISC_TESTS)
    enable_testing()
endif()