#ifndef SALLOC_H
#define SALLOC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "../logger/logger.h"

#include "../useful/Macro.h"
#include "errCodes.h"


#define HEXSPEAK    0XDEADDEDF
#define BLOCKCANARY 0XB10CDEAD

#define STACKFCANARY 0XDEADBEEF
#define STACKTCANARY 0XA10CDEAD


size_t bytesMWords (size_t bytes);

typedef struct
{
    IF_DBG (uintptr_t frontCanary;)
    uintptr_t*  top;
    uintptr_t*  stack;
    size_t      wsize;
    size_t      allocated;
    IF_DBG (uintptr_t tailCanary;)
}Stack;

Stack stackInit (size_t size);
void  stackRemove (Stack* stack);

typedef struct
{
    IF_DBG (uintptr_t frontSign;)
    IF_DBG (uintptr_t* ptr;)
    size_t size;
    IF_DBG (uintptr_t taleSign);
}ChunkHeader;


void* salloc (Stack* stack, size_t size);
void  sfree  (Stack* stack);


void memDump    (const void* pointer, size_t words);

void chunkDump_ (const char* callerFile, unsigned int callerLine, const Stack* stack, const ChunkHeader* header, bool bytesDump);

typedef enum{
    HIDE,
    FIRSNLAST,
    ALL,
}ShowMode;
void stackDump_ (const char* callerFile, unsigned int callerLine, const char* name, const Stack* stack, ShowMode showMode, bool memDump);

#define chunkDump(...) chunkDump_ (__FILE__, __LINE__, __VA_ARGS__)
#define stackDump(stack, ...) stackDump_ (__FILE__, __LINE__, #stack, stack, __VA_ARGS__)

uint64_t chunkVerify_ (const char* callerFile, unsigned int callerLine, const Stack* stack, const ChunkHeader* header);
uint64_t stackVerify_ (const char* callerFile, unsigned int callerLine, const Stack* stack);
#define chunkVerify(...) chunkVerify_ (__FILE__, __LINE__, __VA_ARGS__)
#define stackVerify(...) stackVerify_ (__FILE__, __LINE__, __VA_ARGS__)


#endif