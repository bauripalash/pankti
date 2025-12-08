#ifndef PANKTI_VERSION_H
#define PANKTI_VERSION_H
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Pankti follows SemVer
 */

// Pankti Major Version `X`.X.X
#ifndef PANKTI_VERSION_MAJOR
#define PANKTI_VERSION_MAJOR 0
#endif

// Pankti Major Version X.`X`.X
#ifndef PANKTI_VERSION_MINOR
#define PANKTI_VERSION_MINOR 6
#endif

// Pankti Major Version X.X.`X`
#ifndef PANKTI_VERSION_PATCH
#define PANKTI_VERSION_PATCH 0
#endif

// Release Level
// '_' = Normal Release (Default)
// `dev`, `alpha<NUMBER>`, `beta<NUMBER>`, rc<NUMBER> = Pre-release
#ifndef PANKTI_RELEASE_LEVEL
#define PANKTI_RELEASE_LEVEL "dev"
#endif

// Pankti Git Hash `abcdef`
#ifndef PANKTI_GIT_HASH
#define PANKTI_GIT_HASH         ""
#endif

// Pankti Git Count - Count from the last tag
#ifndef PANKTI_GIT_COUNT
#define PANKTI_GIT_COUNT        ""
#endif

#define PANKTI_STRINGIFY(x)     PANKTI_STRINGIFY_(x)
#define PANKTI_STRINGIFY_(x)    #x

// Pankti base version as string 'x.x.x'
#define PANKTI_VERSION_BASE     \
    PANKTI_STRINGIFY(PANKTI_VERSION_MAJOR) "." \
    PANKTI_STRINGIFY(PANKTI_VERSION_MINOR) "." \
    PANKTI_STRINGIFY(PANKTI_VERSION_PATCH)


// Get version as hex
#define PANKTI_VERSION_AS_HEX \
    ((PANKTI_VERSION_MAJOR << 16) | (PANKTI_VERSION_MINOR << 8) | PANKTI_VERSION_PATCH)


static inline int PanktiVersionHasGitInfo(void){
	return PANKTI_GIT_COUNT[0] != '\0' && PANKTI_GIT_HASH[0] != '\0';
}

static inline const char * PanktiVersionGetBase(void){
	return PANKTI_VERSION_BASE;
}

static inline const char * PanktiVersionGetGitHash(void){
	return PANKTI_GIT_HASH;
}

static inline int PanktiVersionGetHex(void){
	return PANKTI_VERSION_AS_HEX;
}

static inline int PanktiVersionIsPrerelease(void){
	if (PANKTI_RELEASE_LEVEL[0] != '\0') {
		return 1;
	}else{
		return 0;
	}
}

static inline int PanktiVersionIsDevBuild(void){
	return PanktiVersionIsPrerelease() && PANKTI_RELEASE_LEVEL[0] == 'd';
}


static inline const char * PanktiVersionGetString(void){
	if (PanktiVersionIsPrerelease()) {
		if (PanktiVersionHasGitInfo() && PanktiVersionIsDevBuild()) {
			return  PANKTI_VERSION_BASE "-" PANKTI_RELEASE_LEVEL ".r" PANKTI_GIT_COUNT "+" PANKTI_GIT_HASH;
		} else{
			return PANKTI_VERSION_BASE "-" PANKTI_RELEASE_LEVEL;
		}
	}else{
		return PANKTI_VERSION_BASE;
	}
}

#define PANKTI_VERSION PanktiVersionGetString()



#ifdef __cplusplus
}
#endif
#endif
