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
* Description:  Bluetooth Toggle application is being used by Telephony for 
 *  							switching on/off Bluetooth when a long key press of asterisk 
 *  							is detected 
 *
*/



//  Include Files  
#include "bttoggle.h"
#include "btengprivatecrkeys.h"
#include "btengsrvbbconnectionmgr.h"
#include "debug.h"

#include <e32base.h>
#include <e32std.h>

void Panic(TBTTogglePanic aPanic)
  {
  User::Panic(KBTTogglePanicCat, aPanic);
  }

LOCAL_C void DoStartL()
  {
  //Create active scheduler (to run active objects)
  CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
  CleanupStack::PushL (scheduler );
  CActiveScheduler::Install (scheduler );

  CBTToggle* bttoggle = CBTToggle::NewL();			
  bttoggle->ToggleBT();

  CActiveScheduler::Start();

  delete bttoggle; 		
  CleanupStack::PopAndDestroy (scheduler );
  return; 
  }

//  Global Functions
GLDEF_C TInt E32Main()
  {	 	
  CTrapCleanup* cleanup = CTrapCleanup::New();	

  // Run application code inside TRAP harness		
  TRAPD(mainError, DoStartL());
  if (mainError)
    {
    TRACE_INFO((_L("[BTENG][BTTOGGLE]DoStartL error: %d" ), mainError))
    }
  delete cleanup;		
  return KErrNone;
  }

// ==========================================================
// CBTToggle::CBTToggle()
// Constructor
// ==========================================================
//
CBTToggle::CBTToggle() : CActive(EPriorityStandard)
		    {		
		    }

// ==========================================================
// CBTToggle::~CBTToggle()
// Destructor
// ==========================================================
//
CBTToggle::~CBTToggle()
  {
  TRACE_FUNC_ENTRY

  Cancel(); 		
  delete iSettings; 	
  iNotifier.Close();		
  iLinkCountProperty.Close();

  TRACE_FUNC_EXIT
  }

// ==========================================================
// CBTToggle::ConstructL
// 
// ==========================================================
//
void CBTToggle::ConstructL()
  {
  TRACE_FUNC_ENTRY		
  iSettings = CBTEngSettings::NewL(NULL);	
  CActiveScheduler::Add(this); 	
  iActiveNotifier = ENoneQuery; 
  TInt errConnect = iNotifier.Connect();	
  if (errConnect)
    {
    TRACE_INFO((_L("[BTENG][BTTOGGLE] iNotifier connect fail: %d" ), errConnect))
    User::Leave(errConnect);
    }				
  User::LeaveIfError( iLinkCountProperty.Attach( KPropertyUidBluetoothCategory, 
      KPropertyKeyBluetoothGetPHYCount ) );	  
  TRACE_FUNC_EXIT
  }

// ==========================================================
// CBTToggle::NewL
//
// ==========================================================
//
CBTToggle* CBTToggle::NewL()
  {
  TRACE_FUNC_ENTRY
  CBTToggle* self = new ( ELeave ) CBTToggle();
  CleanupStack::PushL(self);  
  self->ConstructL();
  CleanupStack::Pop(self);
  TRACE_FUNC_EXIT
  return self; 
  }

// ==========================================================
// CBTToggle::ToggleBT
// Toggles Bluetooth on or off and shows dialog 
// (if the BT is switched for the first time after flashing)
// and notification
// ==========================================================
//
void CBTToggle::ToggleBT()
  {
  TRACE_FUNC_ENTRY	

  TBTPowerStateValue state;	
  iSettings->GetPowerState(state);	  

  //BT off
  if (EBTPowerOff == state)
    {					
    TBool isInOffline = CheckOfflineMode(); 
    if (isInOffline) //Offline mode
      {					
      TRACE_INFO((_L("[BTENG][BTTOGGLE]Offline mode") ))
      SwitchBtOnWithPowerNotif(); 						
      }
    else //other than offline
      {									
      TBuf<KHCILocalDeviceNameMaxLength> localName;
      iSettings->GetLocalName(localName);														

      //There is no BT local name defined		
      if (0 >= localName.Length()) 
        {																
        SwitchBtOnWithPowerNotif(); 
        }
      else //BT name already defined
        {				
        ChangeBtPowerMode(EBTPowerOn);
        }				
      } 		
    } 	
  else //BT on 
    {				
    TInt linkCount = 0; 
    TInt errLinkCount = iLinkCountProperty.Get( linkCount );		
    TRACE_INFO((_L("[BTENG][BTTOGGLE] linkCount: %d error: %d" ), linkCount, errLinkCount))		

    //BT Connected or error obtaining link count (in the case of error, we are cautious and assume BT is connected)
    if (errLinkCount || linkCount > 0)
      {
      TRACE_INFO((_L("[BTENG][BTTOGGLE]Ongoing BT connection") ))
      __ASSERT_DEBUG(iActiveNotifier == ENoneQuery, Panic(EBTToggleInvalidStateCloseConnectionQuery));

      iActiveNotifier = ECloseConnectionQuery;
      iPckgGenericQuery().iMessageType = EBTSwitchOffAnyway;
      iPckgGenericQuery().iNameExists = EFalse;
      iNotifier.StartNotifierAndGetResponse(iStatus, KBTGenericQueryNotifierUid, iPckgGenericQuery, iConnQuery );
      SetActive();
      }
    //No open connections
    else
      {
      TRACE_INFO((_L("[BTENG][BTTOGGLE] No BT connections") ))
      ChangeBtPowerMode(EBTPowerOff);
      }
    }
  TRACE_FUNC_EXIT	
  }

