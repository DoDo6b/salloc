#ifndef SALLOC_H
#define SALLOC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "../logger/logger.h"

#include "../useful/Macro.h"

#define HEXSPEAK    0XDEADDEDF

typedef struct
{
    uintptr_t*  top;
    uintptr_t*  stack;
    size_t      wsize;
    size_t      allocated;
}Stack;

Stack stackInit (size_t size);

typedef struct
{
    IF_DBG (uintptr_t frontSign);
    size_t size;
    IF_DBG (uintptr_t taleSign);
}BlockHeader;

size_t bytesMWords (size_t bytes);


void* salloc (Stack* stack, size_t size);
void  sfree  (Stack* stack);


#endif