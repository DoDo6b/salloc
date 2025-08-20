#include "salloc.h"


size_t bytesMWords (size_t bytes)
{
    return (bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
}

static const size_t wsizeofBHeader = (sizeof (ChunkHeader) + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);


Stack stackInit (size_t size)
{
    Stack buf = {0};
    buf.stack = (uintptr_t*)calloc(bytesMWords (size), sizeof(uintptr_t));
    if (!buf.stack) 
    {
        log_err ("internal error", "can't allocate a new stack (malloc returned a NULL)");
        return buf;
    }
    IF_DBG(buf.frontCanary = STACKFCANARY;)
    buf.wsize = bytesMWords (size);
    buf.top = buf.stack;
    buf.allocated = 0;
    IF_DBG(buf.tailCanary = STACKTCANARY;)

    IF_DBG (log_string ("<grn>initilized arena with %zu word(s)<dft>\n", buf.wsize);)
    return buf;
}

void stackRemove (Stack* stack)
{
    if (stack && stack->stack) free(stack->stack);
    IF_DBG
    (
        else
        {
            log_err ("bad args error", "arena wasn't initialised or received a Null");
            IF_SAFE (exit (EXIT_FAILURE);)
        }
    )
}


void* salloc (Stack* stack, size_t size)
{
    if (!stack || !stack->stack)
    {
        log_err ("bad args error", "arena wasn't initialised or received a Null");
        IF_SAFE (exit (EXIT_FAILURE);)
        return NULL;
    }

    size_t wsize = bytesMWords (size) IF_DBG (+1);
    if (wsize == 0 IF_DBG (+1))
    {
        IF_DBG (
            log_err ("bad args error", "can't allocate 0 bytes");
            IF_SAFE (exit (EXIT_FAILURE);)
        )
        return NULL;
    }
    uintptr_t* ptr = stack->top;

    if (ptr + wsize + wsizeofBHeader > stack->stack + stack->wsize)
    {
        IF_DBG (
            IF_SAFE (log_err ("internal error", "can't allocate memory (object too big)");)
            IF_SAFE (exit (EXIT_FAILURE);)
        )
        return NULL;
    }

    ChunkHeader header;
    IF_DBG 
    (
        *ptr = (uintptr_t)ptr ^ HEXSPEAK;
        header.frontSign = BLOCKCANARY;
        header.ptr = ptr;
        header.taleSign = BLOCKCANARY;
    )
    header.size = wsize;

    memcpy(ptr + wsize, &header, sizeof (ChunkHeader));

    stack->top += header.size + wsizeofBHeader;
    stack->allocated++;

    return (void*)(ptr IF_DBG (+1));
}


void sfree (Stack* stack)
{
    if (!stack || !stack->stack)
    {
        log_err ("bad args error", "arena wasn't initialised or received a Null");
        return;
    }
    if (stack->allocated > 0)
    {
        ChunkHeader* header = (ChunkHeader*)stack->top - 1;

        stack->top -= header->size + wsizeofBHeader;
        stack->allocated--;
    }
}

#ifndef NDEBUG


uint64_t chunkVerify_ (const char* callerFile, unsigned int callerLine, const Stack* stack, const ChunkHeader* header)
{
    uint64_t error_accum = 0;

    if (header == NULL)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> received a NULL</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum |= ERRCODE_NULL;
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> verification failed with code: %llu</b>\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
        return error_accum;
    }
    if (header->frontSign != BLOCKCANARY || header->taleSign != BLOCKCANARY)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> signatures is corrupted</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum |= ERRCODE_CH_SIGN;
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> verification failed with code: %llu</b>\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
        return error_accum;
    }
    if (header->ptr < stack->stack || header->ptr >= stack->top)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> Chunk::ptr doesn’t belong to a stack</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum |= ERRCODE_CH_PTROUTOFBOUNDS;
    }
    if (header->size == 1)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> Chunk::size is 0</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum |= ERRCODE_CH_ZEROSIZE;
    }
    if ((const uintptr_t*) (header + 1) > stack->top)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> Chunk overflowing stack</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum |= ERRCODE_CH_OVERFLOW;
    }
    if (*header->ptr != ((uintptr_t)header->ptr ^ HEXSPEAK))
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> memory sign is corrupted</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        memDump (header->ptr, header->size);
        error_accum |= ERRCODE_CH_BLOCKSIGN;
    }

    if (error_accum != 0)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> verification failed with code: %llu</b>\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
    }
    return error_accum;
}

