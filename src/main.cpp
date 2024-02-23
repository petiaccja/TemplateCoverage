#include "Collector.hpp"
#include "Reporter.hpp"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include <format>
#include <fstream>
#include <iostream>


using namespace clang::tooling;
using namespace clang;


enum FormatEnum : size_t {
    FIRST = 0,
};

template <size_t N, size_t... Ns>
auto clFormats(const std::array<ReporterFormat, N>& formats, std::index_sequence<Ns...>) {
    return llvm::cl::values((clEnumValN(static_cast<FormatEnum>(Ns), formats[Ns].name, ""))...);
}


template <size_t N>
auto clFormats(const std::array<ReporterFormat, N>& formats) {
    return clFormats(formats, std::make_index_sequence<N>{});
}

static llvm::cl::OptionCategory templateCoverageCategory("template-coverage options");

static llvm::cl::opt<std::string> outputOpt("out-file",
                                            llvm::cl::init("./coverage"),
                                            llvm::cl::desc("the file into which the coverage report is written"),
                                            llvm::cl::cat(templateCoverageCategory));

static llvm::cl::opt<FormatEnum> formatOpt("format",
                                           llvm::cl::init(FormatEnum::FIRST),
                                           llvm::cl::desc("coverage report file format"),
                                           llvm::cl::cat(templateCoverageCategory),
                                           clFormats(reporterFormats));

static llvm::cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);


int main(int argc, const char** argv) {
    try {
        auto maybeParser = CommonOptionsParser::create(argc, argv, templateCoverageCategory);
        if (!maybeParser) {
            llvm::errs() << maybeParser.takeError();
            return 1;
        }
        CommonOptionsParser& optionsParser = maybeParser.get();
        ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

        const auto& executableLines = CollectExecutableLines(tool);
        const auto format = reporterFormats[static_cast<size_t>(formatOpt.getValue())];
        auto outputPath = std::filesystem::path(outputOpt.getValue());
        if (!outputPath.has_extension()) {
            outputPath.replace_extension(format.extension);
        }
        std::ofstream file(outputPath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            throw std::invalid_argument(std::format("couldn't open output file \"{}\"", outputOpt.getValue()));
        }
        file << Report(executableLines, format);

        return 0;
    }
    catch (std::exception& ex) {
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}