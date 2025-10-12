def x(a,b):
    def o(c):
        print(a + b + c)

    return o

lx = x
print(lx)
cx = lx(1,2)
print(cx)
print(cx(3))
