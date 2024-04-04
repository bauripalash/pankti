const std = @import("std");
const ui = @import("ui");

const Gc = @import("gc.zig").Gc;
const utils = @import("utils.zig");
const Vm = @import("vm.zig").Vm;

pub fn on_closing(_: *ui.Window, _: ?*void) ui.Window.ClosingAction {
    ui.Quit();
    return .should_close;
}

var inputBox: *ui.MultilineEntry = undefined;
var outputBox: *ui.MultilineEntry = undefined;
var runButton: *ui.Button = undefined;

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

    const mainWin = try ui.Window.New("Hello World", 320, 240, .hide_menubar);
    mainWin.as_control().Show();

    const hbox = try ui.Box.New(.Vertical);

    inputBox = try ui.MultilineEntry.New(.NonWrapping);
    outputBox = try ui.MultilineEntry.New(.NonWrapping);

    runButton = try ui.Button.New("Run");

    mainWin.SetResizeable(true);

    mainWin.SetMargined(true);
    hbox.SetPadded(true);

    mainWin.SetChild(hbox.as_control());

    hbox.Append(inputBox.as_control(), .stretch);

    hbox.Append(runButton.as_control(), .dont_stretch);

    hbox.Append(outputBox.as_control(), .stretch);

    runButton.OnClicked(ui.MultilineEntry, on_runBtn, inputBox);

    mainWin.OnClosing(void, on_closing, null);

    ui.Main();
}

pub fn on_runBtn(_: *ui.Button, input: ?*ui.MultilineEntry) void {
    //std.debug.print("\n-->{s}<--\n", .{input.?.Text()});

    var handyGpa = std.heap.GeneralPurposeAllocator(.{}){};
    var gcGpa = std.heap.GeneralPurposeAllocator(.{}){};

    const handyAl = handyGpa.allocator();
    const gcAl = gcGpa.allocator();

    var gc = Gc.new(gcAl, handyAl) catch {
        std.debug.print("Failed to create Garbage Collector\n", .{});
        return;
    };

    var w = std.ArrayList(u8).init(gc.hal());

    gc.boot(w.writer().any(), w.writer().any());

    const rawSrc = std.mem.span(input.?.Text());

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
