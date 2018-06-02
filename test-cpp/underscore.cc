#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_UNDERSCORE_IMPLEMENTATION
#endif

#include "gaf_underscore.h"

TEST_CASE("constructor") {
  Foo foo;
  CHECK(foo.hello == 0);
  CHECK(foo.world == Approx(0.0f));
  CHECK(foo.dog == "");
  CHECK(foo.cat == false);
}

TEST_CASE("setter") {
  Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;
  foo.dog = "dog";
  foo.cat = true;
  REQUIRE(foo.hello == 42);
  REQUIRE(foo.world == 4.2f);
  CHECK(foo.dog == "dog");
  CHECK(foo.cat == true);
}
