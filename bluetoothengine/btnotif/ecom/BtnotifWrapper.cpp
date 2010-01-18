/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Methods for BT notifier Ecom Plugins entry point
*
*/


#include <implementationproxy.h>
#include <AknNotifierWrapper.h> // link against aknnotifierwrapper.lib
#include <btmanclient.h>
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifiers.h>  	// define btninqnotifer uid
#else
#include <btextnotifiers.h>  	// define btninqnotifer uid
#include <btextnotifierspartner.h>  	// define btninqnotifer uid
#endif
#include <btnotif.h>			// notifier id for power mode setting 

const TInt KMaxSynchReplyBufLength = 256;  //what should be?

const TUid KBTInquiryChannel            = {0x00000601}; // Channel for inquiry notifier
const TUid KBTAuthorisationChannel      = {0x00000602}; // Channel for authorisation notifier
const TUid KBTAuthenticationChannel     = {0x00000603}; // Channel for PIN query notifier
const TUid KBTObexPINChannel            = {0x00000605}; // Channel for OBEX PIN notifier
const TUid KBTPowerModeChannel          = {0x00000606}; // Channel for power mode notifier
const TUid KBTPairedDeviceSettingChannel  = {0x00000610}; // Channel for Visibility timeout notifier

const TInt KBTInquiryPriority 			= MEikSrvNotifierBase2::ENotifierPriorityVHigh;
const TInt KBTAuthorisationPriority 	= MEikSrvNotifierBase2::ENotifierPriorityVHigh;
const TInt KBTAuthenticationPriority 	= MEikSrvNotifierBase2::ENotifierPriorityVHigh;
const TInt KBTObexPinPriority 			= MEikSrvNotifierBase2::ENotifierPriorityVHigh;
const TInt KBTPowerModePriority 		= MEikSrvNotifierBase2::ENotifierPriorityVHigh;
const TInt KBTGenericInfoNotifierPriority = MEikSrvNotifierBase2::ENotifierPriorityHigh;
const TInt KBTGenericQueryNotifierPriority = MEikSrvNotifierBase2::ENotifierPriorityHigh;
const TInt KBTNumericComparisonPriority     = MEikSrvNotifierBase2::ENotifierPriorityVHigh;  // for SSP
const TInt KBTPasskeyEntryNotifierPriority = MEikSrvNotifierBase2::ENotifierPriorityVHigh;   // for SSP
const TInt KBTPairedDeviceSettingPriority = MEikSrvNotifierBase2::ENotifierPriorityVHigh;   
const TInt KBTEnterpriseItSecurityInfoNotifierPriority = MEikSrvNotifierBase2::ENotifierPriorityHigh;

const TInt KBTNotifierArrayIncrement = 8;

void CleanupArray(TAny* aArray)
    {
    CArrayPtrFlat<MEikSrvNotifierBase2>*     
        subjects=static_cast<CArrayPtrFlat<MEikSrvNotifierBase2>*>(aArray);
    TInt lastInd = subjects->Count()-1;
    for (TInt i=lastInd; i >= 0; i--)
        subjects->At(i)->Release();	    	    
    delete subjects;
    }

CArrayPtr<MEikSrvNotifierBase2>* DoCreateNotifierArrayL()
    {   	

    CArrayPtrFlat<MEikSrvNotifierBase2>* subjects=
        new (ELeave)CArrayPtrFlat<MEikSrvNotifierBase2>(KBTNotifierArrayIncrement);
    
    CleanupStack::PushL(TCleanupItem(CleanupArray, subjects));

    // Create Wrappers

    // Session owning notifier(if default implementation is enough)
                                   
    CAknCommonNotifierWrapper* master = 
        CAknCommonNotifierWrapper::NewL( KDeviceSelectionNotifierUid,
                                   KBTInquiryChannel,
                                   KBTInquiryPriority,
                                   _L("BTNotif.dll"),
                                   KMaxSynchReplyBufLength);
	  		   
    subjects->AppendL( master );
    
   // Notifiers using masters session 
    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KBTManAuthNotifierUid,
                                   			 				KBTAuthorisationChannel,
                                   			 				KBTAuthorisationPriority));
    
    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KBTPinCodeEntryNotifierUid,
                                   			 				KBTAuthenticationChannel,
                                   			 				KBTAuthenticationPriority));
                                   			 				                               
	subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KBTObexPasskeyQueryNotifierUid,
                                   			 				KBTObexPINChannel,
                                   			 				KBTObexPinPriority));
        
    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KPowerModeSettingNotifierUid,
                                   			 				KBTPowerModeChannel,
                                   			 				KBTPowerModePriority));                               			 				
                                   			 				
    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KBTGenericInfoNotifierUid,
                                   			 				KBTGenericInfoNotifierUid,
                                   			 				KBTGenericInfoNotifierPriority));

    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KBTGenericQueryNotifierUid,
                                   			 				KBTGenericQueryNotifierUid,
                                   			 				KBTGenericQueryNotifierPriority));
    
    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KPbapAuthNotifierUid,
                                   			 				KBTObexPINChannel,
                                   			 				KBTObexPinPriority));   
    
    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KBTPairedDeviceSettingNotifierUid,
                                                            KBTPairedDeviceSettingChannel,
                                                            KBTPairedDeviceSettingPriority));   

    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KBTNumericComparisonNotifierUid,
                                                            KBTNumericComparisonNotifierUid,
                                                            KBTNumericComparisonPriority));       
    
    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master,
                                                            KBTPasskeyDisplayNotifierUid,
                                                            KBTPasskeyDisplayNotifierUid,
                                                            KBTPasskeyEntryNotifierPriority)); 

    subjects->AppendL(new (ELeave) CAknNotifierWrapperLight(*master, 
                                                            KBTEnterpriseItSecurityInfoNotifierUid,
                                   			 				KBTEnterpriseItSecurityInfoNotifierUid,
                                   			 				KBTEnterpriseItSecurityInfoNotifierPriority));

    CleanupStack::Pop();	// array cleanup
    return(subjects);
    }


CArrayPtr<MEikSrvNotifierBase2>* NotifierArray()
    // old Lib main entry point
    {   	   	
    
    CArrayPtr<MEikSrvNotifierBase2>* array = 0;
    TRAPD(ignore, array = DoCreateNotifierArrayL());
    if (ignore != KErrNone) 
    	{
    	ignore = KErrNone;
    	}
    return array;
    }

const TImplementationProxy ImplementationTable[] =
	{
#ifdef __EABI__
	{{0x101FD690},(TFuncPtr)NotifierArray}
#else
	{{0x101FD690},NotifierArray}
#endif
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{   	
	
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy) ;
	return ImplementationTable;
	}

