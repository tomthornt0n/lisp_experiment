
static S8
I_S8FromKey(I_Key key)
{
    S8 result;
    
    static S8 strings[I_Key_MAX + 1] =
    {
#define KeyDef(NAME, STRING, IS_REAL) S8Initialiser(STRING),
#include "graphics__keylist.h"
    };
    
    if(I_Key_None <= key && key <= I_Key_MAX)
    {
        result = strings[key];
    }
    else
    {
        result = S8("ERROR - invalid key");
    }
    
    return result;
}
