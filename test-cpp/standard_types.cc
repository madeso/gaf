#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"
#include "readjsonsource.h"

TEST_CASE("constructor") {
  FooBar fb;
  REQUIRE(fb.a == 0);
  REQUIRE(fb.b == 0);
  REQUIRE(fb.c == 0);
  REQUIRE(fb.d == 0);
  REQUIRE(fb.ua == 0);
  REQUIRE(fb.ub == 0);
  REQUIRE(fb.uc == 0);
  REQUIRE(fb.ud == 0);
  REQUIRE(fb.e == 0);
  REQUIRE(fb.f == false);
  REQUIRE(fb.g == 0);
  REQUIRE(fb.h == 0);
  REQUIRE(fb.i == "");
}

#if GAF_TEST_JSON

TEST_CASE("json") {
  FooBar fb;
  const char* const load = ReadJsonSource(&fb,
    "{\
    \"a\": 1,\
    \"b\": 2,\
    \"c\": 3,\
    \"d\": 4,\
    \"ua\": 5,\
    \"ub\": 6,\
    \"uc\": 7,\
    \"ud\": 8,\
    \"e\": 9,\
    \"f\": true,\
    \"g\": 10.0,\
    \"h\": 11.0,\
    \"i\": \"dog\"}"
  );
  REQUIRE(load == nullptr);

  REQUIRE(fb.a == 1);
  REQUIRE(fb.b == 2);
  REQUIRE(fb.c == 3);
  REQUIRE(fb.d == 4);
  REQUIRE(fb.ua == 5);
  REQUIRE(fb.ub == 6);
  REQUIRE(fb.uc == 7);
  REQUIRE(fb.ud == 8);
  REQUIRE(fb.e == 9);
  REQUIRE(fb.f == true);
  REQUIRE(fb.g == 10.0);
  REQUIRE(fb.h == 11.0);
  REQUIRE(fb.i == "dog");
}

#endif
