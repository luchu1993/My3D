//
// Created by luchu on 2022/1/25.
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "pugixml.hpp"
#include <iostream>


const char* xml_content = R"(
<?xml version="1.0" ?>
<Profile>
    <Tools>
        <Tool Filename="a.txt" Timeout="1" />
        <Tool Filename="b.txt" Timeout="2" />
        <Tool Filename="c.txt" Timeout="3" />
    </Tools>
</Profile>
)";

TEST_CASE("pugixml dom testing", "[engine]")
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xml_content);
    REQUIRE(result.status == pugi::status_ok);

    for (pugi::xml_node tool : doc.child("Profile").child("Tools").children("Tool"))
    {
        int timeout = tool.attribute("Timeout").as_int();
        if (timeout > 0)
        {
            std::cout << "Tool " << tool.attribute("Filename").value() << " has timeout " << timeout << "\n";
        }
    }
}

TEST_CASE("pugixml xpath testing", "[engine]")
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xml_content);
    REQUIRE(result.status == pugi::status_ok);

    pugi::xpath_node_set tools = doc.select_nodes("/Profile/Tools/Tool[@Timeout > 0]");

    for (pugi::xpath_node node : tools)
    {
        pugi::xml_node tool = node.node();
        std::cout << "Tool " << tool.attribute("Filename").value() << " has timeout "
                  << tool.attribute("Timeout").as_int() << "\n";
    }
}