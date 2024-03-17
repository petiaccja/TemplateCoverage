#include "Collector.hpp"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Basic/DirectoryEntry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <unordered_set>


using namespace clang::tooling;
using namespace llvm;
using namespace clang;
namespace matchers = clang::ast_matchers;


matchers::StatementMatcher executableCodeMatcher = matchers::stmt(matchers::anyOf(
                                                                      matchers::returnStmt(matchers::isExpansionInMainFile()),
                                                                      matchers::continueStmt(matchers::isExpansionInMainFile()),
                                                                      matchers::breakStmt(matchers::isExpansionInMainFile()),
                                                                      matchers::coreturnStmt(matchers::isExpansionInMainFile()),
                                                                      matchers::coyieldExpr(matchers::isExpansionInMainFile()),
                                                                      matchers::gotoStmt(matchers::isExpansionInMainFile()),
                                                                      matchers::expr(matchers::isExpansionInMainFile())))
                                                       .bind("executableCode");


class ExecutableLineCollector : public matchers::MatchFinder::MatchCallback {
public:
    void run(const matchers::MatchFinder::MatchResult& result) override {
        auto& sourceManager = *result.SourceManager;

        if (const Stmt* node = result.Nodes.getNodeAs<Stmt>("executableCode")) {
            bool beginInvalid = true;
            const auto beginLine = sourceManager.getExpansionLineNumber(node->getBeginLoc(), &beginInvalid);
            bool endInvalid = true;
            const auto endLine = sourceManager.getExpansionLineNumber(node->getEndLoc(), &endInvalid);

            if (!beginInvalid) {
                const auto validEndLine = !endInvalid ? endLine : beginLine;
                for (size_t line = beginLine; line <= validEndLine; ++line) {
                    m_executableLines.insert(line);
                }
            }
        }
    }

    std::vector<size_t> GetExecutableLines() {
        std::vector<size_t> executableLines(m_executableLines.begin(), m_executableLines.end());
        std::ranges::sort(executableLines);
        return executableLines;
    }

    void Clear() {
        m_executableLines.clear();
    }

private:
    std::unordered_set<size_t> m_executableLines;
};


class FileExecutableLineCollector : public SourceFileCallbacks {
public:
    FileExecutableLineCollector(std::shared_ptr<ExecutableLineCollector> collector) : m_collector(collector) {}

    bool handleBeginSource(CompilerInstance& compilerInstance) override {
        auto& sourceManager = compilerInstance.getSourceManager();
        const auto mainFileId = sourceManager.getMainFileID();
        const auto mainFileEntry = sourceManager.getFileEntryForID(mainFileId);
        const auto path = std::filesystem::path(mainFileEntry->getName().str());
        m_currentFile = path;
        return true;
    }

    void handleEndSource() override {
        auto executableLines = m_collector->GetExecutableLines();
        m_executableLines[m_currentFile] = std::move(executableLines);
        m_collector->Clear();
    }

    const std::unordered_map<std::filesystem::path, std::vector<size_t>>& GetExecutableLines() const {
        return m_executableLines;
    }

private:
    std::filesystem::path m_currentFile;
    std::shared_ptr<ExecutableLineCollector> m_collector;
    std::unordered_map<std::filesystem::path, std::vector<size_t>> m_executableLines;
};


std::unordered_map<std::filesystem::path, std::vector<size_t>> CollectExecutableLines(ClangTool& tool) {
    const auto collector = std::make_shared<ExecutableLineCollector>();
    FileExecutableLineCollector fileCollector{ collector };
    matchers::MatchFinder finder;
    finder.addMatcher(executableCodeMatcher, collector.get());

    const auto result = tool.run(newFrontendActionFactory(&finder, &fileCollector).get());
    if (result != 0) {
        throw std::logic_error("failed");
    }

    return fileCollector.GetExecutableLines();
}