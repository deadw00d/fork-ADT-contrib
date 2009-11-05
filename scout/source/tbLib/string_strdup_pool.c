#include <exec/types.h>
#include <proto/exec.h>
#include <string.h>
#include "tblib.h"
#include "SDI_compiler.h"

/* /// "_strdup_pool()" */
STRPTR _strdup_pool( STRPTR str,
                     APTR pool )
{
    STRPTR tmp;

    tmp = tbAllocVecPooled(pool, strlen(str) + 1);
    if (tmp) strcpy(tmp, str);

    return tmp;
}
/* \\\ */
