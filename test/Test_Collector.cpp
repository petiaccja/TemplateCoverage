//------------------------------------------------------------------------------
// The MIT License (MIT)
// Copyright (c) 2024 Péter Kardos
//------------------------------------------------------------------------------

#include <Collector.hpp>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>


using namespace clang::tooling;
using namespace llvm;
using namespace clang;


static llvm::cl::OptionCategory templateCoverageCategory("template-coverage options");


std::pair<std::shared_ptr<ClangTool>, std::shared_ptr<CommonOptionsParser>> CreateTool(std::string_view file) {
    const auto path = std::filesystem::path(TEST_INPUT_DIR) / file;
    const auto pathStr = path.string();
    std::array argv = {
        static_cast<const char*>("template-coverage"),
        static_cast<const char*>(pathStr.data()),
        static_cast<const char*>("--"),
    };
    int argc = int(argv.size());
    auto maybeParser = CommonOptionsParser::create(argc, argv.data(), templateCoverageCategory);
    if (!maybeParser) {
        llvm::errs() << maybeParser.takeError();
        throw std::logic_error("failed to parse");
    }
    const auto optionsParser = std::make_shared<CommonOptionsParser>(std::move(maybeParser.get()));
    const auto tool = std::make_shared<ClangTool>(optionsParser->getCompilations(), optionsParser->getSourcePathList());
    return { tool, optionsParser };
}


TEST_CASE("Collector: return", "[Collector]") {
    const auto fileName = "return.hpp";
    const auto [tool, parser] = CreateTool(fileName);
    const auto allLines = CollectExecutableLines(*tool);
    REQUIRE(allLines.size() == 1);
    const auto& fileLines = allLines.begin()->second;
    REQUIRE(fileLines == std::vector<size_t>{ 2 });
}


TEST_CASE("Collector: if", "[Collector]") {
    const auto fileName = "if.hpp";
    const auto [tool, parser] = CreateTool(fileName);
    const auto allLines = CollectExecutableLines(*tool);
    REQUIRE(allLines.size() == 1);
    const auto& fileLines = allLines.begin()->second;
    REQUIRE(fileLines == std::vector<size_t>{ 2 });
}


TEST_CASE("Collector: template", "[Collector]") {
    const auto fileName = "template.hpp";
    const auto [tool, parser] = CreateTool(fileName);
    const auto allLines = CollectExecutableLines(*tool);
    REQUIRE(allLines.size() == 1);
    const auto& fileLines = allLines.begin()->second;
    // If delayed template parsing used, like in MSVC, we will only get a single line for the function's declaration.
    REQUIRE_THAT(fileLines,
                 Catch::Matchers::Equals(std::vector<size_t>{ 3 }) || Catch::Matchers::Equals(std::vector<size_t>{ 2 }));
}


TEST_CASE("Collector: namespace_scope", "[Collector]") {
    const auto fileName = "namespace_scope.hpp";
    const auto [tool, parser] = CreateTool(fileName);
    const auto allLines = CollectExecutableLines(*tool);
    REQUIRE(allLines.size() == 1);
    const auto& fileLines = allLines.begin()->second;
    REQUIRE(fileLines.empty());
}


TEST_CASE("Collector: class_scope", "[Collector]") {
    const auto fileName = "class_scope.hpp";
    const auto [tool, parser] = CreateTool(fileName);
    const auto allLines = CollectExecutableLines(*tool);
    REQUIRE(allLines.size() == 1);
    const auto& fileLines = allLines.begin()->second;
    REQUIRE(fileLines.empty());
}


TEST_CASE("Collector: method", "[Collector]") {
    const auto fileName = "method.hpp";
    const auto [tool, parser] = CreateTool(fileName);
    const auto allLines = CollectExecutableLines(*tool);
    REQUIRE(allLines.size() == 1);
    const auto& fileLines = allLines.begin()->second;
    REQUIRE(fileLines == std::vector<size_t>{ 3 });
}


TEST_CASE("Collector: template_method", "[Collector]") {
    const auto fileName = "template_method.hpp";
    const auto [tool, parser] = CreateTool(fileName);
    const auto allLines = CollectExecutableLines(*tool);
    REQUIRE(allLines.size() == 1);
    const auto& fileLines = allLines.begin()->second;
    REQUIRE_THAT(fileLines,
                 Catch::Matchers::Equals(std::vector<size_t>{ 3 }) || Catch::Matchers::Equals(std::vector<size_t>{ 4 }));
}


TEST_CASE("Collector: template_class", "[Collector]") {
    const auto fileName = "template_class.hpp";
    const auto [tool, parser] = CreateTool(fileName);
    const auto allLines = CollectExecutableLines(*tool);
    REQUIRE(allLines.size() == 1);
    const auto& fileLines = allLines.begin()->second;
    REQUIRE_THAT(fileLines,
                 Catch::Matchers::Equals(std::vector<size_t>{ 3 }) || Catch::Matchers::Equals(std::vector<size_t>{ 4 }));
}