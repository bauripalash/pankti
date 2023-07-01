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

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const ga = gpa.allocator();

    var fileToRun: ?[]u8 = null;

    var args = try std.process.argsAlloc(ga);

    if (args.len == 2) {
        fileToRun = try ga.alloc(u8, args[1].len);
        @memcpy(fileToRun.?, args[1]);
    } else if (args.len == 1) {
        std.debug.print("neopank 0.4.0\n", .{});
        std.debug.print("Usage: neopank [FILE]\n", .{});
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
        const isOk =  run.runFile(f);
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
