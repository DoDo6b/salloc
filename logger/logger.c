#include "logger.h"


static char LogFile[NAME_MAX] = "";
static FILE* LogStream = NULL;


FILE* log_start (const char* fname)
{
    if (!fname) print_stderr ("syntax error", "can't open stream");
    FILE* buffer = NULL;
    if      (!strncmp(fname, "stdout", NAME_MAX))   LogStream = stdout;
    else if (!strncmp(fname, "stderr", NAME_MAX))   LogStream = stderr;
    else
    {
        buffer = fopen (fname, "a");
        if (!buffer)
        {
            print_stderr("internal error", "fopen returned a NULL");
            return NULL;
        }
        LogStream = buffer;

        atexit (log_close);

        fprintf (LogStream, "<pre>\n");
    }

    strncpy (LogFile, fname, NAME_MAX);

    return LogStream;
}


static unsigned long djb2Hash (const char* hashable, size_t size)
{
    if (!hashable)
    {
        print_stderr ("internal error", "can't get hash(NULL received)\n");
        return 0;
    }
    unsigned long hash = 5381;
    
    for (; *hashable && size > 0; hashable++, size--)
    {
        hash = (hash << 5) + hash + *hashable;
    }

    return hash;
}


enum SPECS_
{
    RESET        = 177655,      // <R>
    BOLD         = 177671,      // <b>
    BOLD_CLOSE   = 5861558,     // </b>
    ITALIC       = 177678,      // <i>
    ITALIC_CLOSE = 5861565,     // </i>

    BLACK        = 193487390,   // <blk>
    RED          = 193504576,   // <red>
    GREEN        = 193493036,   // <grn>
    YELLOW       = 193512449,   // <ylw>
    BLUE         = 193487400,   // <blu>
    MAGENTA      = 193499207,   // <mgn>
    CYAN         = 193488911,   // <cyn>
    WHITE        = 193510136,   // <wht>
    DEFAULT      = 193489379,   // <dft>
};

#define CASE_SPEC_(spec) case spec: fprintf(LogStream, ( (LogStream == stdout || LogStream == stderr) ? ESC_ ## spec : HTML_ ## spec) ); break

static void print_spec (const char* spec, size_t len)
{
    assert (spec);
    switch (djb2Hash (spec, len))
    {
        CASE_SPEC_(RESET);
        CASE_SPEC_(BOLD);
        CASE_SPEC_(BOLD_CLOSE);
        CASE_SPEC_(ITALIC);
        CASE_SPEC_(ITALIC_CLOSE);
        CASE_SPEC_(BLACK);
        CASE_SPEC_(RED);
        CASE_SPEC_(GREEN);
        CASE_SPEC_(YELLOW);
        CASE_SPEC_(BLUE);
        CASE_SPEC_(CYAN);
        CASE_SPEC_(MAGENTA);
        CASE_SPEC_(WHITE);
        CASE_SPEC_(DEFAULT);

        default:    fprintf(LogStream, "%c%s%c", SPEC_OPEN, spec, SPEC_CLOSE);
    }
}

#undef CASE_SPEC_

static size_t parse_spec (const char* substr)
{
    assert (substr && *substr == SPEC_OPEN);
    size_t collected = 0;

    substr++;

    for (collected = 0; substr[collected] != '\0' && substr[collected] != SPEC_CLOSE; collected++);
    print_spec (substr, collected);

    return collected + 2;
}

int log_string (const char* format, ...)
{
    if (!LogStream)
    {
        print_stderr("init error", "LogStream wasn't open, call log_start()");
        return -1;
    }
    if (!format) return 0;

    va_list args;
    va_start (args, format);

    char Buffer[BUFSIZ] = "";
    vsprintf (Buffer, format, args);

    va_end (args);

    size_t printed = 0;
    for (  printed = 0; Buffer[printed] != '\0' && printed < sizeof(Buffer); printed++)
    {
        if (Buffer[printed] == SPEC_OPEN)   printed += parse_spec(&Buffer[printed]) - 1;
        else                                           fprintf(LogStream, "%c", Buffer[printed]);
    }

    if (LogStream != stderr || LogStream != stdout) fflush (LogStream);
    return (int)printed;
}


void log_close()
{
    fprintf (LogStream, "\n</pre>\n");
    if (LogStream && (LogStream != stdout || LogStream != stderr) ) fclose (LogStream);
}

const char* get_log()
{
    return LogFile;
}