/*
    Copyright � 2002, The AROS Development Team. 
    All rights reserved.
    
    $Id$
*/

#include <exec/types.h>
#include <proto/exec.h>

#include "socket_intern.h"

#define MYDEBUG
#include "debug.h"

/*****************************************************************************

    NAME */
#ifndef __AROS
__asm int LIB_ReleaseCopyOfSocket(register __d0 long s, register __d1 long id)
#else
	AROS_LH2(int, LIB_ReleaseCopyOfSocket,

/*  SYNOPSIS */
	AROS_LHA(long, s, D0),
	AROS_LHA(long, id, D1),

/*  LOCATION */
	struct Library *, SocketBase, 156, Socket)
#endif

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS
	The function itself is a bug ;-) Remove it!

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,SocketBase)

    D(bug("ReleaseCopyOfSocket()\n"));

    return -1;

    AROS_LIBFUNC_EXIT

}
