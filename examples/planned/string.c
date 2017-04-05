const int8 MAX_LENGTH = 255;

[layout(inline), string(value, length)]
struct String {
  [range(0, MAX_LENGTH)]
  int8 length;
  char value[MAX_LENGTH];
}

struct Person {
  String name;
}
