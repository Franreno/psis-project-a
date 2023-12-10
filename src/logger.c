#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

static FILE *log_file = NULL;

/**
 * @brief - Initialize the logger
 *
 * @param filename - Name of the log file
 */
void log_init(const char *filename)
{
    log_file = fopen(filename, "a");
    if (log_file == NULL)
    {
        perror("Error opening log file");
    }
}

/**
 * @brief - Write to the log file
 *
 * @param format - Format of the string to write
 * @param ... - Arguments to the format string
 */
void log_write(const char *format, ...)
{
    if (log_file != NULL)
    {
        va_list args;
        va_start(args, format);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        va_end(args);

        fflush(log_file);
    }
}

/**
 * @brief - Close the log file
 */
void log_close()
{
    if (log_file != NULL)
    {
        fclose(log_file);
        log_file = NULL;
    }
}
