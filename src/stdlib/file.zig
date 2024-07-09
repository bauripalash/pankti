//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const value = @import("../value.zig");
const Vm = @import("../vm.zig").Vm;
const PValue = value.PValue;
const utils = @import("../utils.zig");

pub const Name: []const u8 = "file";

fn fileExists(path: []const u8) ?std.fs.File.Stat {
    const stat = std.fs.cwd().statFile(path) catch {
        return null;
    };

    return stat;
}

pub const NameFuncWrite: []const u8 = "write";
pub fn file_Write(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (utils.IS_WASM) {
        return PValue.makeError(vm.gc, "write(...) is not supported on web editor").?;
    }

    if (argc != 3) {
        return PValue.makeError(vm.gc, "write(path, content , mode) requires a 3 arguments").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "write(path, ... , ...) first argument \"path\" must be a string").?;
    }

    if (!values[0].asObj().isString()) {
        return PValue.makeError(vm.gc, "write(path, ... , ...) first argument \"path\" must be a string").?;
    }

    if (!values[1].asObj().isString()) {
        return PValue.makeError(vm.gc, "write(..., content , ...) second argument \"content\" must be a string").?;
    }

    if (!values[1].isObj()) {
        return PValue.makeError(vm.gc, "write(..., content , ...) second argument \"content\" must be a string").?;
    }

    if (!values[2].isObj()) {
        return PValue.makeError(vm.gc, "write(..., ... , mode) third argument \"mode\" must be a string").?;
    }

    if (!values[2].asObj().isString()) {
        return PValue.makeError(vm.gc, "write(..., ... , mode) third argument \"mode\" must be a string").?;
    }

    const rawFileMode = values[2].toString(vm.gc.hal()) catch {
        return PValue.makeError(vm.gc, "Failed to read `mode`").?;
    };

    if (rawFileMode.len != 1) {
        return PValue.makeError(vm.gc, "Invalid file mode in write(...) function; must be either 'w' or 'a'").?;
    }

    var fileMode: u8 = 'w';

    if (rawFileMode[0] != 'w' and rawFileMode[0] != 'a') {
        vm.gc.hal().free(rawFileMode);
        return PValue.makeError(vm.gc, "Invalid file mode in write(...) function; must be either 'w' or 'a'").?;
    }

    if (rawFileMode[0] == 'a') {
        fileMode = 'a';
    }

    vm.gc.hal().free(rawFileMode);

    const rawFilePath = values[0].toString(vm.gc.hal()) catch {
        return PValue.makeError(vm.gc, "Failed to read file name in write(...)").?;
    };

    var file: std.fs.File = undefined;

    if (fileMode == 'w') {
        file = std.fs.cwd().createFile(rawFilePath, .{}) catch {
            vm.gc.hal().free(rawFilePath);
            return PValue.makeError(vm.gc, "failed to create a new file specified in write(...)").?;
        };
    } else {
        file = std.fs.cwd().openFile(rawFilePath, .{ .mode = .read_write }) catch {
            vm.gc.hal().free(rawFilePath);
            return PValue.makeError(vm.gc, "failed to open file specified in write(...)").?;
        };
    }

    const content = values[1].asObj().toString(vm.gc.hal()) catch {
        vm.gc.hal().free(rawFilePath);
        return PValue.makeError(vm.gc, "Failed to read content to write in write(...)").?;
    };

    var written: usize = 0;

    if (fileMode == 'w') {
        written = file.write(content) catch {
            vm.gc.hal().free(rawFilePath);
            vm.gc.hal().free(content);
            return PValue.makeError(vm.gc, "Failed to write to file specified in write(...)").?;
        };
    } else {
        const stat = file.stat() catch {
            vm.gc.hal().free(rawFilePath);
            vm.gc.hal().free(content);
            return PValue.makeError(vm.gc, "Failed to get file information for appending for write(...)").?;
        };

        file.seekTo(stat.size) catch {
            vm.gc.hal().free(rawFilePath);
            vm.gc.hal().free(content);
            return PValue.makeError(vm.gc, "Failed to write to file specified in write(...)").?;
        };

        written = file.write(content) catch {
            vm.gc.hal().free(rawFilePath);
            vm.gc.hal().free(content);
            return PValue.makeError(vm.gc, "Failed to write to file specified in write(...)").?;
        };
    }

    file.sync() catch return {
        vm.gc.hal().free(rawFilePath);
        vm.gc.hal().free(content);
        return PValue.makeError(vm.gc, "Failed to sync file data after writing for write(...)").?;
    };

    const bw = @as(f64, @floatFromInt(written));
    vm.gc.hal().free(rawFilePath);
    vm.gc.hal().free(content);

    return PValue.makeNumber(bw);
}

pub const NameFuncRead: []const u8 = "read";
pub fn file_Read(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (utils.IS_WASM) {
        return PValue.makeError(vm.gc, "file(path) is not supported on web editor").?;
    }
    if (argc != 1) {
        return PValue.makeError(vm.gc, "read(path) requires a single argument").?;
    }

    if (!values[0].isObj()) {
        return PValue.makeError(vm.gc, "read(path) first argument must be a string").?;
    }

    if (!values[0].asObj().isString()) {
        return PValue.makeError(vm.gc, "read(path) first argument must be a string").?;
    }

    const rawFilePath = values[0].toString(vm.gc.hal()) catch return vm.gc.makeString("");

    const path = std.fs.cwd().openFile(rawFilePath, .{}) catch {
        vm.gc.hal().free(rawFilePath);
        return PValue.makeError(vm.gc, "read(...) failed to read the file specified").?;
    };
    defer path.close();

    const fileContent = path.readToEndAlloc(vm.gc.hal(), std.math.maxInt(usize)) catch {
        vm.gc.hal().free(rawFilePath);
        return PValue.makeError(vm.gc, "read(...) failed to read the file specified").?;
    };

    const f = utils.u8tou32(fileContent, vm.gc.hal()) catch {
        vm.gc.hal().free(rawFilePath);
        vm.gc.hal().free(fileContent);

        return PValue.makeError(vm.gc, "read(...) failed to convert file content to object").?;
    };

    const strObj = vm.gc.copyString(f, @intCast(f.len)) catch {
        vm.gc.hal().free(rawFilePath);
        vm.gc.hal().free(fileContent);
        vm.gc.hal().free(f);
        return PValue.makeError(vm.gc, "read(...) failed to convert file content to string object").?;
    };

    vm.stack.push(PValue.makeObj(strObj.parent())) catch {
        vm.gc.hal().free(rawFilePath);
        vm.gc.hal().free(fileContent);
        vm.gc.hal().free(f);
        return PValue.makeError(vm.gc, "read(...) failed to save file content to memory").?;
    };

    vm.gc.hal().free(rawFilePath);
    vm.gc.hal().free(fileContent);
    vm.gc.hal().free(f);

    return vm.stack.pop() catch return {
        return PValue.makeError(vm.gc, "read(...) failed to fetch file content to memory").?;
    };
}
