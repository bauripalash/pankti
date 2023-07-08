const osMod = @import("os.zig");
const vm = @import("../vm.zig");
const utils = @import("../utils.zig");
const gc = @import("../gc.zig");
const table = @import("../table.zig");
const PObj = @import("../object.zig").PObj;

pub const msl = struct {
    key : []const u32,
    func : PObj.ONativeFunction.NativeFn,

    pub fn m(key : []const u32 , func : PObj.ONativeFunction.NativeFn) msl{
        return msl{
            .key = key,
            .func = func,
        };
    }
};


pub fn _addStdlib(
    v : *vm.Vm,
    tab : *table.PankTable(),
    name: []const u32,
    func: PObj.ONativeFunction.NativeFn,
) !void {
    const nstr = try v.gc.copyString(name, @intCast(name.len));

    try v.stack.push(nstr.parent().asValue());
    var nf = try v.gc.newObj(.Ot_NativeFunc, PObj.ONativeFunction);

    nf.init(func);
    try v.stack.push(nf.parent().asValue());

    try tab.put(
        v.gc.hal(),
        v.stack.stack[0].asObj().asString(),
        v.stack.stack[1],
    );

    _ = try v.stack.pop();
    _ = try v.stack.pop();
}

fn _pushStdlib(v : *vm.Vm ,modname : []const u32 , items : []const msl) void {
    const nameHash = utils.hashU32(modname, v.gc) catch return;

    v.gc.stdlibs[v.gc.stdlibCount] = gc.StdLibMod.new();
    v.gc.stdlibs[v.gc.stdlibCount].name = modname;
    v.gc.stdlibs[v.gc.stdlibCount].hash = nameHash;
    v.gc.stdlibs[v.gc.stdlibCount].ownerCount = 0;

    v.gc.stdlibCount+=1;

    var i : usize = 0;

    while (i < items.len) : (i += 1) {
        _addStdlib(v, &v.gc.stdlibs[v.gc.stdlibCount - 1].items , items[i].key , items[i].func) catch continue;
    }

}

pub fn pushStdlibMath(v : *vm.Vm) void {
    _pushStdlib(v, 
            &[_]u32{'o' , 's'}, 
            &[_]msl{
                msl.m(&[_]u32{'n' , 'a' , 'm' , 'e'}, osMod.os_Name),
    });
}