uint64_t stackVerify_ (const char* callerFile, unsigned int callerLine, const Stack* stack)
{
    uint64_t error_accum = 0;

    if (stack == NULL || stack->stack == NULL)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> received a NULL or stack wasn't initialized</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum |= ERRCODE_NULL;
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> verification failed with code: %llu</b>\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
        return error_accum;
    }
    if (stack->frontCanary != STACKFCANARY || stack->tailCanary != STACKTCANARY)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> stack signatures is corrupted</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum |= ERRCODE_S_SIGN;
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> verification failed with code: %llu</b>\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
        return error_accum;
    }
    else
    {
        if (stack->top > stack->stack + stack->wsize)
        {
            log_string
            (
                "%s:%d: %s: <b><red>verification error:<dft> stack overflow</b>\n",
                callerFile,
                callerLine,
                __func__
            );
            error_accum |= ERRCODE_S_OVERFLOW;
            log_string
            (
                "%s:%d: %s: <b><red>verification error:<dft> verification failed with code: %llu</b>\n",
                callerFile,
                callerLine,
                __func__,
                error_accum
            );
            return error_accum;
        }

        const ChunkHeader* carriage = (const ChunkHeader*)stack->top - 1;
        const ChunkHeader* carriage_lagging = NULL;
        for (size_t i = 0; i < stack->allocated; i++)
        {
            uint64_t chErrCode = chunkVerify(stack, carriage);
            if (chErrCode != 0)
            {
                log_string
                (
                    "%s:%d: %s: <b><red>verification error:<dft> Chunk[%zu] failed verification</b>\n",
                    callerFile,
                    callerLine,
                    __func__,
                    i
                );
                error_accum |= chErrCode;
            }
            else
            {
                if (i >= 1)
                {
                    if ((const uintptr_t*)(carriage + 1) > carriage_lagging->ptr)
                    {
                        log_string
                        (
                            "%s:%d: %s: <b><red>verification error:<dft> Chunks [%zu] and [%zu] overlapping each other</b>\n",
                            callerFile,
                            callerLine,
                            __func__,
                            i-1,
                            i
                        );
                        error_accum |= ERRCODE_S_OVERLAPING;
                    }
                }
            }
            carriage_lagging = carriage;
            carriage = (const ChunkHeader*) ((const uintptr_t*)carriage - carriage->size - wsizeofBHeader);
        }
    }

    if (error_accum != 0)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> verification failed with code: %llu</b>\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
    }
    return error_accum;
}


#endif


void memDump (const void* pointer, size_t words)
{
    const unsigned char* ptr        =   (const unsigned char*)pointer;
    size_t               byteSize   =   words * sizeof(uintptr_t);

    log_string ("  Memory dump of %p(%zu byte(s))\n", pointer, byteSize);
    log_string ("  {\n    ");
    
    log_string ("<blk>");
    for (size_t i = 0; i < byteSize; i++)
    {
        log_string ("%02zX ", i);
    }
    log_string ("<dft>\n    <cyn>");

    for (size_t i=0; i < byteSize; i++)
    {
        log_string ("%02X ", *(ptr+i));
    }
        
    log_string ("<dft>\n  }\n");
}

void chunkDump_ (const char* callerFile, unsigned int callerLine, const Stack* stack, const ChunkHeader* header, bool bytesDump)
{
    #ifndef NDEBUG
    
    if (chunkVerify (stack, header) != 0)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> Chunk failed verification</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        IF_SAFE(exit(EXIT_FAILURE);)
    }

    #else

    if (header == NULL)
    {
        log_string ("%s:%d: %s: <b><red>verification error:<dft> received a NULL</b>\n", callerFile, callerLine, __func__);
        IF_SAFE(exit(EXIT_FAILURE);)
    }

    #endif

    log_string
    (
        "  ptr: %p(%p), size: %zu word(s)\n",
        (const uintptr_t*)header - header->size,
        header,
        header->size
    );

    if (bytesDump)
    {
        memDump ((const uintptr_t*)header - header->size, header->size);
    }
}

void stackDump_ (const char* callerFile, unsigned int callerLine, const char* name, const Stack* stack, ShowMode showMode, bool memDump)
{
    #ifndef NDEBUG

    if (stackVerify (stack) != 0)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> stack failed verification</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        IF_SAFE(exit(EXIT_FAILURE);)
    }

    #else

    if (!stack || !stack->stack)
    {
        log_string
        (
            "%s:%d: %s: <b><red>verification error:<dft> received a NULL or stack wasnt initialized</b>\n",
            callerFile,
            callerLine,
            __func__
        );
        return;
    }

    #endif

    log_string
    (
        "<blu>%s dump:<dft>\n"
        "size: %zu mword(s),\n"
        "real size %zu mword(s),\n"
        "allocated: %zu chunk(s)\n"
        "{\n",
        name,
        stack->wsize,
        stack->top - stack->stack,
        stack->allocated
    );

    if (stack->allocated == 0)
    {
        log_string ("  <i>Empty</i>\n");
        log_string ("}\n");
        return;
    }

    const ChunkHeader* carriage = (const ChunkHeader*)stack->top - 1;
    switch (showMode)
    {
    case HIDE:
        log_string ("  <i>Chunks are hidden</i>\n");
        break;
    case FIRSNLAST:
        for (size_t i = 0; i < stack->allocated; i++)
        {
            if (i < 8 || i > stack->allocated - 8)
            {
                chunkDump (stack, carriage, memDump);
                if (i == 7) log_string ("  ...\n");
            }
            carriage = (const ChunkHeader*) ((const uintptr_t*)carriage - carriage->size - wsizeofBHeader);
        }
        break;
    case ALL:
        for (size_t i = 0; i < stack->allocated; i++)
        {
            chunkDump (stack, carriage, memDump);
            carriage = (const ChunkHeader*) ((const uintptr_t*)carriage - carriage->size - wsizeofBHeader);
        }
        break;
    default:
        log_err ("syntax error", "inappropriate display mode");
        return;
    }

    log_string ("}\n");
}