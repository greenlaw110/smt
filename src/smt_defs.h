#ifndef SMT_DEFS_H
#define SMT_DEFS_H

typedef unsigned char UINT8;

typedef char boolean;

typedef UINT8 appId_t;

#ifndef NULL
#define NULL	((void*)0)
#endif

#ifndef TRUE
#define TRUE	((boolean)1)
#endif

#ifndef FALSE
#define FALSE	((boolean)0)
#endif

#ifndef FOREVER
#define FOREVER for(;;)
#endif

#ifndef byte
#define byte char
#endif

#endif /* SMT_DEFS_H */
