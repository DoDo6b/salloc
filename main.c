#include <stdlib.h>
#include <stdio.h>
#include "logger/logger.h"
#include "allocator/salloc.h"

#include <time.h>


static const size_t Service_info = sizeof (ChunkHeader) IF_DBG (+ sizeof (uintptr_t));

static unsigned int randFromTo (unsigned int min, unsigned int max)
{
    srand (time (NULL));
    return rand() % (max-min) + min;
}

static bool cleaning (Stack* stack)
{
    log_string ("<ylw>Cleaning in process<dft>\n");
    while (stack->allocated > 0)
    {
        sfree(stack);
    }
    stackDump (stack, ALL, true);
    if (stack->allocated == 0)
    {
        log_string ("<grn>Cleaning is done<dft>\n");
        return 0;
    }
    else
    {
        log_string ("<red>something went wrong<dft>\n");
        return 1;
    }
}

static void normalWork ()
{
    log_string ("<blu>Normal work test<dft>\n\n");

    Stack arena = stackInit (randFromTo(1024, 65536));
    stackDump (&arena, ALL, true);

    unsigned int cycles = randFromTo (1, 1024);
    log_string ("<ylw>will do %u cycle(s)<dft>\n", cycles); 
    for (unsigned int i = 0; i < cycles; i++)
    {
        unsigned int random = randFromTo (1, (unsigned int)arena.wsize / 10);
        while ((unsigned int)arena.allocated < random && salloc (&arena, randFromTo(1, 64) * sizeof (uintptr_t) ));
        random = randFromTo (0, (unsigned int)arena.allocated);
        while ((unsigned int)arena.allocated > random) sfree (&arena);
    }
    log_string ("<grn>work is done<dft>\n");
    
    cleaning (&arena);
    stackRemove (&arena);

    log_string ("\n<grn>TEST PASSED<dft>\n\n");
}

static void stupidInput ()
{
    log_string ("<blu>Stupid input test<dft>\n\n");

    Stack arena = stackInit (1024);
    stackDump (&arena, ALL, true);

    log_string ("<ylw>allocating 0 bytes<dft>\n");
    log_string ("returned: %p\n", salloc(&arena, 0));
    stackDump (&arena, ALL, true);
    
    log_string ("<ylw>allocating 1024+1 byte<dft>\n");
    log_string ("returned: %p\n", salloc(&arena, 1025));
    stackDump (&arena, ALL, true);

    log_string ("<ylw>allocating in NULL<dft>\n");
    log_string ("returned: %p\n", salloc(NULL, 5));
    stackDump (&arena, ALL, true);

    log_string ("<ylw>allocating 1024 - <i>service info</i> bytes<dft>\n");
    log_string ("returned: %p\n", salloc(&arena, 1024 - Service_info) );
    stackDump (&arena, ALL, false);

    cleaning (&arena);
    stackRemove (&arena);

    log_string ("\n<grn>TEST PASSED<dft>\n\n");
}

#ifndef NDEBUG

static void chunkOverflow ()
{
    log_string ("<blu>Chunk overflow test<dft>\n\n");

    Stack arena = stackInit (100);
    
    log_string ("<ylw>allocating 8 bytes, overflowing with 13 chars<dft>\n");
    char* ptr = (char*) salloc (&arena, 8);
    for (int i = 0; i < 13; i++) ptr[i] = 9;
    memDump (ptr, 5);

    log_string ("<ylw>running dump with verifier<dft>\n");
    stackDump (&arena, ALL, true);

    cleaning (&arena);
    stackRemove (&arena);

    log_string ("\n<grn>TEST PASSED<dft>\n\n");
}


#endif

int main (int argc, char** argv)
{
    const char* fname = (argc == 2) ? argv[1] : "stdout";
    log_start (fname);

    normalWork();
    stupidInput();
    IF_DBG (chunkOverflow();)
    
    return 0;
}