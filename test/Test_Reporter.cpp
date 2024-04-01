//------------------------------------------------------------------------------
// The MIT License (MIT)
// Copyright (c) 2024 Péter Kardos
//------------------------------------------------------------------------------

#include <Reporter.hpp>
#include <pugixml.hpp>

#include <ranges>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;


const std::unordered_map<std::filesystem::path, std::vector<size_t>> input = {
    {"/my/source.cpp", { 1 }},
};


TEST_CASE("Reporter: SonarQube XML", "[Reporter]") {
    const auto output = ReportSonarQubeXML(input);

    INFO(output);
    pugi::xml_document doc;
    doc.load_string(output.data());
    REQUIRE(doc.child("coverage"));
    REQUIRE(doc.child("coverage").child("file"));
    REQUIRE(doc.child("coverage").child("file").attribute("path"));
    REQUIRE(std::filesystem::path(doc.child("coverage").child("file").attribute("path").as_string()) == std::filesystem::path("/my/source.cpp"));
    REQUIRE(doc.child("coverage").child("file").child("lineToCover"));
    REQUIRE(doc.child("coverage").child("file").child("lineToCover").attribute("lineNumber"));
    REQUIRE(doc.child("coverage").child("file").child("lineToCover").attribute("lineNumber").as_int() == 1);
    REQUIRE(doc.child("coverage").child("file").child("lineToCover").attribute("covered"));
    REQUIRE(doc.child("coverage").child("file").child("lineToCover").attribute("covered").as_bool() == false);
}


TEST_CASE("Reporter: LCOV", "[Reporter]") {
    const auto output = ReportLCOV(input);

    INFO(output);

    constexpr std::string_view expected =
        "TN:\n"
        "SF:/my/source.cpp\n"
        "DA:1,0\n"
        "LH:0\n"
        "LF:1\n"
        "end_of_record\n";

    REQUIRE(output == expected);
}
