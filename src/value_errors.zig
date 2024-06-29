pub const CopyError = error{
    Arr_FailedToCreateNewArray,
    Arr_InsertItems,
    Arr_ItemCopyError,
    Hmap_NewHmap,
    Hmap_AddPair,
    Hmap_ItemCopyError,
    String_NewString,
    BigInt_NewBigInt,
    BigInt_InitInt,
    BigInt_AddDigit,
    ErrObj_NewErrObj,
    ErrObj_CopyError,
};

pub fn copyErrorToString(e: CopyError) []const u8 {
    switch (e) {
        CopyError.Arr_FailedToCreateNewArray => {
            return "Failed to create new array while copying";
        },
        CopyError.Arr_InsertItems => {
            return "Failed to insert newly copied item to new array";
        },
        CopyError.Arr_ItemCopyError => {
            return "Failed to copy array items";
        },

        CopyError.Hmap_NewHmap => {
            return "Failed to create new hashmap while copying";
        },
        CopyError.Hmap_AddPair => {
            return "Failed to insert newly copied key,value to new hashmap";
        },
        CopyError.Hmap_ItemCopyError => {
            return "Failed to copy hashmap items";
        },

        CopyError.String_NewString => {
            return "Failed to copy string";
        },

        CopyError.BigInt_NewBigInt => {
            return "Failed to create new big integer while copying";
        },
        CopyError.BigInt_InitInt => {
            return "Failed to initialize big integer";
        },
        CopyError.BigInt_AddDigit => {
            return "Failed to copy big integer";
        },

        CopyError.ErrObj_CopyError,
        CopyError.ErrObj_NewErrObj,
        => {
            return "Failed to create copy of error object";
        },
    }
}
