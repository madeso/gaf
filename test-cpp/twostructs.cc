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

  const Bar bar;
  REQUIRE(bar.GetFoo().GetHello() == 0);
  REQUIRE(bar.GetFoo().GetWorld() == 0.0f);
  REQUIRE(bar.GetBar() == 0);
}

TEST_CASE("setter") {
  Foo foo;
  foo.SetHello(42);
  foo.SetWorld(4.2f);
  Bar bar;
  bar.SetFoo(foo);
  bar.SetBar(24);
  foo.SetHello(5);
  foo.SetWorld(5.5f);

  REQUIRE(bar.GetFoo().GetHello() == 42);
  REQUIRE(bar.GetFoo().GetWorld() == 4.2f);
  REQUIRE(bar.GetBar() == 24);

  bar.GetFooPtr()->SetHello(13);
  bar.GetFooPtr()->SetWorld(3.7f);
  REQUIRE(bar.GetFoo().GetHello() == 13);
  REQUIRE(bar.GetFoo().GetWorld() == 3.7f);
}

#ifdef GAF_TEST_JSON

TEST_CASE("json_basic") {
  Foo foo;
  const char* const load = foo.ReadJsonSource(" {\"hello\": 12, \"world\": 2.4} ");
  REQUIRE(load == nullptr);
  REQUIRE(foo.GetHello() == 12);
  REQUIRE(foo.GetWorld() == 2.4f);
}

#endif
