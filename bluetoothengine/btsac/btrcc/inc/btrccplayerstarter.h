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
* Description:   This class implements starting Music play	
*
*
*/


#ifndef BTRCCPLAYERSTATER_H
#define BTRCCPLAYERSTATER_H

//  INCLUDES
#include <e32base.h>
#include <remconcoreapitargetobserver.h>

// FORWARD DECLARATIONS
class CRemConCoreApiTarget;

/**
*  This class listens to AVRCP PLAY command by headset,if music player is not running 
*  when such a command is received, it will be launched and requested to start playing.
* */
NONSHARABLE_CLASS(CPlayerStarter) : 
    public CActive, public MRemConCoreApiTargetObserver
	{
	
public:

	/**
	* Two-phased constructor.
	*      
	*/
	static CPlayerStarter* NewL();

	/**
	 * Set the reference for AVRCP Core API command receiving and
	 * responses to those command.
	 */
	void SetCoreTarget(CRemConCoreApiTarget& aTarget);
	
	/**
	* Destructor.
	*/
	~CPlayerStarter();
	
private:  // From CActive

    /**
    * Informs the observer that an accessory has created a control 
    * connection to our device.
    * @param                            None.
    * @return                           None.
    */
    void RunL();

    /**
    * Handles a possible leave from RunL().
    * @param aError                     The leave code from RunL().
    * @return                           KErrNone.
    */
    TInt RunError(TInt aError);

    /**
    * Cancels the volume change operation.
    * @param                            None.
    * @return                           None.
    */
    void DoCancel();
    
    // from base class MRemConCoreApiTargetObserver

    /**
     * From MRemConCoreApiTargetObserver
     * A 'play' command has been received.
     *
     * @param aSpeed The playback speed.
     * @param aButtonAct The button action associated with the command.
     */
    void MrccatoPlay(
        TRemConCoreApiPlaybackSpeed aSpeed,
        TRemConCoreApiButtonAction aButtonAct );

    /**
     * From MRemConCoreApiTargetObserver
     * A command has been received.
     *
     * @param aOperationId The operation ID of the command.
     * @param aButtonAct The button action associated with the command.
     */
    void MrccatoCommand(
        TRemConCoreApiOperationId aOperationId,
        TRemConCoreApiButtonAction aButtonAct );

private:

        /**
        * C++ default constructor.
        */
        CPlayerStarter();

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

        /**
         * Start music player if it is not running yet.
         */
        void StartPlayIfNeeded();
        
        /**
         * Tells if MPX music player is running or not.
         */
        TBool IsMusicPlayerRunning();
        
        /**
         * Tells if Video player is running or not.
         */
        TBool IsVideoPlayerRunning();

        /**
         * Launch MPX player.
         */
        void LaunchMusicPlayerL();
        
private:
    enum TOpState
        {
        EIdle = 0, 
        ERespondPlayOk , // responding OK to play command is outstanding
        ERespondPlayNok,   // responding NOT OK to play command is outstanding
        EPlayMusicTiming,  // scheduling internal PLAY command to player
        };
    
private:    // Data
    
    CRemConCoreApiTarget* iCoreTarget; // not own
    
	RTimer iTimer;
	
	TOpState iState;
    };

#endif      // BTRCCACLLINKLISTENER_H
            
// End of File
