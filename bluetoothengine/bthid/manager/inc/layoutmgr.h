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

/*!
 \file 
 Keyboard layout manager interface, public header.
 */
#ifndef __LAYOUTMGR_H
#define __LAYOUTMGR_H
#include <e32std.h>
#include <e32svr.h>
#include "genericclient.h"
#include "hidlayoutids.h"
// CONSTANTS
/*!
 Layout server sessions are created indirectly using this server name.
 */
_LIT(KLayoutServerName,"LayoutServer");
/*!
 Name of the semaphore used to confirm server startup.  Semaphores
 are globally accessible kernel objects and are referenced by name.
 */
_LIT(KLayoutServerSemaphoreName, "LayoutServerSemaphore");

// FORWARD DECLARATIONS
class TDecodedKeyInfo;
class TLockKeys;
class TTranslatedKey;

/*!
 Handle class representing a session with the keyboard layout
 manager server.
 */

// CLASS DECLARATION

class RLayoutManager : public RGenericSession
    {
public:
    IMPORT_C RLayoutManager();

    /*!
     Connect to the keyboard layout manager server and create a new session.

     @result KErrNone if successful; otherwise another of the standard 
     Symbian system-wide error codes.
     */
    IMPORT_C TInt Connect();

    // ------------------------------------------
    //  API intended for use by keyboard driver:
    // ------------------------------------------

    /*!
     Notify the layout server of a key-down or key-up event. Every
     key down event must be followed by a corresponding key up event;
     the two events do not have to be consecutive.

     @param aIsKeyDown ETrue for a key down event, EFalse for a key up event.
     @param aHidKey HID usage ID for the key causing this event
     @param aUsagePage HID usage page for the key causing this event
     @param aModifiers Current modifier state
     @param aLockKeys Current lock key state
     @param aDecodedKeys The results of the key decoding

     @result KErrNone if successful; otherwise another of the standard 
     Symbian system-wide error codes.
     */

    IMPORT_C TInt KeyEvent(TBool aIsKeyDown, TInt aHidKey, TInt aUsagePage,
            TInt aModifiers, const TLockKeys& aLockKeys,
            TDecodedKeyInfo& aDecodedKeys) const;

    /*! Convenience function, equivalent to #KeyEvent(ETrue, ...) */
    inline TInt KeyDownEvent(TInt aHidKey, TInt aUsagePage, TInt aModifiers,
            const TLockKeys& aLockKeys, TDecodedKeyInfo& aDecodedKeys) const;

    /*! Convenience function, equivalent to #KeyEvent(EFalse, ...) */
    inline TInt KeyUpEvent(TInt aHidKey, TInt aUsagePage, TInt aModifiers,
            const TLockKeys& aLockKeys, TDecodedKeyInfo& aDecodedKeys) const;

    /*!
     Reset the state of keyboard layout decoder object associated
     with this session.

     @result KErrNone if successful; otherwise another of the standard 
     Symbian system-wide error codes.
     */
    IMPORT_C TInt Reset() const;

    /*!
     %SetInitialLayout() causes the layout manager to select the most
     appropriate keyboard layout given the country, vendor and
     product parameters read from a keyboard device.  The server will
     choose the layout DLL (<b>reword here</b>) in the following
     order:

     -# By keyboard vendor and product code
     -# By keyboard country code
     -# By current phone language setting

     If no appropriate layout can be found, the server will attempt
     to load the default US layout (EUnitedStates).

     As the layout server only maintains a single keyboard layout,
     this setting will affect all other RLayoutManager sessions.

     @param aCountry HID country ID code for the keyboard device
     (refer to bCountryCode in "Device Class Definition for Human
     Interface Devices", USB Implementers' Forum, Version 1.11,
     2001, Section 6.2.1, "HID descriptor".)  

     @param aVendor HID manufacturer ID code for the keyboard device,
     as supplied by the USB Implementers' Forum, http://www.usb.org/.

     @param aProduct Vendor specific product code for the keyboard
     device.

     @result KErrNone if successful; otherwise another of the standard 
     Symbian system-wide error codes.
     */
    IMPORT_C TInt
            SetInitialLayout(TInt aCountry, TInt aVendor, TInt aProduct) const;

    /*!
     As SetInitialLayout() but will Leave() if there is an error. 
     */
    inline void
            SetInitialLayoutL(TInt aCountry, TInt aVendor, TInt aProduct) const;

    // ------------------------------------------
    //  API intended for use by applications:
    // ------------------------------------------

    /*!
     Query the current HID system keyboard layout

     @param aLayoutId Keyboard layout ID code

     @result KErrNone if successful; otherwise another of the standard 
     Symbian system-wide error codes.
     */
    IMPORT_C TInt GetLayout(TInt& aLayoutId) const;

    /*!
     As GetLayout() but will Leave() if there is an error. 
     
     @result Keyboard layout ID code
     */
    inline TInt GetLayoutL() const;

    /*!
     Query the initial layout chosen for the current keyboard.
     This is the result of the last call to SetInitialLayoutL().

     @param aLayoutId Keyboard layout ID code

     @result KErrNone if successful; otherwise another of the standard 
     Symbian system-wide error codes.
     */
    IMPORT_C TInt GetInitialLayout(TInt& aLayoutId) const;

    /*!
     As GetInitialLayout() but will Leave() if there is an error. 
     
     @result Keyboard layout ID code
     */
    inline TInt GetInitialLayoutL() const;

    /*!
     Set the HID system keyboard layout by ID.  If the server can't
     find a keyboard layout DLL containing the given layout ID, then
     the layout will remain unchanged.

     @param aLayoutId Keyboard layout ID code

     @result KErrNone if successful; otherwise another of the standard 
     Symbian system-wide error codes.
     */
    IMPORT_C TInt SetLayout(TInt aLayoutId) const;

    /*!
     As SetLayout() but will Leave() if there is an error. 
     */
    inline void SetLayoutL(TInt aLayoutId) const;

    /*!
     Return information about the attached keyboard

     @param aIsNokiaSu8 ETrue if the keyboard is a Nokia SU-8

     @param aFoundLayout ETrue if the keyboard returned a valid
     country code and a matching keyboard layout ID exists.

     @result KErrNone if successful; otherwise another of the standard 
     Symbian system-wide error codes.
     */
    IMPORT_C TInt GetDeviceInfo(TBool& aIsNokiaSu8, TBool& aFoundLayout) const;
    };

