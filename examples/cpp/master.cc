#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "gaf_master.h"
#include "readjsonsource.h"

#if GAF_TEST_JSON
#include "gaf_rapidjson_master.h"
#endif

#if GAF_TEST_XML
#include "pugixmlsource.h"
#include "gaf_pugixml_master.h"
#endif

using Catch::Matchers::Equals;
template <typename T>
struct Vector : public std::vector<T>
{
    Vector<T>& operator<<(const T& t)
    {
        this->push_back(t);
        return *this;
    }
};

TEST_CASE("custom vector equals works")
{
    std::vector<int> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);

    REQUIRE_THAT(a, Equals(Vector<int>() << 1 << 2 << 3));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// standard

TEST_CASE("master std constructor")
{
    Standard fb;
    REQUIRE(fb.a == 0);
    REQUIRE(fb.b == 0);
    REQUIRE(fb.c == 0);
    REQUIRE(fb.d == 0);
    REQUIRE(fb.ua == 0);
    REQUIRE(fb.ub == 0);
    REQUIRE(fb.uc == 0);
    REQUIRE(fb.ud == 0);
    REQUIRE(fb.f == false);
    REQUIRE(fb.g == 0);
    REQUIRE(fb.h == 0);
    REQUIRE(fb.i == "");
    REQUIRE(fb.j.value == 0);
    REQUIRE(fb.k == Enum::A);
}

#if GAF_TEST_JSON

TEST_CASE("master std json")
{
    Standard fb;
    const std::string load = ReadJsonSource(&fb,
                                            "{\
    \"a\": 1,\
    \"b\": 2,\
    \"c\": 3,\
    \"d\": 4,\
    \"ua\": 5,\
    \"ub\": 6,\
    \"uc\": 7,\
    \"ud\": 8,\
    \"e\": 9,\
    \"f\": true,\
    \"g\": 10.0,\
    \"h\": 11.0,\
    \"i\": \"dog\",\
    \"j\": {\"value\": 3},\
    \"k\": \"C\"\
    }");
    REQUIRE(load == "");

    REQUIRE(fb.a == 1);
    REQUIRE(fb.b == 2);
    REQUIRE(fb.c == 3);
    REQUIRE(fb.d == 4);
    REQUIRE(fb.ua == 5);
    REQUIRE(fb.ub == 6);
    REQUIRE(fb.uc == 7);
    REQUIRE(fb.ud == 8);
    REQUIRE(fb.f == true);
    REQUIRE(fb.g == 10.0f);
    REQUIRE(fb.h == 11.0);
    REQUIRE(fb.i == "dog");
    REQUIRE(fb.j.value == 3);
    REQUIRE(fb.k == Enum::C);
}

#endif

#if GAF_TEST_XML

