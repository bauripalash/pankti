const std = @import("std");
const utils = @import("utils.zig");
const object = @import("object.zig");
const Vm = @import("vm.zig").Vm;
const Pobj = object.PObj;

const StringTableContext = struct {
    pub fn eql(self : @This() , a : []const u32 , b : []const u32 , index : usize) bool{
        _ = index;
        _ = self;
        return std.mem.eql(u32, a, b);
    }

    pub fn hash(self : @This() , key : []const u32) u32{
        _ = self;
        
        const result = utils.hashU32(key) catch 0;
        //std.debug.print("<{any}->{d}>\n\n", .{key , result});
        return @intCast(u32, result);
    }
};

pub fn StringTable() type{
    return std.ArrayHashMapUnmanaged([]const u32, *Pobj.OString, StringTableContext, true);
}

pub fn freeStringTable(vm : *Vm , table : StringTable()) bool {
    std.debug.print("{d}" , .{table.keys().len});
    for (table.values()) |value| {
        value.free(vm);
    }
    return true;
}
