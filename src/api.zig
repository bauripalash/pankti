//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const Gc = @import("gc.zig").Gc;
const utils = @import("utils.zig");
const _vm = @import("vm.zig");
const Vm = _vm.Vm;
const flags = @import("flags.zig");
const writer = @import("writer.zig");

extern fn writeOut(ptr : usize , len : usize) void;
extern fn writeErr(ptr : usize , len : usize) void;

fn writeOutString(bts : []const u8) void {
    writeOut(@intFromPtr(bts.ptr), bts.len);
}

fn writeErrString(bts : []const u8) void {
    writeErr(@intFromPtr(bts.ptr), bts.len);
}

export fn runCodeApi(rawrawSource: [*]u8, len: u32) bool {
    const rawSource = rawrawSource[0..len];
    var handyGpa = std.heap.GeneralPurposeAllocator(.{}){};
    var gcGpa = std.heap.GeneralPurposeAllocator(.{}){};

    const handyAl = handyGpa.allocator();
    const gcAl = gcGpa.allocator();

    var gc = Gc.new(gcAl, handyAl) catch {
        //std.debug.print("Failed to create a Garbage Collector\n" , .{});
        return false;
    };

    const StdoutWriter = writer.OutWriter.new(writeOutString);
    const StderrWriter = writer.OutWriter.new(writeErrString);
    

    gc.boot(StdoutWriter.writer(), StderrWriter.writer());
    const source = utils.u8tou32(rawSource, gc.hal()) catch {
        //std.debug.print("Failed to convert UTF-8 encoded source to UTF-32 encoded text\n" , .{});
        return false;
    };

    var myVm = Vm.newVm(gc.hal()) catch {
        //std.debug.print("Failed to create a Vm\n" , .{});
        return false;
    };

    myVm.bootVm(gc);

    const result = myVm.interpret(source);

    myVm.freeVm(gc.hal());
    gc.hal().free(source);
    if (flags.DEBUG) {
        //std.debug.print("VM RESULT -> {s}\n" , .{result.toString()});
    }
    switch (result) {
        .Ok => return true,
        .RuntimeError => return false,
        .CompileError => return false,
    }
}
