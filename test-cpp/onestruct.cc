#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"

TEST_CASE("constructor") {
  Foo foo;
  CHECK(foo.hello == 0);
  CHECK(foo.world == Approx(0.0f));
}

TEST_CASE("setter") {
  Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;
  REQUIRE(foo.hello == 42);
  REQUIRE(foo.world == 4.2f);
}
