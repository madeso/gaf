struct Foo {
  // the range property asserts that value are in the range
  // from(inclusive) to (exclusive)
  [range(1, 5)]
  int32 value;

  // so valid values for value in this case are
  // 1, 2, 3 and 4
}
