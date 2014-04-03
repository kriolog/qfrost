# - FindMathGL.cmake
# This module can be used to find MathGL and several of its optional components.
#
# You can specify one or more component as you call this find module.
# Possible components are: FLTK, GLUT, Qt, WX.
#
# The following variables will be defined for your use:
#
#  MATHGL_FOUND           = MathGL and all specified components found
#  MATHGL_INCLUDE_DIRS    = The MathGL include directories
#  MATHGL_LIBRARIES       = The libraries to link against to use MathGL
#                           and all specified components
#  MATHGL_VERSION_STRING  = A human-readable version of the MathGL (e.g. 1.11)
#  MATHGL_XXX_FOUND       = Component XXX found (replace XXX with uppercased
#                           component name -- for example, QT or FLTK)
#
# The minimum required version and needed components can be specified using
# the standard find_package()-syntax, here are some examples:
#  find_package(MathGL 1.11 Qt REQUIRED) - 1.11 + Qt interface, required
#  find_package(MathGL 1.10 REQUIRED)    - 1.10 (no interfaces), required
#  find_package(MathGL 1.10 Qt WX)       - 1.10 + Qt and WX interfaces, optional
#  find_package(MathGL 1.11)             - 1.11 (no interfaces), optional
#
# Typical usage could be something like this:
#   find_package(MathGL 1.11 GLUT REQUIRED)
#   include_directories(${MATHGL_INCLUDE_DIRS})
#   add_executable(myexe main.cpp)
#   target_link_libraries(myexe ${MATHGL_LIBRARIES})
#

#=============================================================================
# Copyright (c) 2011 Denis Pesotsky <denis@kde.ru>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file COPYING-CMAKE-MODULES for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

find_path(MATHGL_INCLUDE_DIR
	NAMES
	mgl/mgl.h
	DOC
	"The MathGL include directory")
find_library(MATHGL_LIBRARY
	NAMES
	mgl
	PATHS
	${MATHGL_LIBRARY_DIR}
	DOC
	"The MathGL include directory")

get_filename_component(MATHGL_LIBRARY_DIR ${MATHGL_LIBRARY} PATH)

set(MATHGL_LIBRARIES ${MATHGL_LIBRARY})
set(MATHGL_INCLUDE_DIRS ${MATHGL_INCLUDE_DIR})

if(MATHGL_INCLUDE_DIR)
	set(_CONFIG_FILE_NAME "mgl/config.h")
	set(_CONFIG_FILE_PATH "${MATHGL_INCLUDE_DIR}/${_CONFIG_FILE_NAME}")
	set(_VERSION_ERR "Cannot determine MathGL version")
	if(EXISTS "${_CONFIG_FILE_PATH}")
		file(STRINGS
			"${_CONFIG_FILE_PATH}"
			MATHGL_VERSION_STRING
			REGEX
			"^#define PACKAGE_VERSION \"[^\"]*\"$")
		if(MATHGL_VERSION_STRING)
			string(REGEX
				REPLACE
				"^#define PACKAGE_VERSION \"([^\"]*)\"$"
				"\\1"
				MATHGL_VERSION_STRING
				${MATHGL_VERSION_STRING})
		else()
			message(FATAL_ERROR
				"${_VERSION_ERR}: ${_CONFIG_FILE_NAME} parse error")
		endif()
	else()
		message(FATAL_ERROR "${_VERSION_ERR}: ${_CONFIG_FILE_NAME} not found")
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MathGL
	REQUIRED_VARS
	MATHGL_LIBRARY
	MATHGL_INCLUDE_DIR
	VERSION_VAR
	MATHGL_VERSION_STRING)

foreach(_Component ${MathGL_FIND_COMPONENTS})
	string(TOLOWER ${_Component} _component)
	string(TOUPPER ${_Component} _COMPONENT)

	set(MathGL_${_Component}_FIND_REQUIRED ${MathGL_FIND_REQUIRED})
	set(MathGL_${_Component}_FIND_QUIETLY true)

	find_path(MATHGL_${_COMPONENT}_INCLUDE_DIR
		NAMES
		mgl/mgl_${_component}.h
		PATHS
		${MATHGL_INCLUDE_DIR}
		NO_DEFAULT_PATH)
	find_library(MATHGL_${_COMPONENT}_LIBRARY
		NAMES
		mgl-${_component}
		PATHS
		${MATHGL_LIBRARY_DIR}
		NO_DEFAULT_PATH)

	find_package_handle_standard_args(MathGL_${_Component}
		DEFAULT_MSG
		MATHGL_${_COMPONENT}_LIBRARY
		MATHGL_${_COMPONENT}_INCLUDE_DIR)

	if(MATHGL_${_COMPONENT}_FOUND)
		set(MATHGL_LIBRARIES
			${MATHGL_LIBRARIES}
			${MATHGL_${_COMPONENT}_LIBRARY})
		set(MATHGL_INCLUDE_DIRS
			${MATHGL_INCLUDE_DIRS}
			${MATHGL_${_COMPONENT}_INCLUDE_DIR})
	endif()

	mark_as_advanced(MATHGL_${_COMPONENT}_INCLUDE_DIR
		MATHGL_${_COMPONENT}_LIBRARY)
endforeach()

mark_as_advanced(MATHGL_INCLUDE_DIR MATHGL_LIBRARY)
