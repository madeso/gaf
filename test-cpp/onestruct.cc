#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"

TEST_CASE("constructor") {
  const Foo foo;
  REQUIRE(foo.GetHello() == 0);
  REQUIRE(foo.GetWorld() == 0.0f);
}

TEST_CASE("setter") {
  Foo foo;
  foo.SetHello(42);
  foo.SetWorld(4.2f);
  REQUIRE(foo.GetHello() == 42);
  REQUIRE(foo.GetWorld() == 4.2f);
}
