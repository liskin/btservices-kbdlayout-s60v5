/*
* Copyright (c) 2004 - 2006 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __MOUSECURSORIMAGE_H__
#define __MOUSECURSORIMAGE_H__

// INCLUDES
#include <w32adll.h>
#include "pointmsgqueue.h"

// CONSTANTS

// Pixels the animation moves each timer event
static const TInt KStepSize = 15;

// The timer timeout resolution
#define KTimerPeriod (50000)

// CLASS DECLARATION

/**
 * CImage.
 * An instance of class CImage encapsulates the functionality
 * of the bouncing square on the screen.
 */
class CImage : public CSpriteAnim
    {

public:
    // Constructors and destructors

    /**
     * CImage()
     * Construct the object, forced to be public.
     */
    CImage();

    /**
     * ~CImage()
     * Virtual destructor.
     */
    virtual ~CImage();

public:
    // From CSpriteAnim

    /**
     * ConstructL()
     * Complete animation server object construction
     * (called by windows server).
     * @param aArgs any arguments required during construction
     * (passed from client through window server)
     */
    void ConstructL(TAny* aArgs);

    /**
     * CommandReplyL()
     * Perform aCommand with optional additional arguments aArgs and
     * return a reply (called by windows server).
     * Note! This method can return an error, so any actions that can leave
     * should use this method and not Command ().
     * This command is not buffered and so requests from the client side are
     * performed immediately.  For these reason these functions should
     * not take a long time.
     * @param aCommand the command to perform
     * @param aArgs any additional arguments required.
     */
    TInt CommandReplyL(TInt aCommand, TAny* aArgs);

    /**
     * Command()
     * Perform aCommand with optional additional arguments aArgs.
     * Note! Because there is no way to return an error from this method, it
     * is important that these commands should not be able to fail or leave.
     * This command is buffered and so may have been requested by the client
     * side an unknown period ago.
     * @param aCommand the command to perform
     * @param aArgs any additional arguments required
     */
    void Command(TInt aCommand, TAny* aArgs);

    /**
     * Animate()
     * Animate the object, normally called by the Windows Server
     * at a fixed period
     */
    void Animate(TDateTime* aDateTime);

    /**
     * FocusChanged()
     * Handle the event when the focus changes.
     * @param aState TRUE if the Animation Server has focus
     */
    void FocusChanged(TBool aState);

    /**
     * OfferRawEvent()
     * For handling raw events. Not used in this example so return false.
     */
    TBool OfferRawEvent(const TRawEvent &aRawEvent);

private:
    void DrawCursor();

private:
    // Data
    CFbsBitmapDevice* iBitmapDevice;
    CFbsBitmapDevice* iBitmapMaskDevice;
    CFbsBitGc* iSpriteGc;
    CFbsBitmap* iCursorBitmap;
    CFbsBitmap* iCursorBitmapMask;
    };

#endif // __MOUSECURSORIMAGE_H__
// End of File
