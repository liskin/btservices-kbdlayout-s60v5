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
* Description:  Bluetooth Toggle application meant used by Telephony for 
 *  							switching on/off Bluetooth when a long key press of asterisk 
 *  							is detected 
 *
*/


#ifndef __BTTOGGLE_H__
#define __BTTOGGLE_H__

//  Include Files

#include <e32base.h>
#include <btengsettings.h>
#include <bt_sock.h>
#include <btnotif.h>

//  Function Prototypes

GLDEF_C TInt E32Main();

_LIT(KBTTogglePanicCat, "BTToggle Panic");

/**
 *TBTTogglePanic - Panics that can be raised by the BTToggle component.
 */
enum TBTTogglePanic
  {
  EBTToggleInvalidStateCloseConnectionQuery	= 0,	// A notifier could not be started for ECloseConnectionQuery as another notifier is already being waited on.
  EBTToggleInvalidStatePowerModeQuery		= 1,	// A notifier could not be started for EPowerModeQuery as another notifier is already being waited on.
  EBTToggleInvalidStateChangeBtPowerMode	= 2,	// A notifier could not be started by CBTToggle::ChangeBtPowerMode() as another notifier is already being waited on.
  };

void Panic(TBTTogglePanic aPanic);

/**
 * Class used for switching BT on/off.
 *
 *  @lib bttoggle.exe
 *  @since S60 5.1
 */
NONSHARABLE_CLASS(CBTToggle): public CActive						
  {
  public:		

    /**
     * Two-phase constructor
     * @since S60 v5.1 
     * @return Pointer to the constructed CBTToggle object.
     */
    static CBTToggle* NewL();

    /**
     * Destructor
     */
    ~CBTToggle();

    /**
     * Toggles Bluetooth on/off 
     *
     * @since S60 v5.1
     * @return void 
     */
    void ToggleBT();	

    /**
     * Checks whether phone is in offline mode.
     *
     * @since S60 v5.1
     * 
     * @return TBool - ETrue: offline mode enabled; 
     * EFalse: diffrent than offline  
     */
    TBool CheckOfflineMode(); 

    /**
     * Switches BT on using PowerMode notifier
     *
     * @since S60 v5.1
     * 
     * @return void   
     */
    void SwitchBtOnWithPowerNotif(); 

  private: //From CActive

    void RunL(); 	
    TInt RunError(TInt /*aError*/); 
    void DoCancel();			

  private:		

    /**
     * C++ default constructor
     *
     * @since S60 v5.1		     
     */
    CBTToggle();

    /**
     * Symbian 2nd-phase constructor
     *
     * @since S60 v5.1
     */
    void ConstructL();

    /**
     * Shows the notification if Bluetooth is switched on/off.
     *
     * @since S60 v5.1
     * @param aStatus The type of notification to be shown; 
     * ETrue - BT switched on; EFalse - BT switched off. 
     * 	 
     * @return void
     */
    void ShowNotification(TBool aStatus); 

    /**
     * Switches BT on or off and shows standard notification
     *
     * @since S60 v5.1
     * @param TBTPowerStateValue aNewPowerMode 
     * 	 
     * @return TInt Symbian error code
     */
    TInt ChangeBtPowerMode(TBTPowerStateValue aNewPowerMode);


  private: //Data types

    enum TToggleActiveNotifier
      {
      ENoneQuery = 0,		    
      EPowerModeQuery,
      ECloseConnectionQuery, 
      EFakeNotif
      };

  private: //Data	
    /**
     *  
     */
    CBTEngSettings* iSettings;	//own

    /**
     * Handle to the notifier
     */
    RNotifier iNotifier;

    /**
     * Result package from the notifier's asynchronous call 
     * (Generic Query notifier) 
     */    
    TPckgBuf<TBool> iConnQuery;

    /**
     ** Result package from the notifier's asynchronous call 
     * (Power Mode notifier) 
     */
    TPckgBuf<TInt> iPowerResult;

    /**
     * Notifier which is currently being in use
     */
    TToggleActiveNotifier iActiveNotifier;

    /**
     * P&S key. Shows whether the BT has open connection  
     */
    RProperty iLinkCountProperty;        

	/**
	 * Notifier's buffer holder  
	 */
    TBTGenericQueryNotiferParamsPckg iPckgGenericQuery;
  };


#endif  // __BTTOGGLE_H__

