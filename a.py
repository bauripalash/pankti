def add(a,b):
    return a + b

def mul(a,b):
    return a * b

def comp(a):
    return add(mul(a, a), mul(a,2))

sum = 0
i = 0
limit = 100000

while i < limit:
    sum = sum + comp(i)
    i = i + 1

print(sum)
