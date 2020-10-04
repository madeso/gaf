#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "gaf_twostructs.h"
#include "readjsonsource.h"

TEST_CASE("constructor") {
  Foo foo;
  REQUIRE(foo.hello == 0);
  REQUIRE(foo.world == 0.0f);

  Bar bar;
  REQUIRE(bar.foo.hello == 0);
  REQUIRE(bar.foo.world == 0.0f);
  REQUIRE(bar.bar == "");
  REQUIRE(bar.b == false);
}

TEST_CASE("setter") {
  Foo foo;
  foo.hello = 42;
  foo.world = 4.2f;
  Bar bar;
  bar.foo = foo;
  bar.bar = "dog";
  bar.b = true;
  foo.hello =5;
  foo.world = 5.5f;

  REQUIRE(bar.foo.hello == 42);
  REQUIRE(bar.foo.world == 4.2f);
  REQUIRE(bar.bar == "dog");
  REQUIRE(bar.b == true);

  bar.foo.hello = 13;
  bar.foo.world = 3.7f;
  REQUIRE(bar.foo.hello == 13);
  REQUIRE(bar.foo.world == 3.7f);
}

#if GAF_TEST_JSON

TEST_CASE("json_basic") {
  Foo foo;
  const std::string load = ReadJsonSource(&foo, " {\"hello\": 12, \"world\": 2.4} ");
  REQUIRE(load == "");
  REQUIRE(foo.hello == 12);
  REQUIRE(foo.world == 2.4f);
}

TEST_CASE("json_double_can_be_ints") {
  Foo foo;
  const std::string load = ReadJsonSource(&foo, " {\"hello\": 12, \"world\": 2} ");
  REQUIRE(load == "");
  REQUIRE(foo.hello == 12);
  REQUIRE(foo.world == 2.0f);
}

TEST_CASE("json_missing_world") {
  Foo foo;
  const std::string load = ReadJsonSource(&foo, " {\"hello\": 12} ");
  REQUIRE(load != "");
}


TEST_CASE("json_empty_document") {
  Foo foo;
  const std::string load = ReadJsonSource(&foo, "{}");
  REQUIRE(load != "");
}


TEST_CASE("json_advanced") {
  Bar bar;
  const std::string load = ReadJsonSource(&bar, "{\"bar\": \"cat and dog\", \"b\": true, \"foo\": {\"hello\": 12, \"world\": 2.4}}");
  REQUIRE(load == "");
  CHECK(bar.foo.hello == 12);
  CHECK(bar.foo.world == 2.4f);
  CHECK(bar.bar == "cat and dog");
  CHECK(bar.b == true);
}

#endif
