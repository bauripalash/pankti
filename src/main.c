#include "include/core.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PANKTI_OS_WIN
void setupWindows() {
#include <io.h>
    SetConsoleOutputCP(65001);
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
        printf("Error: Please provide a filename to run.\n");
        printf("Pankti Programming Language\n");
        printf("Usage: pankti [Filename]\n");
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
