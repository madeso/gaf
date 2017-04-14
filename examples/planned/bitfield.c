bitfield Adultness {
    IS_GROWNUP,
    HAS_CAR,
    HAS_DOG,
    PAYS_BILLS,

    // aliases and combinations must come last
    IS_RESPONSIBLE = PAYS_BILLS,
    CAN_TAKE_DOG_TO_VET = HAS_CAR | HAS_DOG
    // more types of combinations?
}

struct Foo {
  Adultness adultness;
}
