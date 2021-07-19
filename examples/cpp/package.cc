#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gaf_package.h"

TEST_CASE("package constructor") {
  const test::Foo foo;
  REQUIRE(foo.hello == 0);
  REQUIRE(foo.world == 0.0f);
}

TEST_CASE("package setter") {
  test::Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;
  REQUIRE(foo.hello == 42);
  REQUIRE(foo.world == 4.2f);
}
