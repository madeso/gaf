#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "mygaf.h"

TEST_CASE("constructor") {
  const test::Foo foo;
  REQUIRE(foo.GetHello() == 0);
  REQUIRE(foo.GetWorld() == 0.0f);
}

TEST_CASE("setter") {
  test::Foo foo;
  foo.SetHello(42);
  foo.SetWorld(4.2f);
  REQUIRE(foo.GetHello() == 42);
  REQUIRE(foo.GetWorld() == 4.2f);
}