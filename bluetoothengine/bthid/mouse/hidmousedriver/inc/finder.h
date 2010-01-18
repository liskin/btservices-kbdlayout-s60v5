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


#ifndef __FINDER_H
#define __FINDER_H

#include <e32std.h>
#include "hidreportroot.h"

// ----------------------------------------------------------------------

/**
 *  Field finder for the ordinary mouse fields.
 */
class TMouseFinder : public MHidFieldFinder
    {
public:
    // From MHidFieldFinder
    virtual TBool BeginCollection(const CCollection *aCollection);
    virtual TBool EndCollection(const CCollection *aCollection);
    virtual void Field(const CField* aField);

public:
    /** Constructor */
    TMouseFinder();

    /**
     *  Get the field containing the standard keys, if found.
     *  @return CField* Pointer to the field, or NULL.
     */
    inline const CField* ButtonsField() const;
    /**
     *  Get the field containing the xy field, if found.
     *  @return CField* Pointer to the field, or NULL.
     */
    inline const CField* XYField() const;
    /**
     *  Get the field containing the wheel info, if found.
     *  @return CField* Pointer to the field, or NULL.
     */
    inline const CField* WheelField() const;
    /**
     *  Check whether the required fields have been found.
     *  @return ETrue if they have.
     */
    inline TBool Found() const;

    /**
     *  Check whether a given field contains the XY Coordinates
     *  @param aField Pointer to the field to test.
     *  @return ETrue if it does.
     */
    TBool IsXY(const CField* aField) const;
    /**
     *  Check whether a given field contains the Buttons.
     *  @param aField Pointer to the field to test.
     *  @return ETrue if it does.
     */
    TBool IsButtons(const CField* aField) const;
    /**
     *  Check whether a given field contains a wheel.
     *  @param aField Pointer to the field to test.
     *  @return ETrue if it does.
     */
    TBool IsWheel(const CField* aField) const;

private:
    /** Pointer to the field containing the XY pointer info. */
    const CField* iXY;

    /** Pointer to the field containing the buttons. */
    const CField* iButtons;

    /** Pointer to the field containing the Wheel info. */
    const CField* iWheel;

    /** Pointer to the top level application collection being searched. */
    const CCollection* iAppCollection;
    };
// ----------------------------------------------------------------------

inline TBool TMouseFinder::Found() const
    {
    // Standard and modifier key fields are always necessary, but the
    // Wheel field is optional:
    //
    return iButtons && iXY;
    }

inline const CField* TMouseFinder::ButtonsField() const
    {
    return iButtons;
    }

inline const CField* TMouseFinder::XYField() const
    {
    return iXY;
    }

inline const CField* TMouseFinder::WheelField() const
    {
    return iWheel;
    }

#endif
