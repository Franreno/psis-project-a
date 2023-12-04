#include "logger.h"

static FILE *log_file = NULL;

void log_init(const char *filename)
{
    log_file = fopen(filename, "a"); // "a" to append to the file if it exists
    if (log_file == NULL)
    {
        // Handle error, you might want to print to stderr or take other actions
        perror("Error opening log file");
    }
}

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

void log_close()
{
    if (log_file != NULL)
    {
        fclose(log_file);
        log_file = NULL;
    }
}
