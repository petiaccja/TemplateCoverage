add_executable(Test)

target_sources(Test
    PRIVATE
        main.cpp
        Test_Collector.cpp
        Test_Reporter.cpp
)

target_compile_definitions(Test PRIVATE "TEST_INPUT_DIR=\"${CMAKE_CURRENT_LIST_DIR}/input\"")

find_package(Catch2)
target_link_libraries(Test Catch2::Catch2)
target_link_libraries(Test LibTemplateCoverage)