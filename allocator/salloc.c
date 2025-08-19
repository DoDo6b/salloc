#include "salloc.h"


size_t bytesMwsize (size_t bytes)
{
    return (bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
}

static const size_t wsizeofBHeader = (sizeof (BlockHeader) + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);


Stack stackInit (size_t size)
{
    Stack buf = {0};
    buf.stack = (uintptr_t*) malloc (size + sizeof(uintptr_t) - 1);
    if (!buf.stack) 
    {
        log_err ("internal error", "can't allocate a new stack (malloc returned a NULL)");
        return buf;
    }
    buf.wsize = bytesMwsize (size);
    buf.top = buf.stack;
    buf.allocated = 0;
    return buf;
}


void* salloc (Stack* stack, size_t size)
{
    if (!stack || !stack->stack)
    {
        log_err ("call error", "arena wasn't initialised or received a Null");
        return NULL;
    }

    size_t wsize = bytesMwsize (size);
    uintptr_t* ptr = stack->top;

    if (ptr + wsize + wsizeofBHeader > stack->stack + stack->wsize)
    {
        log_err ("internal error", "can't allocate memory (object too big)");
        return NULL;
    }

    BlockHeader header;
    IF_DBG (header.frontSign = (uintptr_t)ptr ^ HEXSPEAK;)
    header.size = wsize;
    IF_DBG (header.taleSign = (uintptr_t)wsize ^ HEXSPEAK;)

    memcpy(ptr + wsize, &header, sizeof (BlockHeader));

    stack->top += header.size + wsizeofBHeader;
    stack->allocated++;

    return (void*)ptr;
}


void sfree (Stack* stack)
{
    if (!stack || !stack->stack)
    {
        log_err ("call error", "arena wasn't initialised or received a Null");
        return;
    }
    if (stack->allocated > 0)
    {
        BlockHeader* header = (BlockHeader*)stack->top - 1;

        stack->top -= header->size + wsizeofBHeader;
        stack->allocated--;
    }
}