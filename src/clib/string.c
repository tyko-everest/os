#include "string.h"

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *dest_ptr = (uint8_t *) dest;
    uint8_t *src_ptr = (uint8_t *) src;
    if (dest < src) {
        for (size_t i = 0; i < n; i++) {
            dest_ptr[i] = src_ptr[i];
        }
    } else if (dest > src) {
        for (size_t i = n; i > 0; i--) {
            dest_ptr[i - 1] = src_ptr[i - 1];
        }
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *dest_ptr = (uint8_t *) dest;
    uint8_t *src_ptr = (uint8_t *) src;
    if (dest < src) {
        for (size_t i = 0; i < n; i++) {
            dest_ptr[i] = src_ptr[i];
        }
    } else if (dest > src) {
        for (size_t i = n; i > 0; i--) {
            dest_ptr[i - 1] = src_ptr[i - 1];
        }
    }
    return dest;
}

void *memset(void *str, int c, size_t n) {
    uint8_t *str_ptr = (uint8_t *) str;
    for (size_t i = 0; i < n; i++) {
        str_ptr[i] = (uint8_t) c;
    }
    return str;
}

int memcmp(const void *str1, const void *str2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (((uint8_t *) str1)[i] < ((uint8_t *) str2)[i]) {
            return -1;
        } else if (((uint8_t *) str1)[i] > ((uint8_t *) str2)[i]) {
            return 1;
        }
    }
    return 0;
}

void *memchr(const void *ptr, int val, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (*((unsigned char *) ptr + i) == (unsigned char) val) {
            return ((char *) ptr) + i;
        }
    }
    return NULL;
}

size_t strlen(const char *str) {
    size_t i = 0;
    while (str[i] != NULL) {
        i++;
    }
    return i;
}

char *strchr(const char *str, int c) {
    while (*str != NULL) {
        if (*str == c) {
            return str;
        }
        str++;
    }
    return NULL;
}

char *strrchr(const char *str, int c) {
    for (int i = strlen(str); i >= 0; i--) {
        if (str[i] == c) {
            return str + i;
        }
    }
    return NULL;
}

// NOTE: currently only supports 1 delimiter, first char in given string!
char *strtok(char *str, const char *delim) {
    static char *token;

    // set token to point to the new given string
    if (str != NULL) {
        token = str;
        // skip any delim chars at the begininng
        while (*token == *delim) {
            *token = NULL;
            token++;
        }
    }
    char *ret = token;
    // if (*ret == 0) {
    //     ret = NULL;
    // }

    // get first delim character
    token = strchr(token, *delim);
    // see if there are any more
    while (token != NULL && *token == *delim) {
        *token = NULL;
        token++;
    }

    return ret;
}

int strcmp (const char* str1, const char* str2) {
    while (1) {
        if (*str1 < *str2) {
            return -1;
        } else if (*str1 > *str2) {
            return 1;
        } else {
            return 0;
        }
        str1++;
        str2++;
    }
}

size_t strcspn(const char *str1, const char *str2) {
    size_t index = 0;
    while (*str1 != 0) {
        const char *key = str2;
        while (*key != 0) {
            if (*key == *str1) {
                return index;
            } else {
                key++; 
            }
        }
        str1++;
        index++;
    }
    return index;
}
