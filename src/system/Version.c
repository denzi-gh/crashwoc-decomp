#include "stdio.h"
#include "stddef.h"

int strncasecmp(const char* s1, const char* s2, size_t n);

extern char date_string[];
extern char time_string[];
extern char* months[];
extern char ver_string[];

void Version(void) {
    int i;
    char day_tens;
    char yr0, yr1;

    for (i = 0; i <= 11; i++) {
        if (strncasecmp(date_string, months[i], 3) == 0)
            break;
    }

    i++;

    day_tens = '0';
    yr0 = date_string[9];
    yr1 = date_string[10];
    if (date_string[4] != ' ') {
        day_tens = date_string[4];
    }

    sprintf(ver_string, "VERSION: %c%c%02d%c%c.%c%c%c%c",
            yr0, yr1,
            i, day_tens, date_string[5],
            time_string[0], time_string[1],
            time_string[3], time_string[4]);
}
