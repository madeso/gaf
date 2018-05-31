#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"
#include "readjsonsource.h"

TEST_CASE("pointer constructor") {
  Foo foo;
  REQUIRE(foo.value == nullptr);

  FooRoot bar;
  REQUIRE(bar.name == nullptr);
  REQUIRE(bar.foo == nullptr);
}

TEST_CASE("optional constructor") {
  Bar foo;
  REQUIRE(foo.value == 0);

  BarRoot bar;
  REQUIRE(bar.name == "");
  REQUIRE(bar.foo.value == 0);
}

#if GAF_TEST_JSON

TEST_CASE("pointer json_basic") {
  Foo foo;
  const std::string load = ReadJsonSource(&foo, " {\"value\": 12} ");
  REQUIRE(load == "");
  REQUIRE(foo.value != nullptr);
  REQUIRE(*foo.value == 12);
}

TEST_CASE("optional json_basic") {
  Bar foo;
  const std::string load = ReadJsonSource(&foo, " {\"value\": 12} ");
  REQUIRE(load == "");
  REQUIRE(foo.value == 12);
}

TEST_CASE("pointer json_missing_foo") {
  FooRoot bar;
  const std::string load = ReadJsonSource(&bar, " {\"name\": \"good dog\"} ");
  REQUIRE(load == "");
  REQUIRE(bar.name != nullptr);
  REQUIRE(*bar.name == "good dog");
  REQUIRE(bar.foo == nullptr);
}

TEST_CASE("optional json_missing_foo") {
  BarRoot bar;
  const std::string load = ReadJsonSource(&bar, " {\"name\": \"good dog\"} ");
  REQUIRE(load == "");
  REQUIRE(bar.name == "good dog");
  REQUIRE(bar.foo.value == 0);
}

TEST_CASE("pointer json_invalid_value_for_name") {
  // optional means optional, not accept if invalid
  FooRoot bar;
  const std::string load = ReadJsonSource(&bar, " {\"name\": 3} ");
  REQUIRE(load != "");
}

TEST_CASE("optional json_invalid_value_for_name") {
  // optional means optional, not accept if invalid
  BarRoot bar;
  const std::string load = ReadJsonSource(&bar, " {\"name\": 3} ");
  REQUIRE(load != "");
}


TEST_CASE("pointer json_empty_document") {
  FooRoot bar;
  const std::string load = ReadJsonSource(&bar, "{}");
  REQUIRE(load == "");
  REQUIRE(bar.name == nullptr);
  REQUIRE(bar.foo == nullptr);
}

TEST_CASE("optional json_empty_document") {
  BarRoot bar;
  const std::string load = ReadJsonSource(&bar, "{}");
  REQUIRE(load == "");
  REQUIRE(bar.name == "");
  REQUIRE(bar.foo.value == 0);
}


TEST_CASE("pointer json_optional_struct") {
  FooRoot bar;
  const std::string load = ReadJsonSource(&bar, "{\"foo\": {\"value\": 5}}");
  REQUIRE(load == "");
  CHECK(bar.name == nullptr);
  CHECK(bar.foo != nullptr);
  CHECK(bar.foo->value != nullptr);
  CHECK(*bar.foo->value == 5);
}

TEST_CASE("optional json_optional_struct") {
  BarRoot bar;
  const std::string load = ReadJsonSource(&bar, "{\"foo\": {\"value\": 5}}");
  REQUIRE(load == "");
  CHECK(bar.name == "");
  CHECK(bar.foo.value == 5);
}

#endif
