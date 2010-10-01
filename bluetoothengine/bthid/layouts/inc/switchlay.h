/* vim:set et: */

#ifndef __SWITCHLAY_H
#define __SWITCHLAY_H

#include <e32std.h>
#include <e32svr.h>

#include "layout.h"
#include "decode.h"
#include "modifier.h"
#include "hidlayoutids.h"
#include "hidvalues.h"

/*
 * A combined layout which provides Alt+Shift switching.
 */
template <class Lay1, class Lay2, TInt LayId>
class CSwitchKeyboardLayout : public CKeyboardLayout {
private:
    const CKeyboardLayout *lay1, *lay2;
    const CKeyboardLayout **cur;

public:
    CSwitchKeyboardLayout() : lay1(Lay1::NewL()), lay2(Lay2::NewL()),
        cur(new (const CKeyboardLayout *)(lay1)) {
    }

    ~CSwitchKeyboardLayout() {
        delete lay1;
        delete lay2;
        delete cur;
    }

    virtual TUint16 TranslateKey(TInt aHidKey, TInt aUsagePage,
            THidModifier aModifiers, TLockKeys aLockKeys) const {
        // holding left alt (and not ctrl => not alt-gr) and pressing left
        // shift switches the layout
        if (!aModifiers.Control() && aModifiers.LeftAlt() && aHidKey == 0xE1
            && aUsagePage == EUsagePageKeyboard) {
            if (*cur == lay1)
                *cur = lay2;
            else
                *cur = lay1;
        }

        return (*cur)->TranslateKey(aHidKey, aUsagePage, aModifiers, aLockKeys);
    }

    virtual TUint16 Unicode(TInt aHidKey, THidModifier aModifiers) const {
        return (*cur)->Unicode(aHidKey, aModifiers);
    }

    virtual TBool IsRepeatingKey(TInt aUnicodeKey) const {
        return (*cur)->IsRepeatingKey(aUnicodeKey);
    }

    virtual TBool IsDeadKey(TInt aUnicodeKey) const {
        return (*cur)->IsDeadKey(aUnicodeKey);
    }

    virtual TUint16 FindCombiningChar(TUint16 aDeadKeyUnicode,
            TUint16 aUnicodeKey) const {
        return (*cur)->FindCombiningChar(aDeadKeyUnicode, aUnicodeKey);
    }

    virtual TBool ChangesWithNumLock(TInt aHidKey) const {
        return (*cur)->ChangesWithNumLock(aHidKey);
    }

    virtual TBool
            ChangesWithCapsLock(TInt aHidKey, THidModifier aModifiers) const {
        return (*cur)->ChangesWithCapsLock(aHidKey, aModifiers);
    }

    virtual TBool AltGrIsControlAlt() const {
        return (*cur)->AltGrIsControlAlt();
    }

    virtual TInt LayoutId() const {
        return LayId;
    }
};

#endif
