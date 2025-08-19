#include <stdlib.h>
#include <stdio.h>
#include "logger/logger.h"
#include "allocator/salloc.h"


int main ()
{
    log_start ("log.html");
    Stack arena = stackInit (65536);
    log_string ("%p %p %zu %zu\n", arena.stack, arena.top, arena.wsize, arena.allocated);

    while (arena.allocated < arena.wsize && salloc (&arena, sizeof (uintptr_t*)));
    log_string ("%p %p %zu %zu\n", arena.stack, arena.top, arena.wsize, arena.allocated);

    while (arena.allocated > 0) sfree (&arena);
    log_string ("%p %p %zu %zu\n", arena.stack, arena.top, arena.wsize, arena.allocated);
    return 0;
}