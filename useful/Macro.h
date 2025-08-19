#ifndef MACRO_H
#define MACRO_H


// Dbg mode
#ifndef NDEBUG
    #define IF_DBG(...) __VA_ARGS__
#else
    #define IF_DBG(...)
#endif

// Safe/Unsafe mode
#ifndef UNSAFE
    #define IF_SAFE(...) __VA_ARGS__
#else
    #define IF_SAFE(...)
#endif

// turn something off
#define OFF if(0)


#endif