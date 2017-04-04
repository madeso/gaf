const int32 LIFE = 42;

struct Foo {
  int32 hello[42]; // arrays are always of constant size
  int32 hello2[LIFE]; // constants can also be used

  int32 world_size;
  float world[world_size]; // except dynamic arrays, where world_size is the size of the array

  int32 world2_items;
  int32 world2_size;
  float world2[world2_items, world2_size]; // if a second identifier is specified, the first is the number of items and second value is actual size of the array
}
