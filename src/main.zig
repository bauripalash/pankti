//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const flags = @import("flags.zig");
const run = @import("run.zig");
const builtin = @import("builtin");
const utils = @import("utils.zig");
const abouttxt = @embedFile("emfiles/about.txt");
const pankti = @import("pankti.zig");

pub fn windowsOsSetup() void {
    if (builtin.target.os.tag == .windows) {
        const win = @cImport({
            @cInclude("windows.h");
        });

        const ioh = @cImport({
            @cInclude("io.h");
        });

        const locale = @cImport({
            @cInclude("locale.h");
        });

        _ = locale.setlocale(@as(c_int, 2), "bn_IN.utf8");
        _ = win.SetConsoleOutputCP(@as(c_uint, @bitCast(@as(c_int, 65001))));
        _ = ioh._setmode(@as(c_int, 1), @as(c_int, 131072));
    }
}

pub fn main() !void {
    if (utils.IS_WASM) return;
    windowsOsSetup();
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const ga = gpa.allocator();

    var fileToRun: ?[]u8 = null;

    const args = try std.process.argsAlloc(ga);

    if (args.len == 2) {
        const flag = args[1];

        if (std.mem.eql(
            u8,
            flag,
            "-v",
        ) or std.mem.eql(
            u8,
            flag,
            "--version",
        )) {
            std.debug.print("{s}\n", .{pankti.build_options.version_string});
        } else if (std.mem.eql(u8, flag, "-h") or std.mem.eql(
            u8,
            flag,
            "--help",
        )) {
            std.debug.print(abouttxt, .{pankti.build_options.version_string});
        } else {
            fileToRun = try ga.alloc(u8, args[1].len);
            @memcpy(fileToRun.?, args[1]);
        }
    } else if (args.len == 1) {
        std.debug.print(abouttxt, .{pankti.build_options.version_string});
        std.process.exit(0);
    }
    std.process.argsFree(ga, args);

    defer {
        if (fileToRun) |f| {
            ga.free(f);
        }
        _ = gpa.deinit();
    }

    if (fileToRun) |f| {
        const isOk = run.runFile(f);
        if (!isOk) {
            std.process.exit(1);
        }
    }
}

test "AllTest" {
    std.testing.refAllDecls(@This());
    _ = @import("lexer/lexer.zig");
    _ = @import("instruction.zig");
    _ = @import("vm.zig");
    _ = @import("compiler.zig");
}
