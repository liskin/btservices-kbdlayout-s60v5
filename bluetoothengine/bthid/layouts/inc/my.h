#ifndef MY_H
#define MY_H

// INCLUDES
#include <e32std.h>
#include <e32svr.h>

#include "layout.h"
#include "switchlay.h"
#include "unitedstates.h"
#include "czech.h"

class CMyLayout : public CSwitchKeyboardLayout<CUnitedStatesLayout, CCzechLayout, EUnitedKingdom>
{
public:
    static CKeyboardLayout* NewL();
private:
    CMyLayout() {};
};

#endif  //MY_H
