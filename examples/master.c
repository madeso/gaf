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

  byte e;
  bool f;

  float g;
  double h;
  string i;
}

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

  byte e[];
  bool f[];

  float g[];
  double h[];
  string i[];

  Standard standard[];
}

