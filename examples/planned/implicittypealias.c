// for a few types, for convinence if it isnt aliased already, we automatically add the definition
// this should ofc be able to be disabled on the commandlione

struct Person {
  int age;  // always aliased to int32
  string name;  // default to some array type of bytes or some sort?
}
