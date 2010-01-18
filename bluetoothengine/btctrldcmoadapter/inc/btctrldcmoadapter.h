//
/*
 * ==============================================================================
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
 * Description:
 * 
 * ==============================================================================
 */

#ifndef BTCTRLDCMOADAPTER_H
#define BTCTRLDCMOADAPTER_H

#include <dcmointerface.h>
#include <dcmoconst.h>


// An implementation of the CDCMOInterface definition
class CBluetoothCtrlDcmoAdapter : public CDCMOInterface
	{
public:
	// Standardised safe construction which leaves nothing the cleanup stack.
	static CBluetoothCtrlDcmoAdapter* NewL(TAny* aInitParams);

	// Destructor	
	~CBluetoothCtrlDcmoAdapter();

private:
	// Implementation of CDCMOInterface
    virtual TDCMOStatus  GetDCMOPluginIntAttributeValueL(TDCMONode aId, TInt& aValue); 
    virtual TDCMOStatus  GetDCMOPluginStrAttributeValueL(TDCMONode aId, TDes& aStrValue);
    virtual TDCMOStatus  SetDCMOPluginIntAttributeValueL(TDCMONode aId, TInt aValue);
    virtual TDCMOStatus  SetDCMOPluginStrAttributeValueL(TDCMONode aId, const TDes& aStrValue);
    virtual void GetLocalizedNameL( HBufC*& aLocName );
	
private:
    
    enum TBluetoothDcmoPanic
        {
        EPropertyBufferTooSmall = 0,
        EDescriptionBufferTooSmall = 1
        };
    
	// Construction
    CBluetoothCtrlDcmoAdapter(TAny* aInitParams);
	static TDCMOStatus MapFeatureControlError(TInt aErrorCode);
	static void Panic(TBluetoothDcmoPanic aPanic);

private:
	// Parameters taken from client
	CDCMOInterface::TDCMOInterfaceInitParams* iInitParams;
	};  

#endif // BTCTRLDCMOADAPTER_H
