enum Happiness {
    SAD, INDIFFERENT, HAPPY
}

// unless specified, type defaults to something like int32
enum Project : byte {
    Protobuf, Gaf, Other
}

struct Person {
  Happiness happiness;
  Project favoriteProject;
}