TEST_CASE("master std xml")
{
    Standard fb;
    const std::string load = ReadXmlSource(&fb,
                                           "<Standard\
    a=\"1\"\
    b=\"2\"\
    c=\"3\"\
    d=\"4\"\
    ua=\"5\"\
    ub=\"6\"\
    uc=\"7\"\
    ud=\"8\"\
    f=\"true\"\
    g=\"10.0\"\
    h=\"11.0\"\
    i=\"dog\"\
    k=\"C\"\
    ><j value=\"3\" />\
     </Standard>",
                                           ReadXmlElementStandard);
    REQUIRE(load == "");

    REQUIRE(fb.a == 1);
    REQUIRE(fb.b == 2);
    REQUIRE(fb.c == 3);
    REQUIRE(fb.d == 4);
    REQUIRE(fb.ua == 5);
    REQUIRE(fb.ub == 6);
    REQUIRE(fb.uc == 7);
    REQUIRE(fb.ud == 8);
    REQUIRE(fb.f == true);
    REQUIRE(fb.g == 10.0f);
    REQUIRE(fb.h == 11.0);
    REQUIRE(fb.i == "dog");
    REQUIRE(fb.j.value == 3);
    REQUIRE(fb.k == Enum::C);
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// arrays

TEST_CASE("master array constructor")
{
    Arrays fb;
    REQUIRE(fb.a.size() == 0);
    REQUIRE(fb.b.size() == 0);
    REQUIRE(fb.c.size() == 0);
    REQUIRE(fb.d.size() == 0);
    REQUIRE(fb.ua.size() == 0);
    REQUIRE(fb.ub.size() == 0);
    REQUIRE(fb.uc.size() == 0);
    REQUIRE(fb.ud.size() == 0);
    REQUIRE(fb.f.size() == 0);
    REQUIRE(fb.g.size() == 0);
    REQUIRE(fb.h.size() == 0);
    REQUIRE(fb.i.size() == 0);
    REQUIRE(fb.j.size() == 0);
    REQUIRE(fb.k.size() == 0);
}

#if GAF_TEST_JSON

TEST_CASE("master array empty json")
{
    Arrays advanced;

    const std::string load = ReadJsonSource(&advanced, "{}");
    REQUIRE(load != "");
}

TEST_CASE("master array basic json")
{
    Arrays fb;
    const std::string load = ReadJsonSource(&fb,
                                            "{\
    \"a\": [],\
    \"b\": [],\
    \"c\": [],\
    \"d\": [],\
    \"ua\": [],\
    \"ub\": [],\
    \"uc\": [],\
    \"ud\": [],\
    \"f\": [],\
    \"g\": [],\
    \"h\": [],\
    \"i\": [],\
    \"j\": [],\
    \"k\": []}");
    REQUIRE(load == "");

    REQUIRE(fb.a.size() == 0);
    REQUIRE(fb.b.size() == 0);
    REQUIRE(fb.c.size() == 0);
    REQUIRE(fb.d.size() == 0);
    REQUIRE(fb.ua.size() == 0);
    REQUIRE(fb.ub.size() == 0);
    REQUIRE(fb.uc.size() == 0);
    REQUIRE(fb.ud.size() == 0);
    REQUIRE(fb.f.size() == 0);
    REQUIRE(fb.g.size() == 0);
    REQUIRE(fb.h.size() == 0);
    REQUIRE(fb.i.size() == 0);
    REQUIRE(fb.j.size() == 0);
    REQUIRE(fb.k.size() == 0);
}

// todo: parse standard too
TEST_CASE("master arrays advanced json")
{
    Arrays fb;
    const std::string load = ReadJsonSource(&fb,
                                            "{\
    \"a\": [1, 2],\
    \"b\": [3, 4],\
    \"c\": [5, 6],\
    \"d\": [7, 8],\
    \"ua\": [9, 10],\
    \"ub\": [11, 12],\
    \"uc\": [13, 14],\
    \"ud\": [15, 16],\
    \"f\": [false, true],\
    \"g\": [19, 20],\
    \"h\": [21, 22],\
    \"i\": [\"cat\", \"dog\"],\
    \"j\": [],\
    \"k\": [\"A\", \"C\"]}");
    REQUIRE(load == "");

    REQUIRE_THAT(fb.a, Equals(Vector<std::int8_t>() << 1 << 2));
    REQUIRE_THAT(fb.b, Equals(Vector<std::int16_t>() << 3 << 4));
    REQUIRE_THAT(fb.c, Equals(Vector<std::int32_t>() << 5 << 6));
    REQUIRE_THAT(fb.d, Equals(Vector<std::int64_t>() << 7 << 8));

    REQUIRE_THAT(fb.ua, Equals(Vector<std::uint8_t>() << 9 << 10));
    REQUIRE_THAT(fb.ub, Equals(Vector<std::uint16_t>() << 11 << 12));
    REQUIRE_THAT(fb.uc, Equals(Vector<std::uint32_t>() << 13 << 14));
    REQUIRE_THAT(fb.ud, Equals(Vector<std::uint64_t>() << 15 << 16));

    REQUIRE_THAT(fb.f, Equals(Vector<bool>() << false << true));
    REQUIRE_THAT(fb.g, Equals(Vector<float>() << 19 << 20));
    REQUIRE_THAT(fb.h, Equals(Vector<double>() << 21 << 22));

    REQUIRE_THAT(fb.i, Equals(Vector<std::string>() << "cat"
                                                    << "dog"));

    // todo(Gustav): add json loading check
    // REQUIRE_THAT(fb.j, Equals(std::vector<Struct>{}));
    REQUIRE(fb.j.size() == 0);

    REQUIRE_THAT(fb.k, Equals(Vector<Enum>() << Enum::A << Enum::C));
}

#endif

#if GAF_TEST_XML

TEST_CASE("master array empty xml")
{
    Arrays advanced;

    const std::string load = ReadXmlSource(&advanced, "{}", ReadXmlElementArrays);
    REQUIRE(load != "");
}

TEST_CASE("master array basic xml")
{
    Arrays fb;
    const std::string load = ReadXmlSource(&fb, "<Arrays/>", ReadXmlElementArrays);
    REQUIRE(load == "");

    REQUIRE(fb.a.size() == 0);
    REQUIRE(fb.b.size() == 0);
    REQUIRE(fb.c.size() == 0);
    REQUIRE(fb.d.size() == 0);
    REQUIRE(fb.ua.size() == 0);
    REQUIRE(fb.ub.size() == 0);
    REQUIRE(fb.uc.size() == 0);
    REQUIRE(fb.ud.size() == 0);
    REQUIRE(fb.f.size() == 0);
    REQUIRE(fb.g.size() == 0);
    REQUIRE(fb.h.size() == 0);
    REQUIRE(fb.i.size() == 0);
    REQUIRE(fb.j.size() == 0);
    REQUIRE(fb.k.size() == 0);
}

// todo: parse standard too
TEST_CASE("master arrays advanced xml")
{
    Arrays fb;
    const std::string load = ReadXmlSource(&fb,
                                           "<Arrays>\
    <a>1</a><a>2</a>\
    <b>3</b><b>4</b>\
    <c>5</c><c>6</c>\
    <d>7</d><d>8</d>\
    <ua>9</ua><ua>10</ua>\
    <ub>11</ub><ub>12</ub>\
    <uc>13</uc><uc>14</uc>\
    <ud>15</ud><ud>16</ud>\
    <f>false</f><f>true</f>\
    <g>19</g><g>20</g>\
    <h>21</h><h>22</h>\
    <i>cat</i><i>dog</i>\
    <k>A</k><k>C</k>\
    </Arrays>",
                                           ReadXmlElementArrays);

    REQUIRE(load == "");

    REQUIRE_THAT(fb.a, Equals(Vector<std::int8_t>() << 1 << 2));
    REQUIRE_THAT(fb.b, Equals(Vector<std::int16_t>() << 3 << 4));
    REQUIRE_THAT(fb.c, Equals(Vector<std::int32_t>() << 5 << 6));
    REQUIRE_THAT(fb.d, Equals(Vector<std::int64_t>() << 7 << 8));

    REQUIRE_THAT(fb.ua, Equals(Vector<std::uint8_t>() << 9 << 10));
    REQUIRE_THAT(fb.ub, Equals(Vector<std::uint16_t>() << 11 << 12));
    REQUIRE_THAT(fb.uc, Equals(Vector<std::uint32_t>() << 13 << 14));
    REQUIRE_THAT(fb.ud, Equals(Vector<std::uint64_t>() << 15 << 16));

    REQUIRE_THAT(fb.f, Equals(Vector<bool>() << false << true));
    REQUIRE_THAT(fb.g, Equals(Vector<float>() << 19 << 20));
    REQUIRE_THAT(fb.h, Equals(Vector<double>() << 21 << 22));

    REQUIRE_THAT(fb.i, Equals(Vector<std::string>() << "cat"
                                                    << "dog"));

    // todo(Gustav): add json loading check
    // REQUIRE_THAT(fb.j, Equals(std::vector<Struct>{}));
    REQUIRE(fb.j.size() == 0);

    REQUIRE_THAT(fb.k, Equals(Vector<Enum>() << Enum::A << Enum::C));
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// optional

TEST_CASE("master pointer constructor")
{
    Foo foo;
    REQUIRE(foo.value == nullptr);

    FooRoot bar;
    REQUIRE(bar.name == nullptr);
    REQUIRE(bar.foo == nullptr);
}

TEST_CASE("master optional constructor")
{
    Bar foo;
    REQUIRE(foo.value == 0);

    BarRoot bar;
    REQUIRE(bar.name == "");
    REQUIRE(bar.foo.value == 0);
}

#if GAF_TEST_JSON

TEST_CASE("master pointer json_basic")
{
    Foo foo;
    const std::string load = ReadJsonSource(&foo, " {\"value\": 12} ");
    REQUIRE(load == "");
    REQUIRE(foo.value != nullptr);
    REQUIRE(*foo.value == 12);
}

TEST_CASE("master optional json_basic")
{
    Bar foo;
    const std::string load = ReadJsonSource(&foo, " {\"value\": 12} ");
    REQUIRE(load == "");
    REQUIRE(foo.value == 12);
}

TEST_CASE("master pointer json_missing_foo")
{
    FooRoot bar;
    const std::string load = ReadJsonSource(&bar, " {\"name\": \"good dog\"} ");
    REQUIRE(load == "");
    REQUIRE(bar.name != nullptr);
    REQUIRE(*bar.name == "good dog");
    REQUIRE(bar.foo == nullptr);
}

TEST_CASE("master optional json_missing_foo")
{
    BarRoot bar;
    const std::string load = ReadJsonSource(&bar, " {\"name\": \"good dog\"} ");
    REQUIRE(load == "");
    REQUIRE(bar.name == "good dog");
    REQUIRE(bar.foo.value == 0);
}

TEST_CASE("master pointer json_invalid_value_for_name")
{
    // optional means optional, not accept if invalid
    FooRoot bar;
    const std::string load = ReadJsonSource(&bar, " {\"name\": 3} ");
    REQUIRE(load != "");
}

TEST_CASE("master optional json_invalid_value_for_name")
{
    // optional means optional, not accept if invalid
    BarRoot bar;
    const std::string load = ReadJsonSource(&bar, " {\"name\": 3} ");
    REQUIRE(load != "");
}

TEST_CASE("master pointer json_empty_document")
{
    FooRoot bar;
    const std::string load = ReadJsonSource(&bar, "{}");
    REQUIRE(load == "");
    REQUIRE(bar.name == nullptr);
    REQUIRE(bar.foo == nullptr);
}

TEST_CASE("master optional json_empty_document")
{
    BarRoot bar;
    const std::string load = ReadJsonSource(&bar, "{}");
    REQUIRE(load == "");
    REQUIRE(bar.name == "");
    REQUIRE(bar.foo.value == 0);
}

TEST_CASE("master pointer json_optional_struct")
{
    FooRoot bar;
    const std::string load = ReadJsonSource(&bar, "{\"foo\": {\"value\": 5}}");
    REQUIRE(load == "");
    CHECK(bar.name == nullptr);
    CHECK(bar.foo != nullptr);
    CHECK(bar.foo->value != nullptr);
    CHECK(*bar.foo->value == 5);
}

TEST_CASE("master optional json_optional_struct")
{
    BarRoot bar;
    const std::string load = ReadJsonSource(&bar, "{\"foo\": {\"value\": 5}}");
    REQUIRE(load == "");
    CHECK(bar.name == "");
    CHECK(bar.foo.value == 5);
}

#endif

#if GAF_TEST_XML

TEST_CASE("master pointer xml_basic")
{
    Foo foo;
    const std::string load = ReadXmlSource(&foo, "<Foo value=\"12\" />", ReadXmlElementFoo);
    REQUIRE(load == "");
    REQUIRE(foo.value != nullptr);
    REQUIRE(*foo.value == 12);
}

TEST_CASE("master optional xml_basic")
{
    Bar foo;
    const std::string load = ReadXmlSource(&foo, "<Foo value=\"12\" />", ReadXmlElementBar);
    REQUIRE(load == "");
    REQUIRE(foo.value == 12);
}

TEST_CASE("master pointer xml_missing_foo")
{
    FooRoot bar;
    const std::string load = ReadXmlSource(&bar, "<FooRoot name=\"good dog\" />", ReadXmlElementFooRoot);
    REQUIRE(load == "");
    REQUIRE(bar.name != nullptr);
    REQUIRE(*bar.name == "good dog");
    REQUIRE(bar.foo == nullptr);
}

TEST_CASE("master optional xml_missing_foo")
{
    BarRoot bar;
    const std::string load = ReadXmlSource(&bar, "<BarRoot name=\"good dog\" />", ReadXmlElementBarRoot);
    REQUIRE(load == "");
    REQUIRE(bar.name == "good dog");
    REQUIRE(bar.foo.value == 0);
}

TEST_CASE("master pointer xml_empty_document")
{
    FooRoot bar;
    const std::string load = ReadXmlSource(&bar, "<FooRoot />", ReadXmlElementFooRoot);
    REQUIRE(load == "");
    REQUIRE(bar.name == nullptr);
    REQUIRE(bar.foo == nullptr);
}

TEST_CASE("master pointer xml_empty_document with invalid attribute")
{
    FooRoot bar;
    const std::string load = ReadXmlSource(&bar, "<FooRoot dog=\"dog\"/>", ReadXmlElementFooRoot);
    REQUIRE(load ==
            "Found unused attributes for type FooRoot at /FooRoot:\n"
            "Invalid attribute dog at /FooRoot@dog and it could be name");
}

TEST_CASE("master pointer xml_empty_document with invalid child")
{
    FooRoot bar;
    const std::string load = ReadXmlSource(&bar, "<FooRoot><cat /> </FooRoot>", ReadXmlElementFooRoot);
    REQUIRE(load ==
            "Found unused elements for type FooRoot at /FooRoot:\n"
            "Invalid child cat at /FooRoot/cat and it could be foo");
}

TEST_CASE("master optional xml_empty_document")
{
    BarRoot bar;
    const std::string load = ReadXmlSource(&bar, "<BarRoot />", ReadXmlElementBarRoot);
    REQUIRE(load == "");
    REQUIRE(bar.name == "");
    REQUIRE(bar.foo.value == 0);
}

TEST_CASE("master pointer xml_optional_struct")
{
    FooRoot bar;
    const std::string load =
        ReadXmlSource(&bar, "<FooRoot><foo value=\"5\"/></FooRoot>", ReadXmlElementFooRoot);
    REQUIRE(load == "");
    CHECK(bar.name == nullptr);
    CHECK(bar.foo != nullptr);
    CHECK(bar.foo->value != nullptr);
    CHECK(*bar.foo->value == 5);
}

TEST_CASE("master optional xml_optional_struct")
{
    BarRoot bar;
    const std::string load =
        ReadXmlSource(&bar, "<BarRoot><foo value=\"5\" /></BarRoot>", ReadXmlElementBarRoot);
    REQUIRE(load == "");
    CHECK(bar.name == "");
    CHECK(bar.foo.value == 5);
}

#endif
