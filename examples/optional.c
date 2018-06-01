struct FooRoot;

struct Foo {
  int32? value;
  FooRoot? root;
}

struct FooRoot {
  string? name;
  Foo? foo;
}

struct Bar {
  int32 value?;
}

struct BarRoot {
  string name?;
  Bar foo?;
}

