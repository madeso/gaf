struct Struct
{
    int32 value;
}

enum Enum
{
    A, B, C
}

///////////////////////////////////////////////////////////////////////////////

struct Standard
{
    int8 a;
    int16 b;
    int32 c;
    int64 d;

    uint8 ua;
    uint16 ub;
    uint32 uc;
    uint64 ud;

    bool f;

    float g;
    double h;
    string i;

    Struct j;
    Enum k;
}

///////////////////////////////////////////////////////////////////////////////

struct Arrays
{
    int8 a[];
    int16 b[];
    int32 c[];
    int64 d[];

    uint8 ua[];
    uint16 ub[];
    uint32 uc[];
    uint64 ud[];

    bool f[];

    float g[];
    double h[];
    string i[];

    Struct j[];
    Enum k[];
}

///////////////////////////////////////////////////////////////////////////////

struct Optionals
{
    int8? a;
    int16? b;
    int32? c;
    int64? d;

    uint8? ua;
    uint16? ub;
    uint32? uc;
    uint64? ud;

    bool? f;

    float? g;
    double? h;
    string? i;

    Struct? j;
    Enum? k;
}

///////////////////////////////////////////////////////////////////////////////

struct FooRoot;

struct Foo
{
    int32? value;
    FooRoot? root;
}

struct FooRoot
{
    string? name;
    Foo? foo;
}

struct Bar
{
    int32 value?;
}

struct BarRoot
{
    string name?;
    Bar foo?;
}

