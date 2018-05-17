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
}

TEST_CASE("setter") {
  Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;
  REQUIRE(foo.hello == 42);
  REQUIRE(foo.world == 4.2f);
}

TEST_CASE("reset") {
  Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;

  foo = Foo();
  REQUIRE(foo.hello == 12);
  REQUIRE(foo.world == 3.14f);
}
