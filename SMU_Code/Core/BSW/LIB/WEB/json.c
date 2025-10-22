/*
 * json.c
 *
 *  Created on: Feb 7, 2025
 *      Author: 98912
 */
#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Platform.h"

// Function to extract a double value associated with a key
double extract_value(const char *json, const char *key) {

    const char *pos = strstr(json, key);
    if (pos == NULL) {
        return INVALID_DATA; // Key not found
    }

    pos += strlen(key); // Move past the key
    float value;
      sscanf(pos+1,"%f",&value);

	    return value; // Convert the following number to an integer
}
// Function to extract a double value associated with a key
double extract_value2(const char *json, const char *key) {
    char search_key[64];
    snprintf(search_key, sizeof(search_key), "[\"%s\",", key);  // Construct exact match pattern like [Vbat,

    const char *pos = strstr(json, search_key);
    //const char *pos = strstr(json, key);
    if (pos == NULL) {
        return INVALID_DATA; // Key not found
    }

    pos += strlen(search_key); // Move past the key
    float value;
      sscanf(pos+1,"%f",&value);

	    return value; // Convert the following number to an integer
}

// Function to extract an integer value associated with a key
int extract_int_value(const char *json, const char *key) {

	    const char *pos = strstr(json, key);
	    if (pos == NULL) {
	        return -1; // Key not found
	    }

	    pos += strlen(key); // Move past the key
	    int value=0;
        sscanf(pos,":%d",&value);

	    return value; // Convert the following number to an integer

}
