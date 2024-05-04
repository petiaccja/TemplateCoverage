//------------------------------------------------------------------------------
// The MIT License (MIT)
// Copyright (c) 2024 Péter Kardos
//------------------------------------------------------------------------------

#include <Collector.hpp>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include <fstream>

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


std::pair<std::vector<size_t>, std::vector<size_t>> GetExpectedLines(std::string_view file) {
    const auto path = std::filesystem::path(TEST_INPUT_DIR) / file;
    std::ifstream s(path);
    std::vector<size_t> executableLines;
    std::vector<size_t> lateTemplateLines;
    size_t currentLine = 1;
    while (s.good()) {
        std::string line;
        std::getline(s, line);
        const auto commentStart = line.find_last_of("//");
        if (commentStart != line.npos) {
            const auto comment = std::string_view(line.begin() + commentStart, line.end());
            if (comment.find('X') != comment.npos) {
                executableLines.push_back(currentLine);
            }
            if (comment.find('T') != comment.npos) {
                lateTemplateLines.push_back(currentLine);
            }
        }
        ++currentLine;
    }
    return { std::move(executableLines), std::move(lateTemplateLines) };
}


TEST_CASE("Collector: line matches", "[Collector]") {
    std::filesystem::directory_iterator inputDirIt(std::filesystem::path(TEST_INPUT_DIR));
    size_t numSections = 0;
    for (const auto inputFile : inputDirIt) {
        if (!inputFile.is_regular_file()) {
            continue;
        }
        const auto filePath = inputFile.path();
        if (filePath.extension() != ".cpp" && filePath.extension() != ".hpp") {
            continue;
        }
        const auto fileName = inputFile.path().filename().string();
        SECTION(fileName) {
            const auto [executableLines, lateTemplateLines] = GetExpectedLines(fileName);
            const auto [tool, parser] = CreateTool(fileName);
            const auto allLines = CollectExecutableLines(*tool);
            REQUIRE(allLines.size() == 1);
            const auto& fileLines = allLines.begin()->second;
            REQUIRE_THAT(fileLines, Catch::Matchers::Equals(executableLines) || Catch::Matchers::Equals(lateTemplateLines));
            ++numSections;
        }
    }
    REQUIRE(numSections != 0); // To warn if the above iteration is buggy and it's testing zero files.
}
