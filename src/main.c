#include "include/argparse.h"
#include "include/core.h"
#include "include/flags.h"
#include "include/printer.h"
#include "include/terminal.h"
#include "include/utils.h"
#include <locale.h>
#include <stdlib.h>

#ifdef PANKTI_OS_WIN
#include <fcntl.h>
#include <io.h>
#include <windows.h>
void setupWindows(void) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

    DWORD outMode = 0;
    DWORD errMode = 0;

    if (GetConsoleMode(hOut, &outMode)) {
        SetConsoleMode(hOut, outMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    if (GetConsoleMode(hErr, &errMode)) {
        SetConsoleMode(hErr, errMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
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
    InitTermInfo();
    InitDebugFlags();

    PanktiArgs args;
    PanArgsResult argRes = ParsePanArgs(argc, argv, &args);

    switch (argRes) {
        case PARGS_EXIT_OK: return EXIT_SUCCESS;
        case PARGS_EXIT_ERR: return EXIT_FAILURE;
        case PARGS_OK: break;
    }

    if (args.scriptPath != NULL) {
        const char *filePath = args.scriptPath;
        if (!DoesFileExists(filePath)) {
            PanPrint("Failed to open file : '%s'\n", filePath);
            return EXIT_FAILURE;
        }

        PanktiCore *core = NewCore(filePath);

        if (core == NULL) {
            PanPrint("Error: Failed to initialize Pankti Runtime\n");
            return EXIT_FAILURE;
        }
        core->scriptArgCount = args.scriptArgCount;
        core->scriptArgs = args.scriptArgs;
        RunCore(core);
        FreeCore(core);
    }

    return EXIT_SUCCESS;
}
