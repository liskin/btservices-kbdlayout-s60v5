/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Declares main application class.
 *
*/


#ifndef __MODIFIER_H
#define __MODIFIER_H

#include <e32std.h>

// ----------------------------------------------------------------------

/*!
 Convenience functions for working with HID (boot protocol style)
 keyboard modifier flags.  These are stored as an eight bit vector as
 follows:
 
 <pre>
 +-------------------------------------------------------+
 |           Right           |           Left            |
 +-------------------------------------------------------+
 | Func |  Alt | Shft | Ctrl | Func |  Alt | Shft | Ctrl |
 +-------------------------------------------------------+
 bit:  7      6      5      4      3      2      1      0
 </pre>
 
 We also make use of a "folded" representation, where there
 is no distinction between the left and right versions of
 the modifier keys:
 
 <pre>
 +---------------------------+---------------------------+
 |   0  |   0  |   0  |   0  | Func |  Alt | Shft | Ctrl |
 +---------------------------+---------------------------+
 bit:  7      6      5      4      3      2      1      0
 </pre>
 
 i.e. folded = left OR right.
 */
class THidModifier
    {
public:
    THidModifier(TUint8 aFlags);

    inline void Set(TUint8 aValue);
    inline void Merge(TUint8 aValue);
    inline void Clear(TUint8 aValue);

    inline TUint8 Value() const;

    inline TBool None() const;
    inline TBool Shift() const;
    inline TBool Control() const;
    inline TBool Alt() const;
    inline TBool Func() const;

    inline TBool LeftControl() const;
    inline TBool LeftShift() const;
    inline TBool LeftAlt() const;
    inline TBool LeftFunc() const;

    inline TBool RightControl() const;
    inline TBool RightShift() const;
    inline TBool RightAlt() const;
    inline TBool RightFunc() const;

    inline TBool ControlAlt() const;

    inline TBool ShiftOnly() const;
    inline TBool ControlOnly() const;
    inline TBool AltOnly() const;
    inline TBool FuncOnly() const;

    inline TUint8 Fold() const;

    inline void InvertShift();

public:
    enum TModifiers
        {
        ELeftControl = (1 << 0),
        ELeftShift = (1 << 1),
        ELeftAlt = (1 << 2),
        ELeftFunc = (1 << 3),
        ERightControl = (1 << 4),
        ERightShift = (1 << 5),
        ERightAlt = (1 << 6),
        ERightFunc = (1 << 7)
        };

    enum TFoldedModifiers
        {
        EUnmodified = 0, // ----
        ECtrl = 1, // ---C
        EShift = 2, // --S-
        ECtrlShift = 3, // --SC
        EAlt = 4, // -A--
        EAltCtrl = 5, // -A-C
        EAltShift = 6, // -AS-
        EAltCtrlShift = 7, // -ASC
        EFunc = 8, // F---
        EFuncCtrl = 9, // F--C
        EFuncShift = 10, // F-S-
        EFuncCtrlShift = 11, // F-SC
        EFuncAlt = 12, // FA--
        EFuncAltCtrl = 13, // FA-C
        EFuncAltShift = 14, // FAS-
        EFuncAltCtrlShift = 15
        // FASC
        };

private:
    TUint8 iFlags;
    };

// ----------------------------------------------------------------------

inline THidModifier::THidModifier(TUint8 aFlags) :
    iFlags(aFlags)
    {
    // nothing else to do
    }

inline TUint8 THidModifier::Value() const
    {
    return iFlags;
    }

inline void THidModifier::Set(TUint8 aValue)
    {
    iFlags = aValue;
    }

inline void THidModifier::Merge(TUint8 aValue)
    {
    iFlags |= aValue;
    }

inline void THidModifier::Clear(TUint8 aValue)
    {
    iFlags &= ~aValue;
    }

// ----------------------------------------------------------------------
// Individual keys:

inline TBool THidModifier::LeftControl() const
    {
    return iFlags & ELeftControl;
    }

inline TBool THidModifier::LeftShift() const
    {
    return iFlags & ELeftShift;
    }

inline TBool THidModifier::LeftAlt() const
    {
    return iFlags & ELeftAlt;
    }

inline TBool THidModifier::LeftFunc() const
    {
    return iFlags & ELeftFunc;
    }

inline TBool THidModifier::RightControl() const
    {
    return iFlags & ERightControl;
    }

inline TBool THidModifier::RightShift() const
    {
    return iFlags & ERightShift;
    }

inline TBool THidModifier::RightAlt() const
    {
    return iFlags & ERightAlt;
    }

inline TBool THidModifier::RightFunc() const
    {
    return iFlags & ERightFunc;
    }

// ----------------------------------------------------------------------
// Modifier states:

inline TBool THidModifier::None() const
    {
    return iFlags == 0;
    }

inline TBool THidModifier::Shift() const
    {
    return LeftShift() || RightShift();
    }

inline TBool THidModifier::Control() const
    {
    return LeftControl() || RightControl();
    }

inline TBool THidModifier::Alt() const
    {
    return LeftAlt() || RightAlt();
    }

inline TBool THidModifier::Func() const
    {
    return LeftFunc() || RightFunc();
    }

inline TBool THidModifier::ControlAlt() const
    {
    return Alt() && Control();
    }

// ----------------------------------------------------------------------

inline TUint8 THidModifier::Fold() const
    {
    TUint left = iFlags & 0x0f;
    TUint right = (iFlags & 0xf0) >> 4;
    return static_cast<TUint8> (left | right);
    }

// ----------------------------------------------------------------------

inline TBool THidModifier::ShiftOnly() const
    {
    return Fold() == EShift;
    }

inline TBool THidModifier::ControlOnly() const
    {
    return Fold() == ECtrl;
    }

inline TBool THidModifier::AltOnly() const
    {
    return Fold() == EAlt;
    }

inline TBool THidModifier::FuncOnly() const
    {
    return Fold() == EFunc;
    }

// ----------------------------------------------------------------------

inline void THidModifier::InvertShift()
    {
    if (iFlags & (ELeftShift | ERightShift))
        {
        iFlags &= ~(ELeftShift | ERightShift);
        }
    else
        {
        iFlags |= ELeftShift;
        }
    }

// ----------------------------------------------------------------------

#endif
