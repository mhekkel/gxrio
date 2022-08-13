# Copyright Maarten L. Hekkelman, 2022
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.16)

# set the project name
project(gxrio VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FindPkgConfig)

set(CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "The minimum version of C++ required for this library")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DOCUMENTATION "Build documentation files" OFF)
option(ENABLE_TESTING "Build the unit test applications" OFF)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
elseif(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

if(MSVC)
	# make msvc standards compliant...
	add_compile_options(/permissive-)

	macro(get_WIN32_WINNT version)
		if(WIN32 AND CMAKE_SYSTEM_VERSION)
			set(ver ${CMAKE_SYSTEM_VERSION})
			string(REPLACE "." "" ver ${ver})
			string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

			set(${version} "0x${ver}")
		endif()
	endmacro()

	get_WIN32_WINNT(ver)
	add_definitions(-D_WIN32_WINNT=${ver})
endif()

find_package(ZLIB REQUIRED)
pkg_check_modules(LZMA liblzma REQUIRED)

add_library(gxrio INTERFACE)
add_library(gxrio::gxrio ALIAS gxrio)

target_include_directories(gxrio INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	${ZLIB_INCLUDE_DIR}
)

# adding header sources just helps IDEs
target_sources(gxrio INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/gxrio.hpp
)

target_link_libraries(gxrio INTERFACE ZLIB::ZLIB ${LZMA_LIBRARIES})

set_target_properties(gxrio PROPERTIES PUBLIC_HEADER include/gxrio.hpp)

# installation
set(version_config "${CMAKE_CURRENT_BINARY_DIR}/gxrioConfigVersion.cmake")

set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${version_config}"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion)

install(TARGETS gxrio
	EXPORT gxrioConfig
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gxrio)

export(TARGETS gxrio NAMESPACE gxrio:: FILE gxrioTargets.cmake)

install(DIRECTORY include/gxrio
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	COMPONENT Devel
)

if(WIN32 AND NOT CYGWIN)
	set(CONFIG_LOC CMake)
else()
	set(CONFIG_LOC "${CMAKE_INSTALL_LIBDIR}/cmake/gxrio")
endif()

configure_package_config_file(
	${CMAKE_SOURCE_DIR}/cmake/gxrioConfig.cmake.in
	${CMAKE_SOURCE_DIR}/cmake/gxrioConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gxrio
	PATH_VARS INCLUDE_INSTALL_DIR
)

install(EXPORT gxrioConfig
	FILE gxrioTargets.cmake
	NAMESPACE gxrio::
	DESTINATION ${CONFIG_LOC})
install(
	FILES cmake/gxrioConfig.cmake "${version_config}"
	DESTINATION ${CONFIG_LOC})

if(ENABLE_TESTING)
	enable_testing()

	list(APPEND tests unit-test unit-test-gzip unit-test-xz)

	foreach(TEST IN LISTS tests)
		set(TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/test/${TEST}.cpp")

		add_executable(${TEST} ${TEST_SOURCE})

		target_link_libraries(${TEST} gxrio::gxrio)

		if(MSVC)
			# Specify unwind semantics so that MSVC knowns how to handle exceptions
			target_compile_options(${TEST} PRIVATE /EHsc)
		endif()

		add_test(NAME ${TEST}
			COMMAND $<TARGET_FILE:${TEST}> -- ${CMAKE_SOURCE_DIR}/test)
	endforeach()
endif(ENABLE_TESTING)

# Documentation
if(BUILD_DOCUMENTATION)
	find_program(DOXYGEN doxygen)
	find_program(QUICKBOOK quickbook)
	find_program(XSLTPROC xsltproc)
	find_program(FOP fop)
	find_program(BJAM bjam)

	if(NOT DOXYGEN)
		list(APPEND MISSING_DOC_TOOL_LIST doxygen)
	endif()

	if(NOT QUICKBOOK)
		list(APPEND MISSING_DOC_TOOL_LIST quickbook)
	endif()

	if(NOT XSLTPROC)
		list(APPEND MISSING_DOC_TOOL_LIST xsltproc)
	endif()

	if(NOT FOP)
		list(APPEND MISSING_DOC_TOOL_LIST fop)
	endif()

	if(NOT BJAM)
		list(APPEND MISSING_DOC_TOOL_LIST bjam)
	endif()

	if(DEFINED MISSING_DOC_TOOL_LIST)
		list(JOIN MISSING_DOC_TOOL_LIST ", " MISSING_DOC_TOOLS)
		message(FATAL_ERROR "Cannot create documentation since the following applications could not be found: ${MISSING_DOC_TOOLS}")
	endif()

	make_directory(doc)

	if(UNIX)
		set(QUICKBOOK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/quickbook.sh")

		configure_file(${CMAKE_SOURCE_DIR}/doc/tools/quickbook.sh.in
			${CMAKE_CURRENT_BINARY_DIR}/tmp/quickbook.sh
			@ONLY)
		file(COPY ${CMAKE_CURRENT_BINARY_DIR}/tmp/quickbook.sh
			DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
			FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
		file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/tmp/)
	else()
		set(QUICKBOOK_SCRIPT ${QUICKBOOK})
	endif()

	configure_file(${CMAKE_SOURCE_DIR}/doc/Jamfile.v2.in ${CMAKE_SOURCE_DIR}/doc/Jamfile.v2 @ONLY)

	add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/doc/html/index.html
		${CMAKE_SOURCE_DIR}/doc/html ${CMAKE_SOURCE_DIR}/doc/bin
		${CMAKE_SOURCE_DIR}/doc/autodoc.xml
		COMMAND ${BJAM} ${CMAKE_SOURCE_DIR}/doc
		DEPENDS ${CMAKE_SOURCE_DIR}/doc/Jamfile.v2)
	add_custom_target(doc ALL DEPENDS ${CMAKE_SOURCE_DIR}/doc/html/index.html)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/doc/html DESTINATION ${CMAKE_INSTALL_DOCDIR}/${PACKAGE_NAME})
endif()
