cmake_minimum_required(VERSION 3.20)

project(ExternalLLVM)

include(ExternalProject)


function(remove_debug_flags out_name input_flags)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" MATCHES "GNU")
        set(search_expr "------------")
        set(replace_expr "")
        set(append_expr "-g0")
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel")
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC" OR "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" MATCHES "MSVC")
        set(search_expr "(/Z7|/Zi|/ZI)[ \t\n]*")
        set(replace_expr "")
        set(append_expr "")
    endif()
    string(REGEX REPLACE "${search_expr}" "${replace_expr}" stripped_flags "${input_flags}")
    set(${out_name} "${stripped_flags} ${append_expr}" PARENT_SCOPE)
endfunction()

if (${STRIP_DEBUG_INFO})
    remove_debug_flags(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    remove_debug_flags(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    remove_debug_flags(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    remove_debug_flags(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
    remove_debug_flags(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

set(llvm_cmake_args
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
    "-DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
    "-DCMAKE_CXX_STANDARD:STRING=20"
    "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}"
    "-DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}"
    "-DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
    "-DCMAKE_CXX_FLAGS_MINSIZEREL:STRING=${CMAKE_CXX_FLAGS_MINSIZEREL}"
    "-DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}"
    "-G" "${CMAKE_GENERATOR}"
)

set(llvm_cache_args
    "-DLLVM_ENABLE_PROJECTS:STRING=clang"
    "-DLLVM_TARGETS_TO_BUILD:STRING=X86"
    "-DLLVM_BUILD_TOOLS:BOOL=OFF"
    "-DLLVM_ENABLE_BINDINGS:BOOL=OFF"
    "-DLLVM_ENABLE_LLD:BOOL=OFF"
    "-DLLVM_ENABLE_RTTI:BOOL=ON"
    "-DLLVM_ENABLE_EH:BOOL=ON"
    "-DLLVM_USE_PERF:BOOL=OFF"
    "-DLLVM_USE_INTEL_JITEVENTS:BOOL=OFF"
    "-DLLVM_OPTIMIZED_TABLEGEN:BOOL=ON"
)

set(LLVM_PROJECT_PREFIX "${CMAKE_BINARY_DIR}/llvm-project")

ExternalProject_Add(llvm-project
    URL "https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/llvm-project-17.0.6.src.tar.xz"
    SOURCE_SUBDIR llvm
    PREFIX "${LLVM_PROJECT_PREFIX}"
    STAMP_DIR "${LLVM_PROJECT_PREFIX}/stamp"
    DOWNLOAD_DIR "${LLVM_PROJECT_PREFIX}/download"
    SOURCE_DIR "${LLVM_PROJECT_PREFIX}/src"
    BINARY_DIR "${LLVM_PROJECT_PREFIX}/build"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
    CMAKE_ARGS ${llvm_cmake_args} "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install"
    CMAKE_CACHE_ARGS ${llvm_cache_args}
    BUILD_ALWAYS OFF
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

add_executable(ExternalLLVM ExternalLLVM.cpp)