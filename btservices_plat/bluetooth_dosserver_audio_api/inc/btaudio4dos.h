/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Engine ECom plug-in interface for DosServer audio routing.
*
*/



#ifndef BT_AUDIO_FOR_DOSSERVER_H
#define BT_AUDIO_FOR_DOSSERVER_H

#include <e32base.h>

const TUid KBTAudio4DosInterfaceUid = { 0x1020896B };

/**
 *  Bluetooth Audio API when DosServer is in use.
 *
 *  This is the base class from which a BT DosServer audio ECom plug-in inherit. 
 *  The BT DosServer audio plug-in is only needed when old DosServer is in use. 
 *  Since S60 3.1, the Accessory Framework is replacing the DosServer and
 *  taking care of BT audio routing. The API is deprecated whenever the Accessory
 *  Framwork is in place.
 * 
 *  @since S60 v3.2
 */
class CBTAudio4Dos : public CBase
    {

public:

    /**
     * Constructor; loads the plug-in through the ECom framework.
     *
     * @param aImplementationUid a UID specifying the required interface implementation
     * @return an intance of CBTAudio4Dos
     */
    static CBTAudio4Dos* NewL(TUid aImplementationUid);

    /**
     * Destructor; destroys the plug-in through the ECom framework.
     */
    virtual ~CBTAudio4Dos();

    /**
    * Routes Audio to phone.
    * @return a completion error code
    */
    virtual TInt AudioToPhone() = 0;

    /**
    * Routes Audio to accessory.
    * @param aStatus On completion, will contain an error code
    */
    virtual void AudioToAccessory(TRequestStatus& aStatus) = 0;

    /**
    * Cancel Audio routing to accessory request.
    */
    virtual void CancelAudioToAccessory() = 0;

private:

    /**
     * UID set by ECOM when the instance is created.
     * Used when the instance is destroyed.
     */
    TUid iInstanceUid;
    };

#include <btaudio4dos.inl>

#endif

