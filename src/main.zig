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

    var fileToRun: ?[]u8 = null;

    var args = try std.process.argsAlloc(ga);

    if (args.len == 2) {
        fileToRun = try ga.alloc(u8, args[1].len);
        @memcpy(fileToRun.?, args[1]);
    }
    std.process.argsFree(ga, args);


    defer {
        if (fileToRun) |f| {
            ga.free(f);
        }
        _ = gpa.deinit();
    }
    if (fileToRun) |f| {
        const text = try openfile(f, ga);
        const u = try utils.u8tou32(text, ga);
        var myv = Vm.newVm(ga);
        myv.bootVm();

        const result = myv.interpret(u);
        print("VM Result => {}\n" , .{result});
        myv.freeVm();
        ga.free(u);
        ga.free(text);
    } else{
        var myv = Vm.newVm(ga);
        myv.bootVm();
        const rawSrc = try utils.u8tou32("1!=2", ga);
        const result = myv.interpret(rawSrc);
        print("VM Result=>{}\n", .{result});
        myv.freeVm();
        ga.free(rawSrc);
    }
}


test "AllTest" {
    std.testing.refAllDecls(@This());
    _ = @import("lexer/lexer.zig");
    _ = @import("instruction.zig");
}
