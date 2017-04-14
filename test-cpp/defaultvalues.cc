#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"

TEST_CASE("constructor") {
  const Foo foo;
  REQUIRE(foo.GetHello() == 12);
  REQUIRE(foo.GetWorld() == 3.14f);
}

TEST_CASE("setter") {
  Foo foo;
  foo.SetHello(42);
  foo.SetWorld(4.2f);
  REQUIRE(foo.GetHello() == 42);
  REQUIRE(foo.GetWorld() == 4.2f);
}

TEST_CASE("reset") {
  Foo foo;
  foo.SetHello(42);
  foo.SetWorld(4.2f);
  foo.Reset();
  REQUIRE(foo.GetHello() == 12);
  REQUIRE(foo.GetWorld() == 3.14f);
}
