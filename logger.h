#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

void log_init(const char *filename);
void log_write(const char *format, ...);
void log_close();

#endif // LOG_H
