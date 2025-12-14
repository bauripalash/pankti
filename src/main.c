#include "include/core.h"
#include "include/version.h"
#include <locale.h>
#include <stdio.h>


#ifdef PANKTI_OS_WIN
#include <io.h>
#include <windows.h>
void setupWindows() {
    SetConsoleOutputCP(65001);
	SetConsoleCP(CP_UTF8);
	__setmode(__fileno(stdout), _O_U8TEXT);
	__setmode(__fileno(stderr), _O_U8TEXT);
}
#endif

void setupOs(void) {
    setlocale(LC_ALL, "en_US.UTF-8");
#ifdef PANKTI_OS_WIN
    setupWindows();
#elif PANKTI_OS_MAC
    printf(
        "WARNING : Pankti Should Run on Apple computers."
        "But This platform is not officially supported.\n"
    );
#endif
}

int main(int argc, char **argv) {
    setupOs();
    if (argc < 2) {
		printf("Pankti Programming Language v%s\n", PANKTI_VERSION);
		printf("Usage: pankti [FILENAME]\n");
        return 1;
    } else {
        char *filepath = argv[1];
        PanktiCore *core = NewCore(filepath);
        if (core == NULL) {
            printf("Error: Failed to initialize Pankti Runtime\n");
            return 2;
        }
        RunCore(core);
        FreeCore(core);
    }
}
