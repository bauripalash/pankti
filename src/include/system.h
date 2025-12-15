#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
// Operating System Detection
#if !defined (USING_CMAKE_BUILD) && !defined (USING_ZIG_BUILD)
// Build system didn't provide OS info, so we use compile time flags
#if defined (__linux) || defined (__linux__) || defined (linux) || defined (__gnu_linux__)
#define PANKTI_OS_LINUX 1
#endif

#if defined (_WIN16) || defined (_WIN32) || defined (_WIN64) || defined (__WIN32__) || defined (__WINDOWS__) || defined (__CYGWIN__)
#define PANKTI_OS_WIN 1
#endif

#if defined (macintosh) || defined (Macintosh) || defined (__APPLE__) || defined (__MACH__)
#define PANKTI_OS_MAC 1
#endif

#if defined (__EMSCRIPTEN__)
#define PANKTI_OS_WEB 1
#endif
#endif

#if defined (__amd64) || defined (__amd64__) || defined (__x86_64) || defined (__x86_64__) || defined (_M_X64) || defined (_M_AMD64)
#define PANKTI_ARCH_X86_64
#endif
#if defined (__arm__) || defined (__thumb__) || defined (__aarch64__) || defined (__ARM) || defined (_M_ARM) || defined (_M_ARMT) || defined (__arm)
#define PANKTI_ARCH_ARM
#endif

#if defined (i386) || defined (__i386) || defined (__i386__) || defined (__i686__) || defined (_M_IX86) || defined (__X86__) || defined (_X86_)
#define PANKTI_ARCH_X86
#endif

#if defined (PANKTI_OS_WIN)
#include <direct.h>
// _getcwd is basically same as nix getcwd
// More info: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/getcwd-wgetcwd
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

static inline int GetOsBitSize(void){
	if (sizeof(void*) == 8) {
		return 64;
	} else if (sizeof(void*) == 4){
		return 32;
	}

	return -1;
}

static inline char * GetOsUsername(void){
#if defined (PANKTI_OS_LINUX) || defined (PANKTI_OS_MAC)
	char * username = getenv("USER");
#elif defined (PANKTI_OS_WIN)
	char * username = getenv("USERNAME");
#else 
	return NULL;
#endif

	return username;
}

static inline char * GetHomeDir(void){
#if defined (PANKTI_OS_LINUX) || defined (PANKTI_OS_MAC)
	char * homedir = getenv("HOME");
#elif defined (PANKTI_OS_WIN)
	char * homedir = getenv("USERPROFILE");
#else 
	return NULL;
#endif

	return homedir;
}

#define MAX_CURDIR_LEN 4096
static inline char * GetCurDir(void){
	char * cdir = getcwd(NULL, MAX_CURDIR_LEN);
	if (cdir != NULL) {
		return cdir;
	}else{
		return NULL;
	}
}


#ifdef __cplusplus
}
#endif

#endif
