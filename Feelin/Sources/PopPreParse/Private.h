/*** Includes **************************************************************/

#include <proto/feelin.h>

#include <libraries/feelin.h>

#include "_locale/enums.h"

/****************************************************************************
**** IDs ********************************************************************
****************************************************************************/

enum    {                                       // attributes

        FA_PopPreParse_Contents

        };

/****************************************************************************
**** Object *****************************************************************
****************************************************************************/

struct LocalObjectData
{
    FAreaPublic                    *AreaPublic;
    FObject                         String;
    FObject                         Dialog;
    FObject                         Adjust;
};

