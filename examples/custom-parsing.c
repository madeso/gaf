struct Bar {
    int32 value;
    int32 type;
}

// custom parsing is only valid for xml and json like files
// binary files are saved and written as is

struct Foo {
  [custom_parsing_load(Bar, "custom_load_hello"])]
  int32 hello;

  // this custom parsing allows to parse as if instead of a integer in Foo
  // there is a custom object named Bar defined earlier that will be read to
  // and then the custom_load_hello will be called when loading
  // this version property disallows writing, so no xml/json writing will be generated

  // other versions are custom_parsing and custom_parsing_write
}
