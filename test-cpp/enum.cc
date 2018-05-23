#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"

TEST_CASE("Person") {
  Person dude;
  CHECK(dude.happiness == Happiness::SAD);
  CHECK(dude.favoriteProject == Project::Protobuf);
}

TEST_CASE("enum types") {
  Happiness happiness = Happiness::HAPPY;
  Project project = Project::Gaf;

  Person dude;
  dude.happiness = happiness;
  dude.favoriteProject = project;

  CHECK(dude.happiness == happiness);
  CHECK(dude.favoriteProject == project);
}

// todo: need to add json loading tests
