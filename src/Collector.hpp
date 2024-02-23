#include <clang/Tooling/Tooling.h>

#include <filesystem>
#include <unordered_set>


std::unordered_map<std::filesystem::path, std::vector<size_t>> CollectExecutableLines(clang::tooling::ClangTool& tool);