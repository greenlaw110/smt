#ifndef SMT_DEFS_H
#define SMT_DEFS_H

typedef unsigned char UINT8;

typedef char boolean;

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef TRUE
#define TRUE ((boolean)1)
#endif

#ifndef FALSE
#define FALSE ((boolean)0)
#endif

#ifndef FOREVER
#define FOREVER for (;;)
#endif

#ifndef byte
#define byte char
#endif

#ifndef MAX
#define MAX(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })
#endif

#endif /* SMT_DEFS_H */
