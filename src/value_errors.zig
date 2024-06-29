pub const CopyError = error{
    NonSupportedObjects,
    Arr_NonSupportedObjects,
    Arr_FailedToCreateNewArray,
    Arr_InsertItems,
    Hmap_NonSupportedObjects,
    Hmap_NewHmap,
    Hmap_AddPair,
    String_NewString,
    BigInt_NewBigInt,
    BigInt_InitInt,
    BigInt_AddDigit,
};
