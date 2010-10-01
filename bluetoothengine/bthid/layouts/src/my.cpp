#include "my.h"
#include "hiddebug.h"

CKeyboardLayout* CMyLayout::NewL()
    {
    DBG(RDebug::Print(_L("[HID]\tCMyLayout::NewL()")));
    CKeyboardLayout* layout = new (ELeave) CMyLayout;
    return layout;
    }
