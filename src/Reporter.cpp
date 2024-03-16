#include "Reporter.hpp"

#include <pugixml.hpp>

#include <format>
#include <sstream>


std::string Report(const std::unordered_map<std::filesystem::path, std::vector<size_t>>& executableLines, const ReporterFormat& format) {
    if (format.reporter) {
        return format.reporter(executableLines);
    }
    throw std::invalid_argument(std::format("unsupported format: {}", format.name));
}


std::string ReportSonarQubeXML(const std::unordered_map<std::filesystem::path, std::vector<size_t>>& executableLines) {
    pugi::xml_document doc;
    auto rootNode = doc.append_child("coverage");
    rootNode.append_attribute("version").set_value(1);

    for (const auto& [filePath, linesToCover] : executableLines) {
        auto fileNode = rootNode.append_child("file");
        const auto filePathStr = filePath.string();
        fileNode.append_attribute("path").set_value(filePathStr.c_str(), filePathStr.size());
        for (const auto line : linesToCover) {
            auto lineNode = fileNode.append_child("lineToCover");
            lineNode.append_attribute("lineNumber").set_value(line);
            lineNode.append_attribute("covered").set_value(false);
        }
    }

    std::stringstream ss;
    doc.save(ss);
    return ss.str();
}