#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"
#include "readjsonsource.h"

TEST_CASE("constructor") {
  Foo foo;
  REQUIRE(foo.value == nullptr);

  Bar bar;
  REQUIRE(bar.name == nullptr);
  REQUIRE(bar.foo == nullptr);
}

TEST_CASE("setter") {
  Bar bar;
  bar.name = std::make_shared<std::string>("dog");
  bar.foo = std::make_shared<Foo>();

  REQUIRE(*bar.name == "dog");
  REQUIRE(bar.foo->value == nullptr);
}

#if GAF_TEST_JSON

TEST_CASE("json_basic") {
  Foo foo;
  const char* const load = ReadJsonSource(&foo, " {\"value\": 12} ");
  REQUIRE(load == nullptr);
  REQUIRE(foo.value != nullptr);
  REQUIRE(*foo.value == 12);
}

TEST_CASE("json_missing_foo") {
  Bar bar;
  const char* const load = ReadJsonSource(&bar, " {\"name\": \"good dog\"} ");
  REQUIRE(load == nullptr);
  REQUIRE(bar.name != nullptr);
  REQUIRE(*bar.name == "good dog");
  REQUIRE(bar.foo == nullptr);
}

TEST_CASE("json_invalid_value_for_name") {
  // optional means optiona, not accept if invalid
  Bar bar;
  const char* const load = ReadJsonSource(&bar, " {\"name\": 3} ");
  REQUIRE(load != nullptr);
}


TEST_CASE("json_empty_document") {
  Bar bar;
  const char* const load = ReadJsonSource(&bar, "{}");
  REQUIRE(load == nullptr);
  REQUIRE(bar.name == nullptr);
  REQUIRE(bar.foo == nullptr);
}


TEST_CASE("json_optional_struct") {
  Bar bar;
  const char* const load = ReadJsonSource(&bar, "{\"foo\": {\"value\": 5}}");
  REQUIRE(load == nullptr);
  CHECK(bar.name == nullptr);
  CHECK(bar.foo != nullptr);
  CHECK(bar.foo->value != nullptr);
  CHECK(*bar.foo->value == 5);
}

#endif
