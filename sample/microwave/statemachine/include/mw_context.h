#ifndef MW_CONTEXT
#define MW_CONTEXT

#include "smt_defs.h"

typedef struct mw_context {
    char * statePath;
    boolean lampOn;
    boolean heaterOn;
} mw_context_t;

extern void mw_debugContext(mw_context_t * context);

#endif /* define MW_CONTEXT */