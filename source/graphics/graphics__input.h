
typedef unsigned int I_Modifiers;
typedef enum
{
    I_Modifiers_Ctrl  = Bit(0),
    I_Modifiers_Shift = Bit(1),
    I_Modifiers_Alt   = Bit(2),
    I_Modifiers_ANY   = ~((I_Modifiers)0),
} KeyModifiers_ENUM;

#define KeyDef(NAME, STRING, IS_REAL) I_Key_ ## NAME ,
typedef enum
{
#include "graphics__keylist.h"
} I_Key;

Function S8 I_S8FromKey (I_Key key);
