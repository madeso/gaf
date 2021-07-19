#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("default values constructor")
{
  const Foo foo;
  REQUIRE(foo.hello == 12);
  REQUIRE(foo.world == 3.14f);
  REQUIRE(foo.b == true);
  REQUIRE(foo.s == "dog");
}
