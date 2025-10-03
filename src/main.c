#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/token.h"
#include "include/utils.h"
#include "external/stb/stb_ds.h"

#define FLPATH "a.pank"

int main(int argc, char ** argv){
	printf("Pankti Programming Language\n");
	char * str = ReadFile(FLPATH);
	printf("Source -> \n>>%s<<\n", str);

	Lexer * lx = NewLexer(str);
	Token ** toks = ScanTokens(lx);
	for (int i = 0; i < arrlen(toks); i++) {
		printf("%d -> " , i);
		PrintToken(toks[i]);
		printf("\n");
	}

	free(str);
}

