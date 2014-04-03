# - Try to find muParser
# This module finds if MuParser is installed and determines where the include
# files and libraries are. It also determines what the name of the library is.
# This code sets the following variables:
#
#  MUPARSER_FOUND        = System has muParser
#  MUPARSER_INCLUDE_DIRS = The muParser include directories
#  MUPARSER_LIBRARIES    = The libraries needed to use muParser
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

find_path(MUPARSER_INCLUDE_DIR
	NAMES
	muParser.h
	PATH_SUFFIXES
	muparser
	DOC
	"The muParser include directory")

find_library(MUPARSER_LIBRARY NAMES muparser DOC "The muParser binary")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(muParser
	DEFAULT_MSG
	MUPARSER_LIBRARY
	MUPARSER_INCLUDE_DIR)

if(MUPARSER_FOUND)
	set(MUPARSER_LIBRARIES ${MUPARSER_LIBRARY})
	set(MUPARSER_INCLUDE_DIRS ${MUPARSER_INCLUDE_DIR})
endif()

mark_as_advanced(MUPARSER_INCLUDE_DIR MUPARSER_LIBRARY)
