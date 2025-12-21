> Saturday, October 25, 2025 7:58 PM (IST)

I ran this simple recursive Fibonacci program which calculates the 35th 
Fibonacci number.

```go,pankti
kaj fib(n)
	if n < 2 then
		return n
	end

	return fib(n-2) + fib(n-1)
end

show(fib(10))
```
Compiled with -Wall -flto -O3 flags. Using standard library `clock()` function
I ran some speed tests, using this method:

```c
#if defined DEBUG_TIMES
    clock_t tic = clock();
#endif

    <Lexing/Parsing/Interpreting>()

#if defined DEBUG_TIMES
    clock_t toc = clock();
    printf(
        "[DEBUG] Finished : %f sec.\n",
        (double)(toc - tic) / CLOCKS_PER_SEC
    );
#endif
```

I got the following results:
* *Lexer* finished within **0.000021 seconds**.
* *Parser* finished within **0.000010 seconds**.
* *Interpreter/Evaluator* finished within **13.588408 seconds**.

No GC has not been yet implemented, just plain malloc calls and free everything 
after finishing execution.

-------
> Wednesday, October 29, 2025 7:51 PM (IST)

I implemented a simple mark-sweep garbage collector, which tries to collect 
disconnected objects from memory, after each statement in main script is
finished.

I made it to use simple stack allocated *PValue* for evaluating statements
returns. Numbers, Bools, and Nil now stays in PValue, and PObj* Objects stay
as heap objects and only the pointer is kept in PValue. 

The Environment now uses hash tables from `stb_ds.h` instead of arrays.

This time instead of `-O3` flag, I am using `-O2`.

I got the following results after running fib(35) script: 
* *Lexer* finished within **0.000020 seconds**.
* *Parser* finished within **0.000009 seconds**.
* *Interpreter/Evaluator* finished within **11.863574 seconds**.

(Don't take the numbers too seriously, it can fluctuate according to system 
load)

I am using a ASUS Laptop with following specs:

```
CPU: AMD Ryzen 5 5600H with Integrated Radeon Graphics
RAM: 16GB
OS: Manjaro Linux
Kernel: 6.16.12
Desktop Environment: KDE Plasma 6.4.5
```

> Thursday, October 30, 2025 7:31 PM (IST)

I am using `mimalloc` library for memory allocation. 

Now function call arguments are passed as stack array if the number arguments 
are 16 or less, otherwise fallback to heap allocation. I am using plain dynamic 
array instead of stb_ds array.

For statement execution instead of returning values directly, I wrap them inside
an `ExResult` struct.

```c
typedef enum ExType{
	ET_SIMPLE,
	ET_BREAK,
	ET_RETURN,
}ExType;

typedef struct ExResult{
	PValue value;
	ExType type;
}ExResult;
```

All statement execution functions now return ExResult. With this we no longer 
need Return Object and Break Objects. With this most of the memory usage for 
fib(35) is gone. Before this most of the objects for that program was Return 
objects. 

Before this the Interpreter consume around 4-5 GB of RAM, now according to 
massif the peak usage is 26.4KiB (around 27KB) with system libc memory 
allocation functions, I removed mimalloc usage for this test to get the actual 
impact of this ExResult method.

With all these changes, Interpreter execution time for fib(35) is reduced to
around 8-9 seconds.

> Friday, October 31, 2025 9:19 AM (IST)

I removed mimalloc. The benchmark doesn't show any significant improvement with
or without mimalloc for fib(35).

> Friday, November 28, 2025 6:11 PM (IST)

I made some radical changes to the implementation, which I believe will be
benificial in the long run.

I replaced usage of size_t with uint64_t types. The type size_t is platform
dependent, thus array capacity, map capacity will be different on 64bit and 
32bit systems.

> Thursday, December 4, 2025 5:17 PM (IST)

I swapped stb_ds library's hash table based Environment with verstable.h based
hash table. The benchmarks showed, stb_ds's hash table was a huge factor in 
performance loss. With verstable, the fib(35) benchmarks finished within 3-4
seconds.

With the stb_ds's hash table, there was no way to reset the table, so env
free list was not doing anything for performance. verstable based env without
free list was still much more fast than stb_ds.

> Sunday, December 14, 2025 6:02 PM (IST)

== Building On Windows ==

The Interpreter builds fine as is on Windows and on Linux. Windows MSVC specific
build flags are added in CMakeLists.txt file. 

I have both Windows 11 and Arch based Manjaro installed. I have been using
Visual Studio 2022 for experimenting with Windows compatibility. The Project
was already compiling fine with MSVC, but it was throwing few warnings about
compiler and linker flags which are not supported in MSVC. 

From the beginning I made sure to not include any POSIX specific non-standard
library. Thus, Mingw-w64 based GCC, Clang, MSVC can easily compile the
Interpreter as is.

Currently the Makefile is being used as shortcut dispatcher for long commands 
such as `cmake -S . -B -DCMAKE_C_COMPILER=<..>` or cleaning the build directory
 & building for scratch or running python unittests for runtime. The Makefile
is already Windows NMake compatible except for the HEADERS, SOURCES fetching
with shell commands and NMake doesn't recognize .PHONY commands so as the CMake
builds the output in `build` directory, the build command doesn't work; to
simplify the shortcut dispatcher system instead of making the Makefile for
compatible with NMake I just created build.bat file for windows, it is just line
to line translation from Makefile.

== Minimum Supported Windows Version : Windows 7 ==

I must support Windows 7 32bit, there can't be any compromises with this. 
Currently the Interpreter will work fine on Windows 32bit. But there are two
problems that I am facing currently, Unicode support on `cmd.exe` and powershell
for Windows 7 is terrible. My name in Bengali has 3 graphemes and if in UTF-32
it will 4 codepoints. On Windows 7 cmd.exe or powershell it will be displayed as
`[?][?][?][?]`. Setting the Console Codepage does nothing new. I have to check
for ways to fix this as soon as possible.