// ----------------------------------------------------------------------

inline TInt RLayoutManager::KeyDownEvent(TInt aHidKey, TInt aUsagePage,
        TInt aModifiers, const TLockKeys& aLockKeys,
        TDecodedKeyInfo& aDecodedKeys) const
    {
    return KeyEvent(ETrue, aHidKey, aUsagePage, aModifiers, aLockKeys,
            aDecodedKeys);
    }

inline TInt RLayoutManager::KeyUpEvent(TInt aHidKey, TInt aUsagePage,
        TInt aModifiers, const TLockKeys& aLockKeys,
        TDecodedKeyInfo& aDecodedKeys) const
    {
    return KeyEvent(EFalse, aHidKey, aUsagePage, aModifiers, aLockKeys,
            aDecodedKeys);
    }

// ----------------------------------------------------------------------

inline void RLayoutManager::SetInitialLayoutL(TInt aCountry, TInt aVendor,
        TInt aProduct) const
    {
    User::LeaveIfError(SetInitialLayout(aCountry, aVendor, aProduct));
    }

inline void RLayoutManager::SetLayoutL(TInt aLayoutId) const
    {
    User::LeaveIfError(SetLayout(aLayoutId));
    }

inline TInt RLayoutManager::GetLayoutL() const
    {
    TInt layout;
    User::LeaveIfError(GetLayout(layout));
    return layout;
    }

inline TInt RLayoutManager::GetInitialLayoutL() const
    {
    TInt layout;
    User::LeaveIfError(GetInitialLayout(layout));
    return layout;
    }

// ----------------------------------------------------------------------

#endif
