#include <stdio.h>
#include "../include/color.h"


static int g_ansi_enabled = 1;

void set_ansi_color_enabled(int enabled) {
    g_ansi_enabled = enabled;
}

void print_color(const char* code) {
    if (g_ansi_enabled)
        printf("%s", code);
}
