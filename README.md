# GAF
"We care about performance, except when we don't"



[![Build Status](https://travis-ci.org/madeso/gaf.svg?branch=master)](https://travis-ci.org/madeso/gaf)

# What is it

In short gaf is a "**GA**me **F**ormat" file definition like google protobuf but for "games".

Given the following gaf definition:

    struct Basic {
      int32 hello;
      float world;
    }

    struct Advanced {
      Basic basics[];
    }

Gaf will generate:

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

Not much, I agree. By opting in to additional features you will get:

 * a rapidjson interface for reading/writing json files (in progress)
 * dear imgui function for easy-editor functionality (not yet)
 * option to disable features to get faster binary serialization (not yet)

# Versus protobuf

 * The code that protoc generates is pretty unreadable. Gaf aims to have readable as a programmer would write it.
 * Protobuf mangles your names according to the "google standard". Gaf doesn't care.
 * Protobuf forces you to use setters and getters. Gaf has none of those.
 * Protobuf has a runtime dependency on the protobuf library. Gaf only has standard C++.
 * Protobuf enums are only implemented one way and you have to care for name collisions. Gaf allows you to choose how your enumes should be implemented.
 * The protobuf code offer little options on the output. Gaf allows you to choose if you want a source file or a stb style implementation-in-header library.
 * Gaf have no understanding of versions. It's up to you to handle that.
 * Protobuf has a reflection api. Gaf only has generated code, that could have a reflection api, but currently doesnt.


# How to use

Get a modern version of python (3.5 or better)
either use gaf.py directly or use gaf.cmake like the protobuf cmake

    GAF_GENERATE_CPP(GAF_SOURCES GAF_HEADERS path/to/my.gaf)


