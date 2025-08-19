#include <stdlib.h>
#include <stdio.h>
#include "logger/logger.h"
#include "allocator/salloc.h"

static unsigned int randFromTo (unsigned int min, unsigned int max)
{
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
    log_string ("<blu>Normal work test<dft>\n");

    Stack arena = stackInit (randFromTo(1024, 65536));
    log_string ("<grn>initilized arena with %zu word(s)<dft>\n", arena.wsize);
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

    log_string ("<grn>TEST PASSED<dft>\n");
}


int main (int argc, char** argv)
{
    const char* fname = (argc == 2) ? argv[1] : "stdout";
    log_start (fname);

    normalWork();

    
    return 0;
}