#include "include/core.h"
#include <stdio.h>
#define FLPATH "a.pank"

int main(int argc, char ** argv){
	printf("Pankti Programming Language\n");
	PanktiCore * core = NewCore(FLPATH);
	RunCore(core);

	FreeCore(core);
}

