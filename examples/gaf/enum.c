enum Happiness
{
    SAD, INDIFFERENT, HAPPY, GLAD
}

// unless specified, type defaults to something like int32
enum Project : int8
{
    Protobuf, Gaf, Other
}

struct Person
{
    Happiness happiness = GLAD;
    Project favoriteProject;
}
