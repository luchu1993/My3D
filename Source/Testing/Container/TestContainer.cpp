//
// Created by luchu on 2022/1/22.
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Container/List.h"
#include "Container/String.h"
using namespace My3D;


TEST_CASE("List Add Remove", "[engine]")
{
    My3D::List<int> list;
    REQUIRE(list.Size() == 0);
    REQUIRE(list.Empty());

    for (int i = 1; i <= 5; ++i)
    {
        list.Push(i);
    }
    REQUIRE(list.Size() == 5);
    REQUIRE(list.Front() == 1);
    REQUIRE(list.Back() == 5);

    list.Pop();
    REQUIRE(list.Size() == 4);
}

TEST_CASE("List Find", "[engine]")
{
    My3D::List<String> list;
    for (int i = 1; i <= 5; ++i)
    {
        list.Push(String(i));
    }
    REQUIRE(list.Contains("5"));
    REQUIRE(list.Find("6") == list.End());
}


