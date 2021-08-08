#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "gaf_enum.h"

#if GAF_TEST_JSON
#include "readjsonsource.h"
#include "gaf_rapidjson_enum.h"
#endif

#if GAF_TEST_XML
#include "pugixmlsource.h"
#include "gaf_pugixml_enum.h"
#endif

TEST_CASE("enum Person")
{
    Person dude;

    CHECK(dude.happiness == Happiness::GLAD);
    CHECK(dude.favoriteProject == Project::Protobuf);
}

TEST_CASE("enum enum types")
{
    Happiness happiness = Happiness::HAPPY;
    Project project = Project::Gaf;

    Person dude;
    dude.happiness = happiness;
    dude.favoriteProject = project;

    CHECK(dude.happiness == happiness);
    CHECK(dude.favoriteProject == project);
}

// todo: need to add json loading tests
#if GAF_TEST_JSON

TEST_CASE("enum json_basic")
{
    Happiness happiness = Happiness::INDIFFERENT;
    Project project = Project::Other;

    Person person;
    const std::string load =
        ReadJsonSource(&person, " {\"happiness\": \"INDIFFERENT\", \"favoriteProject\": \"Other\"} ");
    REQUIRE(load == "");
    REQUIRE(person.happiness == happiness);
    REQUIRE(person.favoriteProject == project);
}

TEST_CASE("enum json_missing_project")
{
    Person person;
    const std::string load = ReadJsonSource(&person, " {\"happiness\": 12} ");
    REQUIRE(load != "");
}

TEST_CASE("enum json_empty_document")
{
    Person person;
    const std::string load = ReadJsonSource(&person, "{}");
    REQUIRE(load != "");
}

TEST_CASE("enum json_as_ints")
{
    Person person;
    const std::string load = ReadJsonSource(&person, " {\"happiness\": 1, \"favoriteProject\": 2} ");
    REQUIRE(load != "");
}

#endif

#if GAF_TEST_XML

TEST_CASE("enum xml_basic")
{
    Happiness happiness = Happiness::INDIFFERENT;
    Project project = Project::Other;

    Person person;
    const std::string load = ReadXmlSource(
        &person, "<Person happiness=\"INDIFFERENT\" favoriteProject=\"Other\" />", ReadXmlElementPerson);
    REQUIRE(load == "");
    REQUIRE(person.happiness == happiness);
    REQUIRE(person.favoriteProject == project);
}

TEST_CASE("enum xml_missing_project")
{
    Person person;
    const std::string load =
        ReadXmlSource(&person, " <Person happiness=\"12\" />", ReadXmlElementPerson);
    REQUIRE(load != "");
}

TEST_CASE("enum xml_empty_document")
{
    Person person;
    const std::string load = ReadXmlSource(&person, "<Person />", ReadXmlElementPerson);
    REQUIRE(load != "");
}

TEST_CASE("enum xml_as_ints")
{
    Person person;
    const std::string load =
        ReadXmlSource(&person, "<Person happiness=\"1\" favoriteProject=\"2\"/>", ReadXmlElementPerson);
    REQUIRE(load != "");
}

#endif
