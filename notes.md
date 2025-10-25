===== Fib 35 =====
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

print fib(10)
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
