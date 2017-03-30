[layout(inline)]
struct string {
  int8 length;
  char[255] value;
}

struct Foo {
  [range(1, 10)]
  int32 age;
  
  string name;
}
