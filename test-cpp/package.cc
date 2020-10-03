#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"

#include "mygaf.h"

TEST_CASE("constructor") {
  const test::Foo foo;
  REQUIRE(foo.hello == 0);
  REQUIRE(foo.world == 0.0f);
}

TEST_CASE("setter") {
  test::Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;
  REQUIRE(foo.hello == 42);
  REQUIRE(foo.world == 4.2f);
}
