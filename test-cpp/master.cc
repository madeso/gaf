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

TEST_CASE("custom vector equals works") {
  std::vector<int> a;
  a.push_back(1);
  a.push_back(2);
  a.push_back(3);

  REQUIRE_THAT(a, Equals(Vector<int>() << 1 << 2 << 3));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("std constructor") {
  Standard fb;
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

TEST_CASE("std json") {
  Standard fb;
  const std::string load = ReadJsonSource(&fb,
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
  REQUIRE(load == "");

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("array constructor") {
  Arrays fb;
  REQUIRE(fb.a.size() == 0);
  REQUIRE(fb.b.size() == 0);
  REQUIRE(fb.c.size() == 0);
  REQUIRE(fb.d.size() == 0);
  REQUIRE(fb.ua.size() == 0);
  REQUIRE(fb.ub.size() == 0);
  REQUIRE(fb.uc.size() == 0);
  REQUIRE(fb.ud.size() == 0);
  REQUIRE(fb.e.size() == 0);
  REQUIRE(fb.f.size() == 0);
  REQUIRE(fb.g.size() == 0);
  REQUIRE(fb.h.size() == 0);
  REQUIRE(fb.i.size() == 0);
  REQUIRE(fb.standard.size() == 0);
}

#if GAF_TEST_JSON

TEST_CASE("array empty json") {
  Arrays advanced;

  const std::string load = ReadJsonSource(&advanced, "{}");
  REQUIRE(load != "");
}

TEST_CASE("array basic json") {
  Arrays fb;
  const std::string load = ReadJsonSource(&fb,
    "{\
    \"a\": [],\
    \"b\": [],\
    \"c\": [],\
    \"d\": [],\
    \"ua\": [],\
    \"ub\": [],\
    \"uc\": [],\
    \"ud\": [],\
    \"e\": [],\
    \"f\": [],\
    \"g\": [],\
    \"h\": [],\
    \"i\": [],\
    \"standard\": []}"
  );
  REQUIRE(load == "");

  REQUIRE(fb.a.size() == 0);
  REQUIRE(fb.b.size() == 0);
  REQUIRE(fb.c.size() == 0);
  REQUIRE(fb.d.size() == 0);
  REQUIRE(fb.ua.size() == 0);
  REQUIRE(fb.ub.size() == 0);
  REQUIRE(fb.uc.size() == 0);
  REQUIRE(fb.ud.size() == 0);
  REQUIRE(fb.e.size() == 0);
  REQUIRE(fb.f.size() == 0);
  REQUIRE(fb.g.size() == 0);
  REQUIRE(fb.h.size() == 0);
  REQUIRE(fb.i.size() == 0);
  REQUIRE(fb.standard.size() == 0);
}

TEST_CASE("arrays advanced json") {
  Arrays fb;
  const std::string load = ReadJsonSource(&fb,
    "{\
    \"a\": [1, 2],\
    \"b\": [3, 4],\
    \"c\": [5, 6],\
    \"d\": [7, 8],\
    \"ua\": [9, 10],\
    \"ub\": [11, 12],\
    \"uc\": [13, 14],\
    \"ud\": [15, 16],\
    \"e\": [17, 18],\
    \"f\": [false, true],\
    \"g\": [19, 20],\
    \"h\": [21, 22],\
    \"i\": [\"cat\", \"dog\"],\
    \"standard\": []}"
  );
  REQUIRE(load == "");

  REQUIRE_THAT(fb.a, Equals(Vector<std::int8_t>()  << 1 << 2));
  REQUIRE_THAT(fb.b, Equals(Vector<std::int16_t>() << 3 << 4));
  REQUIRE_THAT(fb.c, Equals(Vector<std::int32_t>() << 5 << 6));
  REQUIRE_THAT(fb.d, Equals(Vector<std::int64_t>() << 7 << 8));

  REQUIRE_THAT(fb.ua, Equals(Vector<std::uint8_t>()   << 9 << 10));
  REQUIRE_THAT(fb.ub, Equals(Vector<std::uint16_t>() << 11 << 12));
  REQUIRE_THAT(fb.uc, Equals(Vector<std::uint32_t>() << 13 << 14));
  REQUIRE_THAT(fb.ud, Equals(Vector<std::uint64_t>() << 15 << 16));

  REQUIRE_THAT(fb.e, Equals(Vector<char>() << 17 << 18));
  REQUIRE_THAT(fb.f, Equals(Vector<bool>() << false << true));
  REQUIRE_THAT(fb.g, Equals(Vector<float>() << 19 << 20));
  REQUIRE_THAT(fb.h, Equals(Vector<double>() << 21 << 22));

  REQUIRE_THAT(fb.i, Equals(Vector<std::string>() << "cat" << "dog"));
}


#endif

