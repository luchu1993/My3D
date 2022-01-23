//
// Created by luchu on 2022/1/22.
//

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Container/List.h"
#include "Container/Vector.h"
#include "Container/HashMap.h"
#include "Container/HashSet.h"
#include "Container/String.h"

using namespace My3D;

TEST_CASE("pod vector testing", "[engine]")
{
    Vector<float> vec;
    REQUIRE(vec.Size() == 0);
    REQUIRE_FALSE(vec.Contains(1));

    vec.Push(100);
    vec.Push(200);

    REQUIRE_FALSE(vec.Empty());
    REQUIRE(vec.Size() == 2);

    vec.Clear();
    REQUIRE(vec.Empty());

    for (int i = 1; i <= 1000; ++i)
    {
        vec.Push(i * 1.0f);
    }

    auto it = vec.Begin();
    while (it != vec.End())
    {
        it = vec.Erase(it);
    }

    REQUIRE(vec.Empty());
}

TEST_CASE("vector testing", "[engine]")
{
    Vector<String> vec;
    vec.Insert(0, "hello world");
    vec.Insert(0, "first");

    REQUIRE(vec.Size() == 2);
    REQUIRE_FALSE(vec.Empty());
    REQUIRE(vec[0] == "first");

    vec.Clear();
    REQUIRE(vec.Empty());
}

TEST_CASE("hashmap testing", "[engine]")
{
    HashMap<String, String> hashmap;
    hashmap.Populate("one", "ONE");
    hashmap["two"] = "Two";

    REQUIRE(hashmap.Contains("one"));
    REQUIRE(hashmap.Contains("two"));

    hashmap["one"] = "One";
    REQUIRE(hashmap.Size() == 2);
}

TEST_CASE("hashset testing", "[engine]")
{
    HashSet<String> hashset;
    REQUIRE(hashset.Empty());
}

TEST_CASE("list testing", "[engine]")
{
    List<int> list;
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

    REQUIRE_FALSE(list.Contains(5));
    REQUIRE(list.Find(5) == list.End());
}

TEST_CASE("string testing", "[engine]")
{
    String str ("hello,world");
    REQUIRE(str.Length() == 11);
    REQUIRE(str.LengthUTF8() == 11);

    String wstr(L"你好");
    REQUIRE(wstr.LengthUTF8() == 2);

    WString ws(wstr);
    REQUIRE(ws.Length() == 2);
}
