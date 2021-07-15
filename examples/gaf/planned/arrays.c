const int32 LIFE = 42;

struct Foo {
  int32 hello[42]; // arrays are always of constant size
  int32 hello2[LIFE]; // constants can also be used

  float world[int32]; // if a type is specified, another variable is created that specifies the size

  float world2[int32, int32]; // if a second type is specified, the first is the number of items and second value is actual size of the array

  float world3[int32, 42]; // the second type can also be a constant
  float world4[int32, LIFE];
}
