#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"

TEST_CASE("constructor") {
  const Foo foo;
  REQUIRE(foo.hello == 12);
  REQUIRE(foo.world == 3.14f);
  REQUIRE(foo.b == true);
  REQUIRE(foo.s == "dog");
}
