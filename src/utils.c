#include "include/utils.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <uchar.h>


char * ReadFile(const char *path){
	char * text = NULL;

	if (path != NULL) {
		FILE * file = fopen(path, "rt");
		if (file != NULL) {
			fseek(file, 0, SEEK_END);
			long size = ftell(file);
			fseek(file, 0, SEEK_SET);

			if (size > 0) {
				text = (char*)calloc(size + 1, sizeof(char));
				if (text == NULL) {
					return NULL;
				}

				fread(text,size,1,file);
				fclose(file);
				text[size] = '\0';
			}
		
		}
	}


	return text;
}


char * SubString(const char * str, int start, int end){
	if (str == NULL) {
		return NULL;
	}

	size_t len = strlen(str);
	if (start > end || end > len) {
		return NULL;
	}

	size_t sublen = end - start;
	char * result = malloc((sublen + 1) * sizeof(char));
	if (result == NULL) {
		return NULL;
	}
	memcpy(result, str + start, sublen);
	result[sublen] = '\0';
	return result;


}

char32_t U8ToU32(const unsigned char * str){
	return ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
}