// ==========================================================
// CBTToggle::CheckOfflineMode
// Checks whether current profile is Offline  
// ==========================================================
//
TBool CBTToggle::CheckOfflineMode()
  {
  TRACE_FUNC_ENTRY
  TCoreAppUIsNetworkConnectionAllowed offline = ECoreAppUIsNetworkConnectionAllowed;  
  TBTEnabledInOfflineMode enabledInOffline; 
  iSettings->GetOfflineModeSettings(offline, enabledInOffline);	
  TRACE_FUNC_EXIT
  return !offline; 	
  }

// ==========================================================
// CBTToggle::SwitchBtOnWithPowerNotif
// Switches BT on using Power Mode notifier   
// ==========================================================
//
void CBTToggle::SwitchBtOnWithPowerNotif()
  {	
  TRACE_FUNC_ENTRY
  __ASSERT_DEBUG(iActiveNotifier == ENoneQuery, Panic(EBTToggleInvalidStatePowerModeQuery));

  iActiveNotifier = EPowerModeQuery;
  // EBTNameQuery is used here as the message type, since the local device name may be modified by the user upon power-on.
  iPckgGenericQuery().iMessageType = EBTNameQuery;			
  iNotifier.StartNotifierAndGetResponse(iStatus, KPowerModeSettingNotifierUid, iPckgGenericQuery, iPowerResult );				
  SetActive();
  TRACE_FUNC_EXIT 	
  }


// ==========================================================
// CBTToggle::ChangeBtPowerMode
// Change BT Power Mode using CBTEngSettings API 
// and show notification
// ==========================================================
//
TInt CBTToggle::ChangeBtPowerMode(TBTPowerStateValue aNewPowerState)
  {
  TRACE_FUNC_ENTRY
  __ASSERT_DEBUG(iActiveNotifier == ENoneQuery, Panic(EBTToggleInvalidStateChangeBtPowerMode));

  TInt errPower = iSettings->SetPowerState(aNewPowerState);
  if (KErrNone == errPower)
    {
    ShowNotification( static_cast<TBool>(aNewPowerState) );
    iActiveNotifier = EPowerModeChangeNote; 
    iStatus = KRequestPending;
    SetActive();
    }

  TRACE_FUNC_EXIT
  return errPower;
  }

// ==========================================================
// CBTToggle::ShowNotification
// Presents generic info notification about BT on (ETrue) 
// and BT off (EFalse)  
// ==========================================================
//
void CBTToggle::ShowNotification( TBool aStatus )
  {
  TRACE_FUNC_ENTRY
  TBTGenericInfoNotiferParamsPckg pckgGenericInfo; 	 	 	 
  TBuf8<sizeof(TInt)> result;

  //BT turned on
  if (aStatus)
    {
    pckgGenericInfo().iMessageType = EBTSwitchedOn;  		 
    }
  //BT turned off
  else
    {
    pckgGenericInfo().iMessageType = EBTSwitchedOff;		
    }

  //Cancel if there is any outstanding notification
  Cancel();

  //Start new notification
  iNotifier.StartNotifierAndGetResponse(iStatus, KBTGenericInfoNotifierUid, pckgGenericInfo, result);	

  TRACE_FUNC_EXIT
  }

// ==========================================================
// CBTToggle::DoCancel
// Cancel 
// ==========================================================
//
void CBTToggle::DoCancel()
  {	
  TRACE_FUNC_ENTRY
  if ( EPowerModeQuery == iActiveNotifier )
    {
    iNotifier.CancelNotifier(KPowerModeSettingNotifierUid); 		
    }
  else if ( ECloseConnectionQuery == iActiveNotifier ) 
    {
    iNotifier.CancelNotifier(KBTGenericQueryNotifierUid); 
    }  
  else if ( EPowerModeChangeNote == iActiveNotifier ) 
    {
    iNotifier.CancelNotifier(KBTGenericInfoNotifierUid); 
    }

  // For all cancels, we must reset iActiveNotifier back to ENoneQuery
  // to permit another request to be made.
  iActiveNotifier = ENoneQuery;

  TRACE_FUNC_EXIT
  }

// ==========================================================
// CBTToggle::RunL
// Inherited from CActive
// ==========================================================
//
void CBTToggle::RunL()
  {
  TRACE_FUNC_ENTRY	

  User::LeaveIfError(iStatus.Int());

  // For all completes, we must reset iActiveNotifier back to ENoneQuery
  // to permit another request to be made.
  switch (iActiveNotifier)
    {							
    case EPowerModeQuery:
      ShowNotification(ETrue);   
      iActiveNotifier = EPowerModeChangeNote ;
      SetActive();
      break; 			
    case ECloseConnectionQuery:							
      iActiveNotifier = ENoneQuery;
      if (iConnQuery()) //user wants to stop existing connection
        {
        ChangeBtPowerMode(EBTPowerOff);										
        }					
      else
        {
        CActiveScheduler::Stop();
        }																				
      break;
    default: //ENoneQuery or EPowerModeChangeNote
      iActiveNotifier = ENoneQuery;
      CActiveScheduler::Stop();
    }						
  
  TRACE_FUNC_EXIT	
  }

// ==========================================================
// CBTToggle::RunError
// Inherited from CActive
// ==========================================================
//
TInt CBTToggle::RunError(TInt aError)
    {	  
    TRACE_INFO((_L("[BTENG][BTTOGGLE] RunError %d"), aError ))	
    iActiveNotifier = ENoneQuery;
    CActiveScheduler::Stop();
    return KErrNone;
    }
