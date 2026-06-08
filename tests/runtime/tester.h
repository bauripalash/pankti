/*
 * Copyright (c) 2022 Palash Bauri
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

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

static bool matchLines(FILE * inputPtr, FILE * goldenPtr, bool debug, int * line){
	if (inputPtr == NULL || goldenPtr == NULL || line == NULL) {
		return false;
	}
	char inputBuf[READ_BUFFER];
	char goldenBuf[READ_BUFFER];

	int ln = 1; 
	bool hasInput = fgets(inputBuf, READ_BUFFER, inputPtr) != NULL;
	bool hasGolden = fgets(goldenBuf, READ_BUFFER, goldenPtr) != NULL;

	while(hasInput || hasGolden){
		char * tempInput = hasInput ? trimSpaces(inputBuf) : "";
		char * tempGolden = hasGolden ? trimSpaces(goldenBuf) : "";

		int cmp = strcmp(tempGolden, tempInput);
		if (cmp != 0) {
		    printf("[ERROR] [Line %d] : Output Mismatch\n", ln);
            printf("[ERROR] Expected : '%s'\n", tempGolden);
            printf("[ERROR] Got      : '%s'\n", tempInput);
			*line = ln;
            return false;
		}
		if (debug) {
			printf("[DEBUG] [Line %d] : Output Matched : '%s'\n", ln, tempGolden);
		}
		hasInput = fgets(inputBuf, READ_BUFFER, inputPtr) != NULL;
		hasGolden = fgets(goldenBuf, READ_BUFFER, goldenPtr) != NULL;
		ln++;
	}
	*line = ln;
	return true;
}

static inline bool matchFiles(const char * inputFile, const char * goldenFile, bool debug){
	FILE * inputFp = fopen(inputFile, "r");
	if (inputFp == NULL) {
		printf("Failed to read input file : %s\n", inputFile);
		return false;
	}

	FILE * goldFp = fopen(goldenFile, "r");
	if (goldFp == NULL) {
		fclose(inputFp);
		printf("Failed to read golden file : %s\n", goldenFile);
		return false;
	}

	int line = 1;
	bool match = matchLines(inputFp, goldFp, debug, &line);
	fclose(inputFp);
	fclose(goldFp);
	return match;

}

static inline bool matchOutput(const char * cmd, const char * golden, bool debug){
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

	bool match = matchLines(cmdFp, gldFp, debug, &line);
	if (!match) {
		fclose(gldFp);
		pclose(cmdFp);
		return false;
	}

    if (pclose(cmdFp) == -1) {
		fclose(gldFp);
        printf("[ERROR] Command exited with error\n");
        return false;
    }

    fclose(gldFp);
    return true;
}

static inline bool RunErrorGolden(const char * script){
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
    bool enableDebug = (enableDebugEnv != NULL);

	char errorsDir[COMMAND_BUFF_SIZE];
	int errDirWrite = snprintf(errorsDir, COMMAND_BUFF_SIZE, "%s/errors", dir);
	if (errDirWrite < 0 || errDirWrite >= COMMAND_BUFF_SIZE) {
        printf("[ERROR] Failed to write error samples directory name\n");
		return false;
	}

	struct stat dirInfo;
	if (stat(errorsDir, &dirInfo) != 0) {
	    printf("[ERROR] Failed to Access Samples Directory : '%s'\n", errorsDir);
        return false;
	}

    char command[COMMAND_BUFF_SIZE];
    char scriptPath[COMMAND_BUFF_SIZE];
    char goldenPath[COMMAND_BUFF_SIZE];
	char tmpPath[COMMAND_BUFF_SIZE];

	int w = snprintf(scriptPath, COMMAND_BUFF_SIZE, "%s/%s.pn", errorsDir, script);
	if (w < 0 || w >= COMMAND_BUFF_SIZE) {
	    printf("[ERROR] Failed to create script path string\n");
    	return false;
	}
	w = 0;
	w = snprintf(goldenPath, COMMAND_BUFF_SIZE, "%s/%s.error.pn", errorsDir, script);
	if (w < 0 || w >= COMMAND_BUFF_SIZE) {
	    printf("[ERROR] Failed to create script path string\n");
    	return false;
	}

	w = 0;

#if defined (PANKTI_OS_WIN)
	w = snprintf(tmpPath, COMMAND_BUFF_SIZE, "%s\\stderr.tmp", dir);
#else
	w = snprintf(tmpPath, COMMAND_BUFF_SIZE, "%s/stderr.tmp", dir);
#endif
	if (w < 0 || w >= COMMAND_BUFF_SIZE) {
	    printf("[ERROR] Failed to create temporary file path string\n");
    	return false;
	}
	w = 0;
#if defined (PANKTI_OS_WIN)
	w = snprintf(command, COMMAND_BUFF_SIZE, "%s %s 2>%s >NUL",panktiPath, scriptPath, tmpPath);
#else
	w = snprintf(command, COMMAND_BUFF_SIZE, "%s %s 2>%s >/dev/null",panktiPath, scriptPath, tmpPath);
#endif
	if (w < 0 || w >= COMMAND_BUFF_SIZE) {
	    printf("[ERROR] Failed to create command string\n");
    	return false;
	}

    if (enableDebug) {
        printf("[INFO] Pankti Binary: '%s'\n", panktiPath);
        printf("[INFO] Script Path: '%s'\n", scriptPath);
        printf("[INFO] Golden Path: '%s'\n", goldenPath);
		printf("[INFO] Command : '%s'\n", command);
	}

	int ret = system(command);
	(void)ret;

	bool result = matchFiles(tmpPath, goldenPath, enableDebug);

	remove(tmpPath);

	return result;
	
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
    
    if (matchOutput(command, goldenPath, enableDebug)){
        return true;
    }else{
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

#define ErrorTest(script) \
    {\
    bool isok = RunErrorGolden(script);\
    if (!isok){\
        ASSERT_TRUE_MSG(isok, "Error Golden Run Failed!");\
    }\
    }


#ifdef __cplusplus
}
#endif

#endif
