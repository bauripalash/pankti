const std = @import("std");
const Allocator = std.mem.Allocator;
const PObj = @import("object.zig").PObj;
const table = @import("table.zig");


pub const Gc = struct {
    inernal_al : Allocator,
    al : Allocator,
    objects : ?*PObj,
    strings : table.StringTable(),
    const Self = @This();

    pub fn new(gc : Allocator) !*Gc{
        const newgc = try gc.create(Self);
        newgc.* = .{
            .al = gc,
            .strings = table.StringTable(){},
            .inernal_al = gc,
            .objects = null,
        };

        return newgc;
        
    }

    pub fn boot(self : *Self , al : Allocator ) void {
        _ = self;
        _ = al;
        
    }

    pub fn getIntAlc(self : *Self) Allocator{
        return self.inernal_al;
    }

    pub fn getAlc(self : *Self) Allocator{
        return self.al;
    }

    pub fn newObj(self : *Self , comptime ParentType : type) !*ParentType{
        //_ = ParentType;

        //std.debug.print("{any}\n" , .{ParentType});
        const ptr = try self.al.create(ParentType);
        
        ptr.parent().objtype =  .Ot_String;
        ptr.parent().isMarked = false;
        ptr.parent().next = self.objects;
        self.objects = ptr.parent();
        return ptr;
    }

    pub fn newString(self : *Self , chars : []u32 , len : u32) !*PObj.OString{
        var ptr = try self.newObj(PObj.OString);
        
        ptr.chars = chars;
        ptr.len = len;

        ptr.obj.isMarked = true;
        return ptr;
    }

    pub fn copyString(self : *Gc, chars : []const u32, len : u32) !*PObj.OString{
        if (self.strings.get(chars)) |interned| {
            return interned;
        }
        //
        //
        //std.debug.print("{any}\n" , .{self.strings});
        const mem_chars = try self.al.alloc(u32, len);
        @memcpy(mem_chars, chars);

        return self.newString(mem_chars, len);

    }

    pub fn freeSingleObject(self : *Self , obj : *PObj) void {
        switch (obj.objtype) {
            .Ot_String => {
                const str_obj = obj.child(PObj.OString);
                str_obj.free(self);

                //self.getAlc().free(str_obj);
            }
        }

        return;
    }

    pub fn freeObjects(self : *Self) void{
        var object = self.objects;

        while (object) |obj| {
            const next = obj.next;
            self.freeSingleObject(obj);
            object = next;
        }
    }

    pub fn free(self : *Self) void{
        self.freeObjects();
        self.strings.deinit(self.al);
        self.al.destroy(self);
    }

};
