# GAF

"We care about performance, except when we don't"

[![windows](https://github.com/madeso/gaf/workflows/windows/badge.svg)](https://github.com/madeso/gaf/actions?query=workflow%3Awindows)
[![linux](https://github.com/madeso/gaf/workflows/linux/badge.svg)](https://github.com/madeso/gaf/actions?query=workflow%3Alinux)
[![macos](https://github.com/madeso/gaf/workflows/macos/badge.svg)](https://github.com/madeso/gaf/actions?query=workflow%3Amacos)

# What is it

In short gaf is a "**GA**me **F**ormat" file definition like google protobuf but for "games".

Given the following gaf definition:

```C++
struct Basic {
  int32 hello;
  float world;
}

struct Advanced {
  Basic basics[];
}
```

Gaf will generate:

```C++
class Basic {
 public:
  Basic();

  int32_t hello;
  float world;
};

class Advanced {
 public:
  std::vector<Basic> basics;
};
```

Not much, I agree, but by opting in to additional features you can get:

-   a rapidjson interface for reading/writing json files (reading works, writing doesn't)
-   dear imgui function for easy-editor functionality (works but needs some love)
-   option to disable features, like std::string, to get faster binary serialization (not yet)

# Versus protobuf

-   The code that protoc generates is pretty unreadable. Gaf aims to have readable as a programmer would write it.
-   Protobuf mangles your names according to the "google standard". Gaf doesn't care.
-   Protobuf forces you to use setters and getters. Gaf has none of those.
-   Protobuf has a runtime dependency on the protobuf library. Gaf has a small C++ library for common IO.
-   Gaf have no understanding of versions. It's up to you to handle that.
-   Protobuf has a reflection api. Gaf only has generated code, that could have a reflection api, but currently doesnt.
    ...but(!) it does have a python plugin system. Inherit from a interface and send that to the 'main'
    and generate whatever code your heart desires.

# How to use

Get a modern version of python (3.5 or better)
either use gaf.py directly or use gaf.cmake kinda like the protobuf cmake

```CMake
GAF_GENERATE_CPP(SOURCES GAF_SOURCES HEADERS GAF_HEADERS FILES path/to/my.gaf)
```
