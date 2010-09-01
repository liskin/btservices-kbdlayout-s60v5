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
* Description:  Find and load plugin
*
*/


#include <aknview.h>
#include "btuipluginman.h"
#include "debug.h"  

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTUIPluginMan* CBTUIPluginMan::NewL( CAknViewAppUi* aAppUi )
    {
    CBTUIPluginMan* self = new( ELeave ) CBTUIPluginMan( aAppUi );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTUIPluginMan::~CBTUIPluginMan()
    {
    TRACE_FUNC_ENTRY
    
    iPluginInfoArray.ResetAndDestroy();
    iPluginInfoArray.Close();
    iPluginArray.Reset();//Do not destroy - Plugins are owned by iAppUi

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Check if availability of some plug-in
// ---------------------------------------------------------------------------
//
TBool CBTUIPluginMan::IsPluginAvaiable(TBTDeviceClass aDeviceClassInfo)
	{
	for( TInt i=0; i<iPluginArray.Count(); i++ )
		{
		TBTDeviceClass devClass = iPluginArray[i]->GetCOD(); 
		if( devClass.MajorDeviceClass() == aDeviceClassInfo.MajorDeviceClass() && 
			devClass.MinorDeviceClass() == aDeviceClassInfo.MinorDeviceClass() )
			return ETrue;
		}
	return EFalse;
	}

// ---------------------------------------------------------------------------
// Get the setting view from PluginArrary based on the COD 
// ---------------------------------------------------------------------------
//
CAknView* CBTUIPluginMan::GetSettingViewL(TBTDevice& aDevice)
	{
	for (TInt ii = 0; ii < iPluginArray.Count(); ii++ )
		{
		TBTDeviceClass tmpCOD = iPluginArray[ii]->GetCOD();
		//Currently one device - one view - one plugin, 
		//in future maybe other device properties will decide on getting the view. 
		if(aDevice.iDeviceClass.MajorDeviceClass() == tmpCOD.MajorDeviceClass() &&
				aDevice.iDeviceClass.MinorDeviceClass() == 	tmpCOD.MinorDeviceClass())
			{
			return (CAknView*)iPluginArray[ii];
			}
		}
	
	return NULL;	
	}

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTUIPluginMan::CBTUIPluginMan( CAknViewAppUi* aAppUi )
    : iAppUi(aAppUi)
    {

    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTUIPluginMan::ConstructL()
    {   
    LoadPluginsL();
    }

// ---------------------------------------------------------------------------
// Load ECom plug-ins
// ---------------------------------------------------------------------------
//
void CBTUIPluginMan::LoadPluginsL()
	{
	TRACE_FUNC_ENTRY
	
	REComSession::ListImplementationsL( KCBtuiPluginInterfaceUid, iPluginInfoArray );
	TInt count = iPluginInfoArray.Count();
	TRACE_INFO( ( _L( "[BTUI]\t CBTUIPluginMan::LoadSettingPluginL() PluginArray count=%d" ), count) )

    iPluginArray.Reset();
	for ( TUint ii = 0 ; ii < count; ++ii )
		{
		CImplementationInformation* impl = iPluginInfoArray[ii];
		//Only append the plugins which datatype() is for HID devices. 
	    if(impl->DataType().Length())
	    	{
	    	TLex8 lex( impl->DataType() );
	    	TUint major = 0, minor = 0;
	    	TInt err1 = lex.Val(major, EHex);
	    	lex.SkipSpace();
	    	TInt err2 = lex.Val(minor, EHex);
	    	if( !err1 && !err2)
	    	    {
	    	    TRACE_INFO( ( _L( "[BTUI]\t CBTUIPluginMan::LoadSettingPluginL() MajorCOD=%d MinorCOD=%d" ), major, minor) )
 	            TBTDeviceClass cod(0x00, major, minor);
	    	                
	    	    CBtuiPluginInterface* plugin = NULL;
	    	    TRAPD( ret, plugin = CBtuiPluginInterface::NewL( impl->ImplementationUid() ) );
	    	    if( !ret )
	    	        {
	    	        CleanupStack::PushL(plugin);
	    	        
	    	        // Transfer the ownership to AppUi
	    	        iAppUi->AddViewL( (CAknView*)plugin ); 
	    	        plugin->SetCOD(cod);
	    	        iPluginArray.AppendL(plugin);
	    	        CleanupStack::Pop(plugin);              
	    	        }	    	        
	    	    }
	        }
		}
	
	TRACE_FUNC_EXIT
	}

