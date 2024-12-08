#include "../inc/libmx.h"

int mx_strlen(const char *s) {
    if (s == NULL) {
        return 0;
    }
    int length = 0;
    while (*s != '\0') {
        length++, s++;
    }
    return length;
}


