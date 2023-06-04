const std = @import("std");
const openfile = @import("openfile.zig").openfile;
const lexer = @import("lexer.zig");
const print = std.debug.print;
const u8tou32 = @import("utils.zig").u8tou32;

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const ga = gpa.allocator();
    defer {
        _ = gpa.deinit();
    }
    const text = try openfile("a.txt", ga);
    const u = try u8tou32(text, ga);
    //print("->{any}\n", .{@TypeOf(text)});
    
    var l = lexer.Lexer.new(u);
    l.debug();
    

    ga.free(u);
    ga.free(text);
}
