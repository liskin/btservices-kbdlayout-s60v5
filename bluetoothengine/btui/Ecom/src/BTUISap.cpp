/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This module is created for Bluetooth SIM Access Profile 
*				  feature variant.
*
*/

#include <aknnotewrappers.h>
#include <aknradiobuttonsettingpage.h>
#include <BtuiViewResources.rsg>		// Compiled resource ids
#include <BTSapDomainPSKeys.h>	
#include <centralrepository.h> 
#include <AknMediatorFacade.h>
#include <btengconnman.h>       // for disconnect type and connection observer
#include <btengsettings.h>
#include <btengdevman.h>
#include <secondarydisplay/BtuiSecondaryDisplayAPI.h>
#include <btfeaturescfg.h>					// For EnterpriseEnablementL()
#include <btnotif.h>
#include <utf.h>
#include <bluetoothuiutil.h>
#include "btdevmodel.h"

#include "debug.h"
#include "BTUIMainView.h"

// ----------------------------------------------------
// CBTUIMainView::SetSapStatusL
// ----------------------------------------------------
//
void CBTUIMainView::SetSapStatusL( TBTSapMode aValue )
	{
	TRACE_FUNC_ENTRY

	TInt previousSapMode = GetSapStatusL();	
	
	if(previousSapMode==EBTSapEnabled)
		previousSapMode=ETrue;
		
	TBool legalToChange = ETrue;

	if ( ( previousSapMode ) && ( !aValue ) ) // SAP Enabled -> Disabled
		{
		// Check active SAP connections before query
		TInt connStatus = EBTSapNotConnected;
        RProperty::Get(KPSUidBluetoothSapConnectionState, KBTSapConnectionState, connStatus);
						
		if(connStatus == EBTSapConnected) 
			{
			//Get name of the currently connected SAP device

			TBTDeviceName connectedSap;			
			TInt error = GetConnectedSapNameL( connectedSap ); 

			if ( error )
				{
				TBTUIViewsCommonUtils::ShowGeneralErrorNoteL();
				}
			else
				{

				// Create confirmation query
				RBuf stringholder;
				CleanupClosePushL( stringholder );
				BluetoothUiUtil::LoadResourceAndSubstringL( stringholder, 
				        R_BT_DISCONNECT_FROM, connectedSap, 0 );
				CAknQueryDialog* dlg = CAknQueryDialog::NewL();

				if(iCoverDisplayEnabled)
					{
					CleanupStack::PushL(dlg); 						
				    dlg->PublishDialogL(ECmdShowDisconnectQuery, KUidCoverUiCategoryBtui); // initializes cover support    
					CAknMediatorFacade* covercl = AknMediatorFacade(dlg); // uses MOP, so control provided 
					if (covercl) // returns null if __COVER_DISPLAY is not defined
					    {	    
					    covercl->BufStream() << BTDeviceNameConverter::ToUTF8L(connectedSap);// takes copy so consts are ok too
					    covercl->BufStream().CommitL(); // no more data to send so commit buf
					    }
					CleanupStack::Pop(dlg);
					}
					
				TInt keypress = dlg->ExecuteLD( R_BT_DISCONNECT_FROM_QUERY, stringholder );

				CleanupStack::PopAndDestroy(&stringholder);	// stringholder

				if( keypress )	// User has accepted the dialog
					{
					legalToChange = ETrue;
					}
				else 
					{
					legalToChange = EFalse;
					}
				}
			}
		}
			
	else if ( ( !previousSapMode ) && ( aValue ) ) // SAP Disabled	-> Enabled 
		{
		
		//if Bluetooth is off
        TBTPowerStateValue powerState;
        iBtEngSettings->GetPowerState(powerState);				
		if(powerState!=EBTPowerOn)
			{
			CAknNoteDialog* dlg2 = new (ELeave) CAknNoteDialog( CAknNoteDialog::EConfirmationTone,
																   CAknNoteDialog::ENoTimeout/*ELongTimeout*/ );
		if(iCoverDisplayEnabled)
			{
			CleanupStack::PushL(dlg2); 						
			dlg2->PublishDialogL(ECmdShowBtSapEnableNote, KUidCoverUiCategoryBtui); // initializes cover support    
			CleanupStack::Pop(dlg2); 						
			}

			dlg2->ExecuteLD( R_BT_SAP_ENABLE_NOTE );
			}
	
		}		

	// Switch sapmode
	//
	if ( legalToChange )
		{		
        CRepository* cenRep = NULL;
        cenRep = CRepository::NewL( KCRUidBTEngPrivateSettings );
        CleanupStack::PushL(cenRep);                         
        TInt err = cenRep->Set( KBTSapEnabled, aValue );        
        CleanupStack::PopAndDestroy(cenRep);        	
		}		
	TRACE_FUNC_EXIT
	}

