const std = @import("std");
const ui = @import("ui");

const Gc = @import("gc.zig").Gc;
const utils = @import("utils.zig");
const Vm = @import("vm.zig").Vm;

pub fn on_closing(_: *ui.Window, _: ?*void) !ui.Window.ClosingAction {
    ui.Quit();
    return .should_close;
}

var inputBox: *ui.MultilineEntry = undefined;
var outputBox: *ui.MultilineEntry = undefined;
var runButton: *ui.Button = undefined;

var handyGpa = std.heap.GeneralPurposeAllocator(.{}){};
var gcGpa = std.heap.GeneralPurposeAllocator(.{}){};

const handyAl = handyGpa.allocator();
const gcAl = gcGpa.allocator();

pub fn main() !void {
    var init_data = ui.InitData{
        .options = .{ .Size = 0 },
    };

    ui.Init(&init_data) catch {
        std.debug.print("Error init", .{});
        init_data.free_error();
        return;
    };

    defer ui.Uninit();

    const menu = try ui.Menu.New("File");
    const menuItemOpen = try menu.AppendItem("Open");
    const menuItemSave = try menu.AppendItem("Save");
    //const menuItemQuit = try menu.AppendQuitItem();

    menuItemOpen.OnClicked(void, ui.Error, on_OpenMenu, null);
    menuItemSave.OnClicked(void, ui.Error, on_SaveMenu, null);

    const mainWin = try ui.Window.New("Hello World", 320, 240, .hide_menubar);
    const hbox = try ui.Box.New(.Vertical);
    const buttonPanel = try ui.Box.New(.Horizontal);

    inputBox = try ui.MultilineEntry.New(.NonWrapping);
    outputBox = try ui.MultilineEntry.New(.NonWrapping);

    runButton = try ui.Button.New("Run");

    buttonPanel.Append(runButton.as_control(), .dont_stretch);

    mainWin.SetResizeable(true);

    mainWin.SetMargined(true);
    hbox.SetPadded(true);

    mainWin.SetChild(hbox.as_control());

    hbox.Append(buttonPanel.as_control(), .dont_stretch);
    hbox.Append(inputBox.as_control(), .stretch);

    hbox.Append(outputBox.as_control(), .stretch);

    runButton.OnClicked(void, ui.Error, on_runBtn, null);

    mainWin.OnClosing(void, ui.Error, on_closing, null);

    mainWin.as_control().Show();
    ui.Main();
}

pub fn on_OpenMenu(_: ?*ui.MenuItem, win: ?*ui.Window, data: ?*void) !void {
    _ = data;

    const f = std.mem.span(win.?.OpenFile());

    if (f) |fname| {
        const file = std.fs.cwd().openFile(fname, .{}) catch {
            win.?.MsgBoxError("File Open Error", "Failed to open file");
            return;
        };

        const fcontent = file.readToEndAlloc(handyAl, std.math.maxInt(usize)) catch {
            win.?.MsgBoxError("File Open Error", "Failed to open file");
            return;
        };

        const buffer = handyAl.alloc(u8, fcontent.len + 1) catch {
            win.?.MsgBoxError("File Open Error", "Failed to open file");
            return;
        };
        const string = std.fmt.bufPrintZ(buffer, "{s}", .{fcontent}) catch {
            win.?.MsgBoxError("File Open Error", "Failed to open file");
            return;
        };

        inputBox.SetText(string.ptr);

        defer {
            file.close();
            handyAl.free(fcontent);
        }
    } else {
        win.?.MsgBoxError("File Open Error", "Failed to open file");
        return;
    }

    return;
}

pub fn on_SaveMenu(_: ?*ui.MenuItem, win: ?*ui.Window, data: ?*void) !void {
    _ = data;
    const f = std.mem.span(win.?.SaveFile());
    if (f) |fname| {
        const file = std.fs.cwd().createFile(fname, .{}) catch {
            win.?.MsgBoxError("File Save Error", "Failed to save file");
            return;
        };

        const content = std.mem.span(inputBox.Text());

        defer file.close();
        _ = file.write(content) catch {
            win.?.MsgBoxError("File Save Error", "Failed to save file");
            return;
        };
    } else {
        win.?.MsgBoxError("File Save Error", "Failed to save file");
        return;
    }

    return;
}

pub fn on_runBtn(_: *ui.Button, _: ?*void) !void {
    //std.debug.print("\n-->{s}<--\n", .{input.?.Text()});

    const input = inputBox;
    var gc = Gc.new(gcAl, handyAl) catch {
        std.debug.print("Failed to create Garbage Collector\n", .{});
        return;
    };

    var w = std.ArrayList(u8).init(gc.hal());

    gc.boot(w.writer().any(), w.writer().any());

    const rawSrc = std.mem.span(input.Text());

    const src = utils.u8tou32(rawSrc, gc.hal()) catch {
        return;
    };

    var myVm = Vm.newVm(gc.hal()) catch {
        return;
    };

    myVm.bootVm(gc);

    _ = myVm.interpret(src);

    const res = gc.hal().alloc(u8, w.items.len + 1) catch {
        std.debug.print("Res Mem Alloc Failed", .{});
        return;
    };
    const news = std.fmt.bufPrintZ(res, "{s}", .{w.items}) catch |e| {
        std.debug.print("Buf Print Fail {any}", .{e});
        return;
    };

    defer {
        w.deinit();
        myVm.freeVm(gc.hal());
        gc.hal().free(src);
        gc.hal().free(res);
    }

    //std.debug.print("00{s}00", .{res});
    outputBox.SetText(news);

    return;
}
