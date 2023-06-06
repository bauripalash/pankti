pub const ObjType = enum(u8) {
    String,
    Function,
    Native,
    Closure,
    Upval,
    Mod,
    Array,
    Err,
    Hmap,
};

pub const PObj = struct {
    otype : ObjType,
    isMarked : bool,
};