// ----------------------------------------------------------
// CBTUIMainView::AskSapModeDlgL
// ----------------------------------------------------------
//
void CBTUIMainView::AskSapModeDlgL()
	{
	TRACE_FUNC_ENTRY

	if ( BluetoothFeatures::EnterpriseEnablementL() == BluetoothFeatures::EDataProfilesDisabled )
		{
		RNotifier notifier;
        User::LeaveIfError(notifier.Connect());
        CleanupClosePushL(notifier);
		User::LeaveIfError(notifier.StartNotifier(KBTEnterpriseItSecurityInfoNotifierUid, KNullDesC8 ));
		CleanupStack::PopAndDestroy(&notifier);
		}
	else
		{
		CDesCArrayFlat* items = iCoeEnv->ReadDesCArrayResourceL( R_BT_SAP_MODES );
		CleanupStack::PushL(items);	
		
		TInt currentItem = KErrNotFound;
		if(GetSapStatusL() == EBTSapEnabled )
			{
				currentItem=0;
			}
		else
			{
				currentItem=1;
			
			}
		CAknRadioButtonSettingPage* dlg = new ( ELeave )CAknRadioButtonSettingPage(
		   R_BT_SAP_SETTING_PAGE, currentItem, items );
		if( dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ))
			{
			if(currentItem == 1)
				{
				SetSapStatusL( static_cast<TBTSapMode>(  EBTSapDisabled ) ); 
				// Cast it back (invert again)
				}			
			else
				{
				SetSapStatusL( static_cast<TBTSapMode>(  EBTSapEnabled ) ); 
				// Cast it back (invert again)			
				}
			} 
		CleanupStack::PopAndDestroy(items); 
		}
	TRACE_FUNC_EXIT
	}

// ----------------------------------------------------------
// CBTUIMainView::GetSapStatusL
// ----------------------------------------------------------
//
TInt CBTUIMainView::GetSapStatusL()
	{
	TRACE_FUNC_ENTRY	
	TInt sapStatus;
	CRepository* repository = CRepository::NewL(KCRUidBTEngPrivateSettings);
	CleanupStack::PushL(repository);            	
	repository->Get(KBTSapEnabled, sapStatus);
	CleanupStack::PopAndDestroy(repository);
	TRACE_FUNC_EXIT
	return sapStatus;	
	}
    
// ----------------------------------------------------------
// CBTUIModel::GetConnectedSapNameL
// ----------------------------------------------------------
//

TInt CBTUIMainView::GetConnectedSapNameL( TDes& aName )
	{
	TRACE_FUNC_ENTRY

    
    CBTEngConnMan* cm = CBTEngConnMan::NewL(NULL);
    CleanupStack::PushL(cm);
        
    RBTDevAddrArray devAddrArray;
    cm->GetConnectedAddresses(devAddrArray, EBTProfileSAP);
    if(devAddrArray.Count()>0)
    {
        CBTEngDevMan* devMan = CBTEngDevMan::NewL(NULL);  
         
        TBTRegistrySearch search;
        search.FindAddress(devAddrArray[0]);
        
        CBTDeviceArray* deviceArray= new (ELeave) CBTDeviceArray(devAddrArray.Count());
        //the name is in first CBTDevice in returned CBTDeviceArray. 
        devMan->GetDevices(search, deviceArray); 
        
        aName = *(CnvUtfConverter::ConvertToUnicodeFromUtf8L(deviceArray->At(0)->DeviceName()));
        
        deviceArray->ResetAndDestroy();
        delete deviceArray;
        
        delete devMan;
    }
        
    devAddrArray.Close();
    CleanupStack::PopAndDestroy(cm);               	
	TRACE_FUNC_EXIT
	return KErrNone;
	}
