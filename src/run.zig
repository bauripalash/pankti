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
const IntrpResult = _vm.IntrpResult;
const flags = @import("flags.zig");
const writer = @import("writer.zig");

const openfile = @import("openfile.zig").openfile;

fn _run(rawSource: []const u8, gc: *Gc) bool {
    const source = utils.u8tou32(rawSource, gc.hal()) catch |e| {
        std.debug.print(
            "Failed to convert UTF-8 encoded source to UTF-32 encoded text : {any}\n",
            .{e},
        );
        return false;
    };

    var myVm = Vm.newVm(gc.hal()) catch {
        std.debug.print("Failed to create a Vm\n", .{});
        return false;
    };

    myVm.bootVm(gc);

    const result = myVm.interpret(source);

    myVm.freeVm(gc.hal());
    gc.hal().free(source);
    if (flags.DEBUG_FINAL) {
        std.debug.print("VM RESULT -> {s}\n", .{result.toString()});
    }
    switch (result) {
        .Ok => return true,
        .RuntimeError => return false,
        .CompileError => return false,
    }
}

pub fn runCode16(rawSource: []const u16) bool {
    var handyGpa = std.heap.GeneralPurposeAllocator(.{}){};
    var gcGpa = std.heap.GeneralPurposeAllocator(.{}){};

    const handyAl = handyGpa.allocator();
    const gcAl = gcGpa.allocator();

    var gc = Gc.new(gcAl, handyAl) catch {
        std.debug.print("Failed to create a Garbage Collector\n", .{});
        return false;
    };

    gc.boot(std.io.getStdOut().writer(), std.io.getStdErr().writer());

    const source: []u8 = std.unicode.utf16leToUtf8Alloc(
        gc.hal(),
        rawSource,
    ) catch {
        std.debug.print("Failed encode source code as UTF-16", .{});
        return false;
    };

    defer {
        gc.hal().free(source);
        gc.freeGc(gcAl);
        _ = handyGpa.deinit();
        _ = gcGpa.deinit();
    }

    return _run(source, gc);
}

pub fn runCode(source: []const u8) bool {
    var handyGpa = std.heap.GeneralPurposeAllocator(.{}){};
    var gcGpa = std.heap.GeneralPurposeAllocator(.{}){};

    const handyAl = handyGpa.allocator();
    const gcAl = gcGpa.allocator();

    var gc = Gc.new(gcAl, handyAl) catch {
        std.debug.print("Failed to create a Garbage Collector\n", .{});
        return false;
    };

    gc.boot(std.io.getStdOut().writer(), std.io.getStdErr().writer());

    defer {
        gc.freeGc(gcAl);
        _ = handyGpa.deinit();
        _ = gcGpa.deinit();
    }

    return _run(source, gc);
}

pub fn runFile(filepath: []const u8) bool {
    var handyGpa = std.heap.GeneralPurposeAllocator(.{}){};
    var gcGpa = std.heap.GeneralPurposeAllocator(.{}){};

    const handyAl = handyGpa.allocator();
    const gcAl = gcGpa.allocator();

    var gc = Gc.new(gcAl, handyAl) catch {
        std.debug.print("Failed to create a Garbage Collector\n", .{});
        return false;
    };

    //var a = std.ArrayList(u8).init(gc.hal());

    gc.boot(std.io.getStdOut().writer().any(), std.io.getStdErr().writer().any());
    //gc.boot(a.writer().any(), a.writer().any());

    const rawSource: []u8 = openfile(filepath, gc.hal()) catch {
        std.debug.print("[X] Failed to open file '{s}'", .{filepath});
        return false;
    };

    defer {
        gc.hal().free(rawSource);
        //a.deinit();
        gc.freeGc(gcAl);

        _ = handyGpa.deinit();
        _ = gcGpa.deinit();
    }
    const result = _run(rawSource, gc);

    //std.debug.print("{any}", .{a.items});

    return result;
}
