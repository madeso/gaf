#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"
#include "readjsonsource.h"

TEST_CASE("constructor") {
  const Foo foo;
  REQUIRE(foo.hello == 0);
  REQUIRE(foo.world == 0.0f);

  const Bar bar;
  REQUIRE(bar.foo.hello == 0);
  REQUIRE(bar.foo.world == 0.0f);
  REQUIRE(bar.bar == 0);
}

TEST_CASE("setter") {
  Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;
  Bar bar;
  bar.foo = foo;
  bar.bar = 24;
  foo.hello =5;
  foo.world = 5.5f;

  REQUIRE(bar.foo.hello == 42);
  REQUIRE(bar.foo.world == 4.2f);
  REQUIRE(bar.bar == 24);

  bar.foo.hello = 13;
  bar.foo.world = 3.7f;
  REQUIRE(bar.foo.hello == 13);
  REQUIRE(bar.foo.world == 3.7f);
}

#if GAF_TEST_JSON

TEST_CASE("json_basic") {
  Foo foo;
  const char* const load = ReadJsonSource(&foo, " {\"hello\": 12, \"world\": 2.4} ");
  REQUIRE(load == nullptr);
  REQUIRE(foo.hello == 12);
  REQUIRE(foo.world == 2.4f);
}

TEST_CASE("json_missing_world") {
  Foo foo;
  const char* const load = ReadJsonSource(&foo, " {\"hello\": 12} ");
  REQUIRE(load != nullptr);
}


TEST_CASE("json_empty_document") {
  Foo foo;
  const char* const load = ReadJsonSource(&foo, "{}");
  REQUIRE(load != nullptr);
}


TEST_CASE("json_advanced") {
  Bar bar;
  const char* const load = ReadJsonSource(&bar, "{\"bar\": 42, \"foo\": {\"hello\": 12, \"world\": 2.4}}");
  REQUIRE(load == nullptr);
  CHECK(bar.foo.hello == 12);
  CHECK(bar.foo.world == 2.4f);
  CHECK(bar.bar == 42);
}

#endif
