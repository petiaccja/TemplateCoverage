#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


struct ReporterFormat {
    std::string_view name;
    std::string (*reporter)(const std::unordered_map<std::filesystem::path, std::vector<size_t>>&) = nullptr;
    std::string_view extension;
};


std::string Report(const std::unordered_map<std::filesystem::path, std::vector<size_t>>& executableLines, const ReporterFormat& format);
std::string ReportSonarQubeXML(const std::unordered_map<std::filesystem::path, std::vector<size_t>>& executableLines);
std::string ReportLCOV(const std::unordered_map<std::filesystem::path, std::vector<size_t>>& executableLines);


constexpr std::array reporterFormats = {
    ReporterFormat{"sonar-xml", &ReportSonarQubeXML, "xml" },
    ReporterFormat{ "lcov",     &ReportLCOV,         "info"},
};