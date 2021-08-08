#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "gaf_twostructs.h"
#include "readjsonsource.h"

#if GAF_TEST_JSON
#include "gaf_rapidjson_twostructs.h"
#endif

#if GAF_TEST_XML
#include "pugixmlsource.h"
#include "gaf_pugixml_twostructs.h"
#endif

TEST_CASE("twostructs constructor")
{
    Foo foo;
    REQUIRE(foo.hello == 0);
    REQUIRE(foo.world == 0.0f);

    Bar bar;
    REQUIRE(bar.foo.hello == 0);
    REQUIRE(bar.foo.world == 0.0f);
    REQUIRE(bar.bar == "");
    REQUIRE(bar.b == false);
}

TEST_CASE("twostructs setter")
{
    Foo foo;
    foo.hello = 42;
    foo.world = 4.2f;
    Bar bar;
    bar.foo = foo;
    bar.bar = "dog";
    bar.b = true;
    foo.hello = 5;
    foo.world = 5.5f;

    REQUIRE(bar.foo.hello == 42);
    REQUIRE(bar.foo.world == 4.2f);
    REQUIRE(bar.bar == "dog");
    REQUIRE(bar.b == true);

    bar.foo.hello = 13;
    bar.foo.world = 3.7f;
    REQUIRE(bar.foo.hello == 13);
    REQUIRE(bar.foo.world == 3.7f);
}

#if GAF_TEST_JSON

TEST_CASE("twostructs json_basic")
{
    Foo foo;
    const std::string load = ReadJsonSource(&foo, " {\"hello\": 12, \"world\": 2.4} ");
    REQUIRE(load == "");
    REQUIRE(foo.hello == 12);
    REQUIRE(foo.world == 2.4f);
}

TEST_CASE("twostructs json_double_can_be_ints")
{
    Foo foo;
    const std::string load = ReadJsonSource(&foo, " {\"hello\": 12, \"world\": 2} ");
    REQUIRE(load == "");
    REQUIRE(foo.hello == 12);
    REQUIRE(foo.world == 2.0f);
}

TEST_CASE("twostructs json_missing_world")
{
    Foo foo;
    const std::string load = ReadJsonSource(&foo, " {\"hello\": 12} ");
    REQUIRE(load != "");
}

TEST_CASE("twostructs json_empty_document")
{
    Foo foo;
    const std::string load = ReadJsonSource(&foo, "{}");
    REQUIRE(load != "");
}

TEST_CASE("twostructs json_advanced")
{
    Bar bar;
    const std::string load = ReadJsonSource(
        &bar, "{\"bar\": \"cat and dog\", \"b\": true, \"foo\": {\"hello\": 12, \"world\": 2.4}}");
    REQUIRE(load == "");
    CHECK(bar.foo.hello == 12);
    CHECK(bar.foo.world == 2.4f);
    CHECK(bar.bar == "cat and dog");
    CHECK(bar.b == true);
}

#endif

#if GAF_TEST_XML

TEST_CASE("twostructs xml_basic")
{
    Foo foo;
    const std::string load =
        ReadXmlSource(&foo, "<Foo hello=\"12\" world=\"2.4\" />", ReadXmlElementFoo);
    REQUIRE(load == "");
    REQUIRE(foo.hello == 12);
    REQUIRE(foo.world == 2.4f);
}

TEST_CASE("twostructs xml_double_can_be_ints")
{
    Foo foo;
    const std::string load = ReadXmlSource(&foo, "<Foo hello=\"12\" world=\"2\" />", ReadXmlElementFoo);
    REQUIRE(load == "");
    REQUIRE(foo.hello == 12);
    REQUIRE(foo.world == 2.0f);
}

TEST_CASE("twostructs xml_missing_world")
{
    Foo foo;
    const std::string load = ReadXmlSource(&foo, "<Foo hello=\"12\" />", ReadXmlElementFoo);
    REQUIRE(load != "");
}

TEST_CASE("twostructs xml_empty_document")
{
    Foo foo;
    const std::string load = ReadXmlSource(&foo, "<Foo />", ReadXmlElementFoo);
    REQUIRE(load != "");
}

TEST_CASE("twostructs xml_advanced")
{
    Bar bar;
    const std::string load = ReadXmlSource(
        &bar, "<Bar bar=\"cat and dog\" b=\"true\"><foo hello=\"12\" world=\"2.4\" /> </Bar>",
        ReadXmlElementBar);
    REQUIRE(load == "");
    CHECK(bar.foo.hello == 12);
    CHECK(bar.foo.world == 2.4f);
    CHECK(bar.bar == "cat and dog");
    CHECK(bar.b == true);
}

#endif
