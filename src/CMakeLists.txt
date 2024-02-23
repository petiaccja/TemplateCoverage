add_library(LibTemplateCoverage STATIC)
add_executable(TemplateCoverage)
target_link_libraries(TemplateCoverage LibTemplateCoverage)
set_target_properties(TemplateCoverage PROPERTIES OUTPUT_NAME "template-coverage")

target_sources(TemplateCoverage
    PRIVATE
        main.cpp
)

target_sources(LibTemplateCoverage
    PRIVATE
        Collector.cpp
        Collector.hpp
        Reporter.cpp
        Reporter.hpp
)

find_package(Clang)

target_include_directories(LibTemplateCoverage
    PUBLIC
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)

target_include_directories(LibTemplateCoverage
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(LibTemplateCoverage
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangSerialization
    clangTooling
)

find_package(pugixml)
target_link_libraries(LibTemplateCoverage pugixml::pugixml)