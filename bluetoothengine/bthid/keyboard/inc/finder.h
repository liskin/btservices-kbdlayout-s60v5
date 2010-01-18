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
 *  Field finder for the ordinary keyboard fields.
 */
class TKeyboardFinder : public MHidFieldFinder
    {
public:
    // From MHidFieldFinder
    virtual TBool BeginCollection(const CCollection *aCollection);
    virtual TBool EndCollection(const CCollection *aCollection);
    virtual void Field(const CField* aField);

public:
    /** Constructor */
    TKeyboardFinder();
    /**
     *  Check whether the required fields have been found.
     *  @return ETrue if they have.
     */
    inline TBool Found() const;

    /**
     *  Get the field containing the standard keys, if found.
     *  @return CField* Pointer to the field, or NULL.
     */
    const CField* StandardKeyField() const;
    /**
     *  Get the field containing the modifier keys, if found.
     *  @return CField* Pointer to the field, or NULL.
     */
    const CField* ModifierKeyField() const;
    /**
     *  Get the field containing the LEDs, if found.
     *  @return CField* Pointer to the field, or NULL.
     */
    const CField* LedField() const;

private:
    /**
     *  Check whether a given field contains the standard keys.
     *  @param aField Pointer to the field to test.
     *  @return ETrue if it does.
     */
    TBool IsStandardKeys(const CField* aField) const;
    /**
     *  Check whether a given field contains the modifier keys.
     *  @param aField Pointer to the field to test.
     *  @return ETrue if it does.
     */
    TBool IsModifiers(const CField* aField) const;
    /**
     *  Check whether a given field contains the LEDs.
     *  @param aField Pointer to the field to test.
     *  @return ETrue if it does.
     */
    TBool IsLeds(const CField* aField) const;

private:
    /** Pointer to the field containing the standard keys. */
    const CField* iStandardKeys;
    /** Pointer to the field containing the modifier keys. */
    const CField* iModifierKeys;
    /** Pointer to the field containing the LEDs. */
    const CField* iLeds;

    /** Pointer to the top level application collection being searched. */
    const CCollection* iAppCollection;
    };

/**
 *  Field finder for the consumer/multimedia keys field.
 */
class TConsumerKeysFinder : public MHidFieldFinder
    {
public:
    // From MHidFieldFinder
    virtual TBool BeginCollection(const CCollection *aCollection);
    virtual TBool EndCollection(const CCollection *aCollection);
    virtual void Field(const CField* aField);

public:
    /** Constructor */
    TConsumerKeysFinder();

    /**
     *  Get the field containing the consumer keys, if found.
     *  @return CField* Pointer to the field, or NULL.
     */
    inline const CField* ConsumerKeysField() const;
    /**
     *  Check whether the consumer keys field has been found.
     *  @return ETrue if it has.
     */
    inline TBool Found() const;

private:
    /** Pointer to the field containing the consumer keys. */
    const CField* iField;
    };

/**
 *  Field finder for the power keys field.
 */
class TPowerKeysFinder : public MHidFieldFinder
    {
public:
    // From MHidFieldFinder
    virtual TBool BeginCollection(const CCollection *aCollection);
    virtual TBool EndCollection(const CCollection *aCollection);
    virtual void Field(const CField* aField);

public:
    /** Constructor */
    TPowerKeysFinder();

    /**
     *  Get the field containing the power keys, if found.
     *  @return CField* Pointer to the field, or NULL.
     */
    inline const CField* PowerKeysField() const;
    /**
     *  Check whether the power keys field has been found.
     *  @return ETrue if it has.
     */
    inline TBool Found() const;

private:
    /** Pointer to the field containing the power keys. */
    const CField* iField;
    };

// ----------------------------------------------------------------------

inline TBool TKeyboardFinder::Found() const
    {
    // Standard and modifier key fields are always necessary, but the
    // LED field is optional:
    //
    return iStandardKeys && iModifierKeys;
    }

inline const CField* TKeyboardFinder::StandardKeyField() const
    {
    return iStandardKeys;
    }

inline const CField* TKeyboardFinder::ModifierKeyField() const
    {
    return iModifierKeys;
    }

inline const CField* TKeyboardFinder::LedField() const
    {
    return iLeds;
    }

// ----------------------------------------------------------------------

inline const CField* TConsumerKeysFinder::ConsumerKeysField() const
    {
    return iField;
    }

inline TBool TConsumerKeysFinder::Found() const
    {
    return (iField != 0);
    }

// ----------------------------------------------------------------------

inline const CField* TPowerKeysFinder::PowerKeysField() const
    {
    return iField;
    }

inline TBool TPowerKeysFinder::Found() const
    {
    return (iField != 0);
    }

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
