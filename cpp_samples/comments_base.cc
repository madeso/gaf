#define CATCH_CONFIG_MAIN
#include "catch.hpp"


TEST_CASE("constructor") {
  Foo foo;
  CHECK(foo.hello == 0);
  CHECK(foo.world == Approx(0.0f));
  CHECK(foo.dog == "");
}

TEST_CASE("setter") {
  Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;
  foo.dog = "dog";
  REQUIRE(foo.hello == 42);
  REQUIRE(foo.world == 4.2f);
  CHECK(foo.dog == "dog");
}
