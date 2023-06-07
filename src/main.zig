//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifie: MPL-2.0

const std = @import("std");
const openfile = @import("openfile.zig").openfile;
const lexer = @import("lexer/lexer.zig");
const print = std.debug.print;
const utils = @import("utils.zig");
const ins = @import("instruction.zig");
const v = @import("vm.zig");
const Vm = v.Vm;
const IntrpResult = v.IntrpResult;
const PValue = @import("value.zig").PValue;

pub fn main() !void {
    //var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    //const ga = gpa.allocator();

    //var fileToRun: ?[]u8 = null;

    //var args = try std.process.argsAlloc(ga);

    //if (args.len == 2) {
    //    fileToRun = try ga.alloc(u8, args[1].len);
    //    @memcpy(fileToRun.?, args[1]);
    //}
    //std.process.argsFree(ga, args);
    //defer {
    //    _ = gpa.deinit();
    //}
    //print("{s}" , .{fileToRun.?});
    //if (fileToRun) |f| {
    //    const text = try openfile(f, ga);
    //    const u = try utils.u8tou32(text, ga);
    //    var l = lexer.Lexer.new(u);
    //    _ = l;
    //    //l.debug();
    //    ga.free(u);
    //    ga.free(f);
    //    ga.free(text);
    //}
    //const text = try openfile("a.txt", ga);
    //const text = "show(100+22.0 == 5.9-2.001);";
    //print("->{any}\n", .{@TypeOf(text)});
    //
    //
    //var n = ins.Instruction.init(ga);
    
    //try n.addConst(PValue.makeNumber(@floatCast(f64, 100)));
    //try n.write(ins.OpCode.Const, ins.InstPos.dummy());
    //try n.write_raw(0, ins.InstPos.dummy());
    //try n.write(ins.OpCode.Return, ins.InstPos.dummy());
    //n.disasm("a");

    //var myv = Vm.newVm(ga);
    //myv.bootVm();

    //const result = myv.interpret(&n);
    //print("VM Result=>{}", .{result});

    

    //n.free();
    //
    const n = PValue.makeNumber(100);
    const f = n.asNumber();
    const m = PValue.makeBool(false);
    std.debug.print("{any}\n{}\n", .{n , f});
    n.printVal();
    std.debug.print("\n{any}\n", .{m});
    m.printVal();

}


test "AllTest" {
    std.testing.refAllDecls(@This());
    _ = @import("lexer/lexer.zig");
    _ = @import("instruction.zig");
}
