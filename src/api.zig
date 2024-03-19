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

extern fn writeStdout(ptr: usize, len: usize) void;
extern fn writeStderr(ptr: usize, len: usize) void;

fn writeOutString(bts: []const u8) void {
    writeStdout(@intFromPtr(bts.ptr), bts.len);
}

fn writeErrString(bts: []const u8) void {
    writeStderr(@intFromPtr(bts.ptr), bts.len);
}
const handyAl = std.heap.wasm_allocator;
const gcAl = std.heap.wasm_allocator;

export fn memAlloc(len: usize) usize {
    const result = handyAl.alloc(u8, len) catch return 0;
    return @intFromPtr(result.ptr);
}

export fn memFree(ptr: [*]const u8, len: usize) void {
    handyAl.free(ptr[0..len]);
}

export fn runCodeApi(rawrawSource: [*]const u8, len: u32) bool {
    const rawSource = rawrawSource[0..len];

    var gc = Gc.new(gcAl, handyAl) catch {
        return false;
    };

    const StdoutWriter = writer.OutWriter.new(writeOutString);
    const StderrWriter = writer.OutWriter.new(writeErrString);

    gc.boot(StdoutWriter.writer(), StderrWriter.writer());
    const source = utils.u8tou32(rawSource, gc.hal()) catch {
        return false;
    };

    var myVm = Vm.newVm(gc.hal()) catch {
        return false;
    };

    myVm.bootVm(gc);

    const result = myVm.interpret(source);

    myVm.freeVm(gc.hal());
    gc.hal().free(source);
    switch (result) {
        .Ok => return true,
        .RuntimeError => return false,
        .CompileError => return false,
    }
}
