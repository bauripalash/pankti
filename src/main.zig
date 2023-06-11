//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

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
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const ga = gpa.allocator();

    //var fileToRun: ?[]u8 = null;

    //var args = try std.process.argsAlloc(ga);

    //if (args.len == 2) {
    //    fileToRun = try ga.alloc(u8, args[1].len);
    //    @memcpy(fileToRun.?, args[1]);
    //}
    //std.process.argsFree(ga, args);
    defer {
        _ = gpa.deinit();
    }
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
    
    //const con = try n.addConst(PValue.makeNumber(10.99));
    //const con2 = try n.addConst(PValue.makeNumber(200));
    //try n.write(ins.OpCode.Const, ins.InstPos.line(1));
    //try n.write_raw(con, ins.InstPos.line(1));
    //try n.write(ins.OpCode.Neg , ins.InstPos.line(1));
    //try n.write(ins.OpCode.Const, ins.InstPos.line(1));
    //try n.write_raw(con2, ins.InstPos.line(1));
    //try n.write(ins.OpCode.Div, ins.InstPos.line(1));
    //try n.write(ins.OpCode.Return, ins.InstPos.line(1));
    
    //std.debug.print("\n{any}\n\n" , .{n.pos});
    
    //n.disasm("a");

    var myv = Vm.newVm(ga);
    myv.bootVm();

    const rawSrc = try utils.u8tou32("show 1+2;", ga);
    const result = myv.interpret(rawSrc);
    print("VM Result=>{}\n", .{result});

    
    myv.freeVm();
    ga.free(rawSrc);

    //n.free();
    //
    //const n = PValue.makeNumber(100);
    //const f = n.asNumber();
    //const m = PValue.makeBool(false);
    //std.debug.print("{any}\n{}\n", .{n , f});
    //n.printVal();
    //std.debug.print("\n{any}\n", .{m});
    //m.printVal();

    //const name = "পলাশ";
    //const name = try openfile("sample/a.txt", ga);
    //const name32 = try utils.u8tou32(name, ga);
    //const name8 = try utils.u32tou8(name32, ga);
    //std.debug.print("\nu32 -> " , .{});
    //utils.printu32(name32);
    //std.debug.print("\nu8  -> {any}\n", .{name8});
    //std.debug.print("\nu8x  -> '{s}'\n", .{name8});
    //ga.free(name32);
    //ga.free(name8);
    //ga.free(name);

}


test "AllTest" {
    std.testing.refAllDecls(@This());
    _ = @import("lexer/lexer.zig");
    _ = @import("instruction.zig");
}
