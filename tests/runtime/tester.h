#ifndef RUNTIME_TEST_TESTER_H
#define RUNTIME_TEST_TESTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../include/utest.h"
#include "../../src/include/system.h"

#ifdef PANKTI_OS_WIN
#define stat _stat
#define popen _popen
#define pclose _pclose
#endif

#define READ_BUFFER 2048
#define COMMAND_BUFF_SIZE 1024

static inline char * trimSpaces(char * str){
    char * end;
    char * ptr = str;
    while (isspace((unsigned char)*ptr)) {
        ptr++;
    }

    if (*ptr == 0) {
        return ptr;
    }

    end = ptr + strlen(ptr) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    end[1] = '\0';
    return ptr;
}
static inline bool matchOutput(const char * cmd, const char * golden, bool debug){
    char cmdBuf[READ_BUFFER];
    char gldBuf[READ_BUFFER];
    FILE * cmdFp;
    FILE * gldFp;
    if ((cmdFp = popen(cmd, "r")) == NULL) {
        printf("[ERROR] Failed to Run Script\n");
        return false;
    }

    if ((gldFp = fopen(golden, "r")) == NULL) {
        printf("[ERROR] Failed to read Golden file\n");
        return false;
    }
    int line = 1;
    while (fgets(cmdBuf, READ_BUFFER, cmdFp) != NULL && fgets(gldBuf, READ_BUFFER, gldFp) != NULL) {
        cmdBuf[strcspn(cmdBuf, "\n")] = '\0';
        gldBuf[strcspn(gldBuf, "\n")] = '\0';
        char * tempCmd = trimSpaces(cmdBuf);
        char * tempGld = trimSpaces(gldBuf);
        int cmpRes = strcmp(tempGld, tempCmd);
        if (cmpRes != 0) {
            printf("[ERROR] [Line %d] : Output Mismatch\n", line);
            printf("[ERROR] Expected : '%s'\n", tempGld);
            printf("[ERROR] Got      : '%s'\n", tempCmd);
            return false;
        }else{
            if (debug) {
                printf("[DEBUG] [Line %d] : Output Matched : '%s'\n", line, tempGld);
            }
        }

        line++;
    }

    if (pclose(cmdFp) == -1) {
        printf("[ERROR] Command exited with error\n");
        return false;
    }

    fclose(gldFp);
    return true;
}

static inline bool RunGolden(const char * script){
    char * panktiPath = getenv("PANKTI_BIN");
    if (panktiPath == NULL) {
        printf("[ERROR] Pankti Binary Not Set\n");
        return false;
    }

    char * dir = getenv("SAMPLES_DIR");
    if (dir == NULL) {
        printf("[ERROR] Samples Directory Not Set\n");
        return false;
    }

    char * enableDebugEnv = getenv("DEBUG_TESTS");
    bool enableDebug = false;
    if (enableDebugEnv != NULL) { 
        enableDebug = true;
    }

    struct stat dirInfo;
    char command[COMMAND_BUFF_SIZE];
    char scriptPath[COMMAND_BUFF_SIZE];
    char goldenPath[COMMAND_BUFF_SIZE];
    if (stat(dir, &dirInfo) == 0) { 
        int written = snprintf(scriptPath, COMMAND_BUFF_SIZE, "%s/%s.pn", dir, script);
        if (written < 0 || written >= COMMAND_BUFF_SIZE) {
            printf("[ERROR] Failed to create script path string\n");
            return false; //todo
        }
        written = 0;
        written = snprintf(command, COMMAND_BUFF_SIZE, "%s %s/%s.pn", panktiPath, dir, script);
        if (written < 0 || written >= COMMAND_BUFF_SIZE) {
            printf("[ERROR] Failed to create command string\n");
            return false; //todo
        }
        written = 0;
        written = snprintf(goldenPath, COMMAND_BUFF_SIZE, "%s/%s.golden.pn", dir, script);
        if (written < 0 || written >= COMMAND_BUFF_SIZE) {
            printf("[ERROR] Failed to create golden path string\n");
            return false; //todo
        }
    }else{
        printf("[ERROR] Failed to Access Samples Directory : '%s'\n", dir);
        return false;
    }

    if (enableDebug) {
        printf("[INFO] Pankti Binary: '%s'\n", panktiPath);
        printf("[INFO] Script Path: '%s'\n", scriptPath);
        printf("[INFO] Golden Path: '%s'\n", goldenPath);
    }
    
    //clock_t pTic = clock();
    //printf("[RUNNING] : %s\n", name);
    if (matchOutput(command, goldenPath, enableDebug)){
        //clock_t pToc = clock();
        //printf("[PASSED] : %s (%f sec)\n",name, (double)(pToc - pTic) / CLOCKS_PER_SEC);
        return true;
    }else{
        //printf("[Failed] : %s\n", name);
        return false;
    }

}

#define GoldenTest(script) \
    {\
    bool isok = RunGolden(script);\
    if (!isok){\
        ASSERT_TRUE_MSG(isok, "Golden Run Failed!");\
    }\
    }


#ifdef __cplusplus
}
#endif

#endif
