cmake_minimum_required(VERSION 3.25.0)

project(TemplateCoverage)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Cpp flags
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)

option(ENABLE_LLVM_COV "Adds compiler flags to generate LLVM source-based code coverage. Only works with Clang." OFF)
option(ENABLE_LLVM_ADDRESS_SANITIZER "Adds compiler flags to generate LLVM source-based code coverage. Only works with Clang." OFF)
option(ENABLE_LLVM_MEMORY_SANITIZER "Adds compiler flags to generate LLVM source-based code coverage. Only works with Clang." OFF)
set(EXTERNAL_LLVM_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Same as CMAKE_BUILD_TYPE, but applied to LLVM when downloading it.")
option(EXTERNAL_LLVM_STRIP_DEBUG_INFO "Compiles the external LLVM without debug info, mainly to reduce its size." OFF)
option(EXTERNAL_LLVM_IGNORE_SYSTEM "Forces a local compilation of LLVM from sources." OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	if (ENABLE_LLVM_COV)
		add_compile_options("-fprofile-instr-generate" "-fcoverage-mapping" "-mllvm" "-enable-name-compression=false")
		add_link_options("-fprofile-instr-generate" "-fcoverage-mapping")
	endif()
	if (ENABLE_LLVM_ADDRESS_SANITIZER)
		add_compile_options("-fsanitize=address")
		add_link_options("-fsanitize=address")
	endif()
	if (ENABLE_LLVM_MEMORY_SANITIZER)
		add_compile_options("-fsanitize=memory")
		add_link_options("-fsanitize=memory")
	endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
	add_compile_options("/MP")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

# LLVM dependency
if (NOT EXTERNAL_LLVM_IGNORE_SYSTEM)
	find_package(Clang CONFIG QUIET)
	find_package(LLVM CONFIG QUIET)
endif()
if(NOT Clang_FOUND OR NOT LLVM_FOUND)
	set(EXTERNAL_LLVM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/LLVM")
	set(EXTERNAL_LLVM_BINARY_DIR "${CMAKE_BINARY_DIR}/externals/LLVM")
	set(Clang_SEARCH_PATH "${EXTERNAL_LLVM_BINARY_DIR}/install/lib/cmake/clang")
	set(LLVM_SEARCH_PATH "${EXTERNAL_LLVM_BINARY_DIR}/install/lib/cmake/llvm")

	find_package(Clang CONFIG QUIET PATHS "${Clang_SEARCH_PATH}" NO_DEFAULT_PATH)
	if(NOT Clang_FOUND)
		message("Installing LLVM from sources... this might take a while.")
		message("You can also set the Clang_DIR and LLVM_DIR environment variables and reconfigure.")

		file(MAKE_DIRECTORY "${EXTERNAL_LLVM_BINARY_DIR}")
		execute_process(
			COMMAND "${CMAKE_COMMAND}"
				-S "${EXTERNAL_LLVM_SOURCE_DIR}"
				-B "${EXTERNAL_LLVM_BINARY_DIR}"
				"-DCMAKE_BUILD_TYPE:STRING=${EXTERNAL_LLVM_BUILD_TYPE}"
				"-DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
				"-DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
				"-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
				"-DSTRIP_DEBUG_INFO:BOOL=${EXTERNAL_LLVM_STRIP_DEBUG_INFO}"
				"-G" ${CMAKE_GENERATOR}
			WORKING_DIRECTORY "${EXTERNAL_LLVM_BINARY_DIR}"
			COMMAND_ERROR_IS_FATAL ANY
		)
		execute_process(
			COMMAND ${CMAKE_COMMAND} --build "${EXTERNAL_LLVM_BINARY_DIR}" --parallel
			WORKING_DIRECTORY "${EXTERNAL_LLVM_BINARY_DIR}"
			COMMAND_ERROR_IS_FATAL ANY
		)
	endif()
endif()
find_package(Clang REQUIRED CONFIG PATHS "${Clang_SEARCH_PATH}" NO_DEFAULT_PATH)
find_package(LLVM REQUIRED CONFIG PATHS "${LLVM_SEARCH_PATH}" NO_DEFAULT_PATH)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Subdirectories
add_subdirectory(src)
add_subdirectory(test)

# Distribution
set_target_properties(TemplateCoverage PROPERTIES INSTALL_RPATH "$ORIGIN")
install(
	TARGETS TemplateCoverage DESTINATION "."
	RUNTIME_DEPENDENCIES
		PRE_EXCLUDE_REGEXES "ext-ms-.*" "api-ms-.*" "hvsifiletrust.dll" "pdmutilities.dll"
		DIRECTORIES ${EXTRA_RUNTIME_DEPENDENCY_DIRS}
		POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
		DESTINATION "."
)