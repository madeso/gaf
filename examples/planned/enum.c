enum Happiness {
    SAD, INDIFFERENT, HAPPY
}

// unless specified, type defaults to something like int32
enum Project : byte {
    PROTOBUF, GAF, OTHER
}

struct Person {
  Happiness happiness;
  Project favoriteProject;
}
