struct Rgb {
    byte red;
    byte green;
    byte blue;
}

struct Foo {
  Rgb white = Rgb(255, 255, 255); // set all the values
  Rgb black = (0, 0, 0); // name is implicit
  Rgb red = (255); // only set the first value, all others are default
  Rgb blue = (blue=255); // set a specific value, others are default
}
