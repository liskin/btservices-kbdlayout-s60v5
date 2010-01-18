/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declaration of P&S listener.
*
*/


#ifndef BTAUDIOPLAYERCONTROLLISTENER_H
#define BTAUDIOPLAYERCONTROLLISTENER_H

#include <e32base.h>
#include <e32property.h>

class MBmbPlayerControl
    {
public:
    virtual void Pause() = 0;
    virtual void Play()  = 0;
    };

class CBTAudioPlayerControlListener : public CActive
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CBTAudioPlayerControlListener* NewL(MBmbPlayerControl& aControl);

        /**
        * Destructor.
        */
        ~CBTAudioPlayerControlListener();
        
    private:
        /**
        * C++ default constructor.
        */
        CBTAudioPlayerControlListener(MBmbPlayerControl& aControl);
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
    public:
        /**
        * Starts asynchronous command handling
        * @param None
        * @return None
        */
	    void Start();

    private:
        /**
        * From CActive. Called when asynchronous request completes.
        * @param None
        * @return None
        */
	    void RunL();

        /**
        * From CActive. Cancels asynchronous request.
        * @param None
        * @return None
        */
	    void DoCancel();
    
    public:
        /**
        * Returns the class status.
        * @param None
        * @return Status code.
        * @since 2.5
        */
        TInt Status() const;

    private:
        MBmbPlayerControl& iControl;

		RProperty iProperty;
    };

#endif
