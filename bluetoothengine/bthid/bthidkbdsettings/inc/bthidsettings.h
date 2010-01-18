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
* Description:  BT HID settings header 
 *
*/

#ifndef BTHIDSETTINGS_H
#define BTHIDSETTINGS_H

// INCLUDES
#include <e32base.h>
#include "layoututils.h"

// CLASS DECLARATION
/**
 * BT HID settings CenRep handler.
 *
 * @lib btkeyboard.exe
 */
class CBtHidSettings : public CBase
    {
public:
    // Constructors and destructor
    /**
     * Two-phased constructor.
     */
    IMPORT_C static CBtHidSettings* NewL();

    /**
     * Two-phased constructor. Leaves object in cleanup stack.
     */
    IMPORT_C static CBtHidSettings* NewLC();

    /**
     * Destructor.
     */
    virtual ~CBtHidSettings();

public:
    // Functions from base classes


public:
    // New functions


    /**
     * C++ default constructor.
     */
    CBtHidSettings();

    /*!
     Retrieve the current keyboard layout value.
     */
    IMPORT_C THidKeyboardLayoutId LoadLayoutSetting();

    /*!
     Set the layout value.
     @param aNewValue The new layout value
     */
    IMPORT_C void SaveLayoutSettingL(THidKeyboardLayoutId aNewValue) const;


private:

    /**
     * By default Symbian 2nd phase constructor is private.
     */
    void ConstructL();

private:
    // New functions
    /**
     * Set value to CenRep
     * @since 
     * @param aKey CenRep key
     * @param aValue Integer value.
     * @return status
     */
    TInt SetValue(const TUint32 aKey, const TInt aValue) const;

    /**
     * Get value from CenRep
     * @since 
     * @param aKey CenRep key
     * @param aValue Integer value.
     * @return status
     */
    TInt GetValue(const TUint32 aKey, TInt& aValue);

private:
    // Data


    };
#endif // BTHIDSETTINGS_H
// End of File
