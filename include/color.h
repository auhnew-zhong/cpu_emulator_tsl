#ifndef COLOR_H
#define COLOR_H


static const char* ANSI_RED     = "\x1b[31m";
static const char* ANSI_GREEN   = "\x1b[32m";
static const char* ANSI_YELLOW  = "\x1b[33m";
static const char* ANSI_BLUE    = "\x1b[34m";
static const char* ANSI_MAGENTA = "\x1b[35m";
static const char* ANSI_CYAN    = "\x1b[36m";
static const char* ANSI_WHITE   = "\x1b[37m";
static const char* ANSI_BOLD    = "\x1b[1m";
static const char* ANSI_RESET   = "\x1b[0m";
static const char* ANSI_BOLD_RED     = "\x1b[1;31m";
static const char* ANSI_BOLD_GREEN   = "\x1b[1;32m";
static const char* ANSI_BOLD_YELLOW  = "\x1b[1;33m";
static const char* ANSI_BOLD_BLUE    = "\x1b[1;34m";
static const char* ANSI_BOLD_MAGENTA = "\x1b[1;35m";
static const char* ANSI_BOLD_CYAN    = "\x1b[1;36m";
static const char* ANSI_BOLD_WHITE   = "\x1b[1;37m";

void set_ansi_color_enabled(int enabled);
void print_color(const char* code);

#endif
