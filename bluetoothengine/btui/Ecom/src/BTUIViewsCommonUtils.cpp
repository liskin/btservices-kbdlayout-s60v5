/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Utility function for the views.
*
*/

#include <BtuiViewResources.rsg>    // Compiled resource ids
#include <aknnotewrappers.h>
#include <StringLoader.h>   		// Series 60 localisation stringloader
#include <e32property.h>
#include <bt_subscribe.h>
#include <featmgr.h>            	// Feature Manager API
#include <AknMediatorFacade.h>
#include <btengsettings.h>
#include <ctsydomainpskeys.h>
#include <CoreApplicationUIsSDKCRKeys.h>
#include <SecondaryDisplay/BtuiSecondaryDisplayAPI.h>

#include "BTUIViewsCommonUtils.h"
#include "debug.h"


// ----------------------------------------------------------
// TBTUIViewsCommonUtils::ShowGeneralErrorNoteL
// ----------------------------------------------------------
//
void TBTUIViewsCommonUtils::ShowGeneralErrorNoteL()
    {
    //TRACE_FUNC_ENTRY

    CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog( CAknNoteDialog::EErrorTone,
                                                       CAknNoteDialog::ELongTimeout );
	if(FeatureManager::FeatureSupported( KFeatureIdCoverDisplay ))
		{  
		CleanupStack::PushL(dlg); 						
        dlg->PublishDialogL(ECmdShowBtGeneralErrorNote, KUidCoverUiCategoryBtui); // initializes cover support    
		CleanupStack::Pop(dlg); 						
		}
		
	TRACE_INFO(_L("ShowGeneralErrorNoteL "))
	dlg->ExecuteLD( R_BT_GENERAL_ERROR_NOTE );

	//TRACE_FUNC_EXIT    
    }
    
// ----------------------------------------------------------
// TBTUIViewsCommonUtils::IsAnyDeviceConnectedL()
// ----------------------------------------------------------
TBool TBTUIViewsCommonUtils::IsAnyDeviceConnectedL()
    {
	//TRACE_FUNC_ENTRY

    TInt connNum = 0;
    TInt retVal = RProperty::Get(KPropertyUidBluetoothCategory, 
      KPropertyKeyBluetoothGetPHYCount, connNum);
    TRACE_INFO((_L("GetConnectionNumber(): %d"), connNum))        
    //TRACE_FUNC_EXIT    
 	return (connNum > 0);
    }          
    
// -----------------------------------------------
// CBTUIPairedDevicesView::TurnBTOnIfNeededL
// -----------------------------------------------
TInt TBTUIViewsCommonUtils::TurnBTOnIfNeededL(CBTEngSettings* aBtEngSettings,TBool aCoverDisplayEnabled )
    {
	//TRACE_FUNC

    TInt errorCode = KErrNone;    
    TBTPowerStateValue previousPowerMode;
       
    errorCode=aBtEngSettings->GetPowerState(previousPowerMode); 
    if(errorCode)    
    	return errorCode;    
         
	if( previousPowerMode==EBTPowerOn )
		return KErrNone;
	
	if ( previousPowerMode==EBTPowerOff ) // power OFF -> ON
        {
        TRACE_INFO(_L("[BTUI]\t power OFF -> ON"))

        TInt offlineModeOff = 0;    // possible values are 0 and 1
        TInt activationEnabled = 0; // possible values are 0 and 1
               
        CRepository* repository = CRepository::NewL(KCRUidCoreApplicationUIs);
        CleanupStack::PushL(repository);
        repository->Get(KCoreAppUIsNetworkConnectionAllowed, offlineModeOff);
		// Closing connection:
		CleanupStack::PopAndDestroy(repository);        
		repository=NULL;
		
        repository = CRepository::NewL(KCRUidBluetoothEngine);
		// Check activation enabled
		CleanupStack::PushL(repository);
		repository->Get(KBTEnabledInOffline, activationEnabled);
		// Closing connection:
		CleanupStack::PopAndDestroy(repository);
        
                
        TRACE_INFO((_L("power OFF -> ON, offlineModeOff = %d"), offlineModeOff))
        TRACE_INFO((_L("power OFF -> ON, activationEnabled = %d"), activationEnabled))

           // Choose user interaction
        //
        if ( !offlineModeOff && !activationEnabled )
            {
            // Load a string, create and show a note
            //
            HBufC* stringHolder = StringLoader::LoadLC( R_BT_OFFLINE_DISABLED );
            CAknInformationNote* note = new (ELeave) CAknInformationNote();

			if(aCoverDisplayEnabled)
				{  
				CleanupStack::PushL(note); 						
	    		note->PublishDialogL(ECmdBtuiShowBtOfflineDisableNote, KUidCoverUiCategoryBtui); // initializes cover support    
				CleanupStack::Pop(note); 						
				}
			
    		note->ExecuteLD( *stringHolder );
    		CleanupStack::PopAndDestroy(stringHolder); //stringHolder
            }
        else
            {
            // Choose query
            //
            TInt resource = 0;

        	TInt coverUiDlgId = 0;
				
            if ( offlineModeOff )
                {
                resource = R_BT_IS_OFF_QUERY;
				if(aCoverDisplayEnabled)
					{  
        	    	coverUiDlgId = ECmdBtuiShowBtIsOffDlg;
					}
                }
            else
                {
                resource = R_BT_ACTIVATE_IN_OFFLINE_QUERY;
				if(aCoverDisplayEnabled)
					{  
        	    	coverUiDlgId = ECmdBtuiShowBtActivateInOfflineDlg;
					}
                }

            // Create and show query
            //
            CAknQueryDialog* dlg = CAknQueryDialog::NewL();

   			if(aCoverDisplayEnabled)
				{  
				CleanupStack::PushL(dlg); 						
    			dlg->PublishDialogL(coverUiDlgId, KUidCoverUiCategoryBtui); // initializes cover support    
				CleanupStack::Pop(dlg); 						
				}
				
            TInt keypress = dlg->ExecuteLD( resource );

            if( keypress )
                {
                // Switch BT ON
                //
                errorCode=aBtEngSettings->SetPowerState(EBTPowerOn ) ;
                if (errorCode)
                    {
                    ShowGeneralErrorNoteL();                   
                    }
                }  else     
            	{
            	errorCode=KErrCancel;               
            	}       
					
                
            }

        }    
	
    return errorCode;
    }    
    


