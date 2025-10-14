from typing import Any


globalOne = None
globalTwo = None

def main():
    global globalOne
    a = "one"
    def one():
        print(a)
    globalOne = one

main()
globalOne()
