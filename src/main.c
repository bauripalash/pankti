#include "include/core.h"
#include <stdio.h>

int main(int argc, char **argv) {
	
	if (argc < 2) {
		printf("Error: Please provide a filename to run.\n");
		printf("Pankti Programming Language\n");
		printf("Usage: pankti [Filename]\n");
		return 1;
	} else {
		char * filepath = argv[1];
		PanktiCore *core = NewCore(filepath);
		RunCore(core);
    	FreeCore(core);
	}
}
