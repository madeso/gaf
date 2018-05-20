#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "config.h"
#if GAF_TEST_HEADER_ONLY
#define GENERATED_MYGAF_IMPLEMENTATION
#endif

#include "mygaf.h"
#include "readjsonsource.h"

using Catch::Matchers::Equals;

template<typename T>
struct Vector : public std::vector<T>
{
  Vector<T>& operator<<(const T& t) {this->push_back(t); return *this;}
};

TEST_CASE("constructor") {
  const Basic basic;
  REQUIRE(basic.some_ints.size() == 0);
  REQUIRE(basic.some_strings.size() == 0);

  const Advanced advanced;
  REQUIRE(advanced.basics.size() == 0);
}

TEST_CASE("custom vector equals works") {
  std::vector<int> a;
  a.push_back(1);
  a.push_back(2);
  a.push_back(3);

  REQUIRE_THAT(a, Equals(Vector<int>() << 1 << 2 << 3));
}

TEST_CASE("vector works") {
  Basic basic;
  basic.some_ints.push_back(41);
  basic.some_strings.push_back("dog");

  Advanced advanced;
  advanced.basics.push_back(basic);

  REQUIRE_THAT(basic.some_ints, Equals(Vector<int>() << 41));
  REQUIRE_THAT(basic.some_strings, Equals(Vector<std::string>() << "dog"));
  // REQUIRE_THAT(advanced.basics, Equals(Vector<Basic>() << basic));
}

#if GAF_TEST_JSON

TEST_CASE("basic_emptyjson") {
  Basic basic;

  const char* const load = ReadJsonSource(&basic, "{}");
  REQUIRE(load != nullptr);
}

TEST_CASE("advanced_emptyjson") {
  Advanced advanced;

  const char* const load = ReadJsonSource(&advanced, "{}");
  REQUIRE(load != nullptr);
}

TEST_CASE("basic_basic_json") {
  Basic basic;

  const char* const load = ReadJsonSource(&basic, "{\"some_ints\": [], \"some_strings\": []}");
  REQUIRE(load == nullptr);

  REQUIRE(basic.some_ints.size() == 0);
  REQUIRE(basic.some_strings.size() == 0);
}

TEST_CASE("advanced_basic_json") {
  Advanced advanced;

  const char* const load = ReadJsonSource(&advanced, "{\"basics\": [] }");
  REQUIRE(load == nullptr);

  REQUIRE(advanced.basics.size() == 0);
}

TEST_CASE("basic_advanced_json") {
  Basic basic;

  const char* const load = ReadJsonSource(&basic, "{\"some_ints\": [10, 20], \"some_strings\": [\"horse\", \"fish\"]}");
  REQUIRE(load == nullptr);

  REQUIRE_THAT(basic.some_ints, Equals(Vector<int>() << 10 << 20));
  REQUIRE_THAT(basic.some_strings, Equals(Vector<std::string>() << "horse" << "fish"));
}

TEST_CASE("advanced_advanced_json") {
  Advanced advanced;

  const char* const load = ReadJsonSource(&advanced, "{\"basics\": [{\"some_ints\": [10, 20], \"some_strings\": [\"horse\", \"fish\"]}] }");
  REQUIRE(load == nullptr);

  Basic basic;
  basic.some_ints.push_back(5);
  basic.some_ints.push_back(7);
  basic.some_strings.push_back("abc");
  basic.some_strings.push_back("def");

  // REQUIRE_THAT(advanced.basics, Equals(Vector<Basic>() << basic));
}

#endif
