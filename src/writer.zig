//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0


const std = @import("std");
const File = std.fs.File;
const utils = @import("utils.zig");

pub const OutWriter = struct {
    pub const WriteFunction = *const fn(bts : []const u8) void;
    wfn : WriteFunction,

    const Self = @This();
    pub fn new(wf : WriteFunction) Self {
        return OutWriter{
            .wfn = wf,
        };

    }

    pub const OutWriterError = error{};

    pub fn write(self : Self , bts : []const u8) OutWriterError!usize {
        self.wfn(bts);
        return bts.len;
    }

    pub const Writer = std.io.Writer(OutWriter, OutWriterError, write);

    pub fn writer(self : Self) Writer{
        return .{
            .context = self,
        };
    }

};

pub const PanWriter = if (utils.IS_WASM) OutWriter else File.Writer;
