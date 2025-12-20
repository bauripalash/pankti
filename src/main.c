#include "include/core.h"
#include "include/printer.h"
#include "include/version.h"
#include <locale.h>

#ifdef PANKTI_OS_WIN
#include <fcntl.h>
#include <io.h>
#include <windows.h>
void setupWindows(void) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(CP_UTF8);
}
#endif

void setupOs(void) {
    setlocale(LC_ALL, "en_US.UTF-8");
#ifdef PANKTI_OS_WIN
    setupWindows();
#elif PANKTI_OS_MAC
    PanPrint(
        "WARNING : Pankti Should Run on Apple computers."
        "But This platform is not officially supported.\n"
    );
#endif
}

int main(int argc, char **argv) {
    setupOs();
    if (argc < 2) {
        PanPrint("Pankti Programming Language v%s\n", PANKTI_VERSION);
        PanPrint("Usage: pankti [FILENAME]\n");
        return 1;
    } else {
        char *filepath = argv[1];
        PanktiCore *core = NewCore(filepath);
        if (core == NULL) {
            PanPrint("Error: Failed to initialize Pankti Runtime\n");
            return 2;
        }
        RunCore(core);
        FreeCore(core);
    }
}
