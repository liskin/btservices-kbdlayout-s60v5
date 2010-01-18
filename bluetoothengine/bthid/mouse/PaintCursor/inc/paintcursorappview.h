/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/
// ----------------------------------------------------------------------
//
// Project: Bluetooth HID Profile
//
// "PaintCursorAppView.h" - BT Hid Simulator Application View Header File
//
//
// ----------------------------------------------------------------------
#ifndef __PaintCursorAPPVIEW_H__
#define __PaintCursorAPPVIEW_H__
#include <coecntrl.h>
#include <aknlists.h>
#include "mousecursordll.h"
#include "clientimagecommander.h"
_LIT( KMouseCurorSrvName, "\\system\\apps\\animation\\BTCURSOR_SERVER" );

/*!
 An instance of the CPaintCursorAppView View object for PaintCursor application
 */
class CPaintCursorAppView : public CCoeControl
    {
public:

    /*!
     Create a CPaintCursorAppView object, which will draw itself to aRect
     @param aRect the rectangle this view will be drawn to
     @result a pointer to the created instance of CPaintCursorAppView
     */
    static CPaintCursorAppView* NewL(const TRect& aRect);

    /*!
     Create a CPaintCursorAppView object, which will draw itself to aRect
     @param aRect the rectangle this view will be drawn to
     @result a pointer to the created instance of CPaintCursorAppView
     */
    static CPaintCursorAppView* NewLC(const TRect& aRect);

    /*!
     Destroy the object and release all memory objects
     */
    ~CPaintCursorAppView();

public:
    // from CCoeControl

    void SizeChanged();

    void Draw(const TRect& aRect) const;

    void HideCursor();

    void ShowCursor();
private:

    /*!
     Perform the second phase construction of a CPaintCursorAppView object
     @param aRect the rectangle this view will be drawn to
     */
    void ConstructL(const TRect& aRect);

    /*!
     Perform the first phase of two phase construction 
     */
    CPaintCursorAppView();

    /**
     * SetupMouseCursorDllL.
     * Setup the mouse cursor animation client Dll.
     */
    void SetupMouseCursorDllL();
    /**
     * SetupImageCommanderL.
     * Setup the animation image commander.
     */
    void SetupImageCommanderL();

private:

    /**
     * iMouseCursorDll
     * Mouse cursor Animation client dll.
     */
    RMouseCursorDll iMouseCursorDll;
    /**
     * iClientCommander
     * Controls an animation image object.
     */
    RImageCommander iClientCommander;

    TBool iMouseInitialized;
    };

#endif // __PaintCursorAPPVIEW_H__
