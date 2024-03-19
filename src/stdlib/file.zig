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

pub const Name = &[_]u32{ 'f', 'i', 'l', 'e' };
pub const NameFuncRead = &[_]u32{ 'r', 'e', 'a', 'd' };

pub fn file_Read(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (utils.IS_WASM) {
        return PValue.makeError(vm.gc, "file(path) is not supported on web editor").?;
    }
    if (argc != 1) {
        return PValue.makeError(vm.gc, "read(path) requires a single function").?;
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
