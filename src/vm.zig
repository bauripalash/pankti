const std = @import("std");
const ins = @import("instruction.zig");

pub const IntrpResult = enum(u8) {
    Ok,
    CompileError,
    RuntimeError,
};

pub const Vm = struct {
    ins : ins.Instruction,
    al : std.mem.Allocator,
    ip : u8,

    const Self = @This();

    pub fn newVm(al : std.mem.Allocator) Vm{
        return Vm{
            .al = al,
            .ins = undefined,
            .ip = 0,
        };
    }

    pub fn bootVm(self : *Self) void {
        self.ins = ins.Instruction.init(self.al);
    }

    pub fn freeVm(self : *Self) void {
        self.ins.free();
    }


};
