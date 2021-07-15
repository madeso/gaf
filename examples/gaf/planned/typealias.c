// set int to actually be a int32
typedef int32 int;

struct Foo {
  int hello;
  float world;
}

// also works for structs and other constructions
typedef Foo Fred;

struct Bar {
  Fred foo;
  int8 bar;
}
