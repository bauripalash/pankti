const std = @import("std");
const lexer = @import("lexer/lexer.zig");

pub const Parser = struct {
    current : lexer.Token,
    previous : lexer.Token,
    lexer : lexer.Lexer,
    hadErr : bool,
    
    const Self = @This();

    fn errorAt(self : *Self, token : *lexer.Token , msg : []const u8) void {
        std.debug.print("[L {d}] Error", .{token.line});
        if (token.toktype == .Eof) {
            std.debug.print(" at end" , .{});
        } else if (token.toktype == .Err){

        } else {
            std.debug.print(" at '{s}'" , token.lexeme);
        }

        std.debug.print(": {s}\n" , msg);
        self.hadErr = true;
    }

    fn errCur(self : *Self , msg : []const u8) void{
        self.errorAt(&self.current, msg);
    }

    fn err(self : *Self , msg : []const u8) void{
        self.errorAt(&self.previous , msg);
    }

    fn advance(self : *Self) void {
       self.previous = self.current;
       while (true) {
            self.current = self.lexer.getToken();
            if (self.current.toktype != .Err) {
                break;
            }


       }
    }
};


