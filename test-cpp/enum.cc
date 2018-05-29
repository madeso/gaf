#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"
#include "readjsonsource.h"

TEST_CASE("Person") {
  Person dude;

#ifdef GAF_ENUM_STYLE_PrefixEnum
CHECK(dude.happiness == Happiness_SAD);
  CHECK(dude.favoriteProject == Project_Protobuf);
#else
  CHECK(dude.happiness == Happiness::SAD);
  CHECK(dude.favoriteProject == Project::Protobuf);
#endif
}

TEST_CASE("enum types") {
#ifndef GAF_ENUM_STYLE_NamespaceEnum
  Happiness
#else
  Happiness::Type
#endif
  happiness =
#ifdef GAF_ENUM_STYLE_PrefixEnum
  Happiness_HAPPY
#else
  Happiness::HAPPY
#endif
  ;

#ifndef GAF_ENUM_STYLE_NamespaceEnum
  Project
#else
  Project::Type
#endif
  project =
#ifdef GAF_ENUM_STYLE_PrefixEnum
  Project_Gaf
#else
  Project::Gaf
#endif
  ;

  Person dude;
  dude.happiness = happiness;
  dude.favoriteProject = project;

  CHECK(dude.happiness == happiness);
  CHECK(dude.favoriteProject == project);
}

// todo: need to add json loading tests

#if GAF_TEST_JSON

TEST_CASE("json_basic") {
#ifndef GAF_ENUM_STYLE_NamespaceEnum
  Happiness
#else
  Happiness::Type
#endif
  happiness =
#ifdef GAF_ENUM_STYLE_PrefixEnum
  Happiness_INDIFFERENT
#else
  Happiness::INDIFFERENT
#endif
  ;

#ifndef GAF_ENUM_STYLE_NamespaceEnum
  Project
#else
  Project::Type
#endif
  project =
#ifdef GAF_ENUM_STYLE_PrefixEnum
  Project_Other
#else
  Project::Other
#endif
  ;

  Person person;
  const std::string load = ReadJsonSource(&person, " {\"happiness\": \"INDIFFERENT\", \"favoriteProject\": \"Other\"} ");
  REQUIRE(load == "");
  REQUIRE(person.happiness == happiness);
  REQUIRE(person.favoriteProject == project);
}

TEST_CASE("json_missing_project") {
  Person person;
  const std::string load = ReadJsonSource(&person, " {\"happiness\": 12} ");
  REQUIRE(load != "");
}


TEST_CASE("json_empty_document") {
  Person person;
  const std::string load = ReadJsonSource(&person, "{}");
  REQUIRE(load != "");
}


TEST_CASE("json_as_ints") {
  Person person;
  const std::string load = ReadJsonSource(&person, " {\"happiness\": 1, \"favoriteProject\": 2} ");
  REQUIRE(load != "");
}

#endif
