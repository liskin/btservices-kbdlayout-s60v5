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


#ifndef __LAYOUTLIBRARY_H
#define __LAYOUTLIBRARY_H

#include <e32std.h>

// ----------------------------------------------------------------------

class CKeyboardLayout;

class CLayoutLibrary : public CBase
    {
public:
    static CLayoutLibrary* NewL();

    virtual ~CLayoutLibrary();

    TInt SetLayout(TInt aLayoutId);

    inline TInt Id() const;
    inline TBool Loaded() const;
    inline CKeyboardLayout* Layout();

    void Close();

private:
    CLayoutLibrary();

private:
    // data

    static const TUid KLayoutUid2;
    TInt iId;
    TBool iLoaded;
    CKeyboardLayout* iLayout;

    };

// ----------------------------------------------------------------------

inline TInt CLayoutLibrary::Id() const
    {
    return iId;
    }

inline TInt CLayoutLibrary::Loaded() const
    {
    return iLoaded;
    }

inline CKeyboardLayout* CLayoutLibrary::Layout()
    {
    return iLayout;
    }

// ----------------------------------------------------------------------

#endif

// End of file


