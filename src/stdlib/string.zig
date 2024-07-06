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
const stdlib = @import("stdlib.zig");
const PObj = @import("../object.zig").PObj;
const errs = @import("string_errors.zig");

//স্ট্রিং
pub const Name = &[_]u32{
    0x09b8,
    0x09cd,
    0x099f,
    0x09cd,
    0x09b0,
    0x09bf,
    0x0982,
};

pub const NameFuncUnicode = &[_]u32{
    0x0987,
    0x0989,
    0x09a8,
    0x09bf,
    0x0995,
    0x09cb,
    0x09a1,
};

pub fn str_Unicode(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeComptimeError(
            vm.gc,
            "Unicode Function takes a single argument",
            .{},
        ).?;
    }

    const rawUc = values[0];

    if (!rawUc.isNumber()) {
        return PValue.makeComptimeError(
            vm.gc,
            "First argument must be number",
            .{},
        ).?;
    }

    const raw_num = rawUc.asNumber();

    if (!utils.isInt(raw_num)) {
        return PValue.makeComptimeError(
            vm.gc,
            "Unicode number must be a integer",
            .{},
        ).?;
    }

    const number = utils.asInt(raw_num);

    if (number < 0 or number > 1114111) {
        return PValue.makeComptimeError(
            vm.gc,
            "Invalid unicode codepoint",
            .{},
        ).?;
    }

    const r = vm.gc.copyString(
        &[1]u32{@intCast(utils.asUint(raw_num))},
        1,
    ) catch {
        return PValue.makeComptimeError(
            vm.gc,
            "Failed to create string from unicode codepoint",
            .{},
        ).?;
    };

    return PValue.makeObj(r.parent());
}

pub const NameFuncString = &[_]u32{
    0x09b8,
    0x09cd,
    0x099f,
    0x09cd,
    0x09b0,
    0x09bf,
    0x0982,
};
pub fn str_String(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 1) {
        return PValue.makeComptimeError(
            vm.gc,
            errs.STR_STRING_ACCEPTS_SINGLE_ARG,
            .{argc},
        ).?;
    }

    const rawString = values[0].toString(vm.gc.hal()) catch {
        return vm.gc.makeString("");
    };
    const myString = utils.u8tou32(rawString, vm.gc.hal()) catch {
        vm.gc.hal().free(rawString);
        return vm.gc.makeString("");
    };
    const strObj = vm.gc.copyString(
        myString,
        @intCast(myString.len),
    ) catch {
        vm.gc.hal().free(rawString);
        return vm.gc.makeString("");
    };

    vm.stack.push(PValue.makeObj(strObj.parent())) catch {
        vm.gc.hal().free(rawString);
        return vm.gc.makeString("");
    };

    vm.gc.hal().free(rawString);
    vm.gc.hal().free(myString);

    return vm.stack.pop() catch return vm.gc.makeString("");
}
pub const NameFuncSplit = &[_]u32{ 0x09ad, 0x09be, 0x0997 };
pub fn str_Split(vm: *Vm, argc: u8, values: []PValue) PValue {
    if (argc != 2) {
        return PValue.makeComptimeError(
            vm.gc,
            errs.STR_SPLIT_ACCEPTS_TWO_ARGS,
            .{argc},
        ).?;
    }

    if (!values[0].isString() or !values[0].isString()) {
        const aS = values[0].toString(vm.gc.hal()) catch {
            return PValue.makeComptimeError(
                vm.gc,
                errs.STR_SPLIT_INT_CONVERT_A_ERROR,
                .{},
            ).?;
        };

        const bS = values[1].toString(vm.gc.hal()) catch {
            return PValue.makeComptimeError(
                vm.gc,
                errs.STR_SPLIT_INT_CONVERT_B_ERROR,
                .{},
            ).?;
        };
        const err = PValue.makeComptimeError(
            vm.gc,
            errs.STR_SPLIT_NOT_STRING_ARGS,
            .{
                aS,
                values[0].getTypeAsSimpleStr(),
                bS,
                values[1].getTypeAsSimpleStr(),
            },
        );

        vm.gc.hal().free(aS);
        vm.gc.hal().free(bS);

        return err.?;
    }

    const rawString = values[0].asObj().asString();

    const rawDelim = values[1].asObj().asString();

    var objArr = vm.gc.newObj(
        PObj.OType.Ot_Array,
        PObj.OArray,
    ) catch {
        return PValue.makeNil();
    };

    objArr.init();

    vm.stack.push(PValue.makeObj(objArr.parent())) catch {
        return PValue.makeNil();
    };

    var ite = std.mem.splitAny(u32, rawString.chars, rawDelim.chars);
    while (ite.next()) |s| {
        const objString = vm.gc.copyString(s, @intCast(s.len)) catch {
            return PValue.makeNil();
        };
        vm.stack.push(PValue.makeObj(objString.parent())) catch {
            return PValue.makeNil();
        };
        if (!objArr.addItem(vm.gc, vm.stack.pop() catch {
            return PValue.makeNil();
        })) {
            return PValue.makeNil();
        }
    }

    return vm.stack.pop() catch return PValue.makeNil();
}
