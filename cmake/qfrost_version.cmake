# Generate something to trigger cmake rerun when VERSION changes
configure_file(${CMAKE_SOURCE_DIR}/VERSION
	${CMAKE_BINARY_DIR}/VERSION.dep)

macro(GET_CONFIG_VALUE keyword var)
	if(NOT ${var})
		file(STRINGS ${CMAKE_SOURCE_DIR}/VERSION str REGEX "^[ ]*${keyword}=")
		if(str)
			string(REPLACE "${keyword}=" "" str ${str})
			string(REGEX REPLACE "[ ].*" "" str "${str}")
			set(${var} ${str})
		endif()
	endif()
endmacro()

macro(GET_VERSION)
	get_config_value("VERSION_MAJOR" MAJOR_VERSION)
	get_config_value("VERSION_MINOR" MINOR_VERSION)
	get_config_value("VERSION_PATCH" PATCH_VERSION)
	if((NOT ${MAJOR_VERSION} MATCHES "^[0-9]+$") OR
	   (NOT ${MINOR_VERSION} MATCHES "^[0-9]+$") OR
	   (NOT ${PATCH_VERSION} MATCHES "^[0-9]+$"))
		message(FATAL_ERROR "VERSION file cannot be parsed.")
	endif()
	set(VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
	message("-- QFrost ${VERSION}")
	mark_as_advanced(VERSION)
endmacro()

get_version()

function(ADD_VERSION_INFO target sources_var)
	if(WIN32)
		set(EXE_NAME ${target})
		#TODO: REGEX REPLACE
		string(REPLACE "../" "" EXE_NAME "${EXE_NAME}")
		set(rcfile ${CMAKE_BINARY_DIR}/win_resources.rc)
		configure_file(${CMAKE_SOURCE_DIR}/res/win_resources.rc.in ${rcfile})
		configure_file(${CMAKE_SOURCE_DIR}/res/qfrost.ico
			${CMAKE_BINARY_DIR}/qfrost.ico
			COPYONLY)
		set(${sources_var} ${${sources_var}} ${rcfile} PARENT_SCOPE)
	endif()
endfunction()
