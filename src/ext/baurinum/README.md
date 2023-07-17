# baurinum 🐢
**Simple and somewhat fast prototype bignum library written in C and Zig**

>Simplicity over speed

## Why
There exists many optimized production grade `bignum` libraries, but why 
create own? You might ask. This project has been started as a part of 
[CPank](https://github.com/bauripalash/pankti), the interpreter for pankti
programming language to provide a small and minimal bignum library for internal
usage of Cpank, but I have decided to move out the project to its own separate
repository.

The Cpank uses baurinum's C version, before than it uses to use GNU GMP and MPFR 
library but those libraries are huge, my interpreter binary's size 
went from 397KB size to 2490KB in an instant; plus, it is really a hassle to 
install GMP and MPFR for usage with MSVC and Visual Studio, so I decided to 
write my own minimal bignum library. 

This project is heavily inspired from books such as Handbook of Applied 
Cryptography and importantly elementary school books. I am getting also inspired
from the awesome library [tommath](https://github.com/libtom/libtommath).

After the Cpank was deprecated in favour of `neopank`, cpank counterpart written 
in Zig, I developed a Zig version of this Library.

The `C` version can be found at the `c` subdirectory, while the `zig` version 
can be found on the `zig` subdirectory.

## Building (C Version)
This project doesn't depend on any external libraries, just the c standard 
library, the project is written with C11 standard, you will need a C11 
compatiable to build the project, most modern compilers such as GCC, Clang, MSVC
would do the job, on *nix systems or on windows MSYS2, just run `make` and it  
will build it instantly, for MSVC you might need to create special scripts,
mostly 1 or 2 lines do the job.

## Building (Zig Version)
Just install the latest master build from zig's official site and install it,
then just run `zig build` or `make`

## License
MIT

## ⚠️ Motto
Simplicity over Speed
