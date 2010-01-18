// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
 @file
 @internalComponent
 @released
*/

#include "avrcpfragmenter.h"
#include "controlcommand.h"
#include <e32debug.h>

// Maximum size of data in a fragment response
static const int KFragmentDataLength = KAVCMaxVendorDependentPayload - KAVRCPMinVendorDependentResponseLen;


CAVRCPFragmenter* CAVRCPFragmenter::NewL()
	{
	CAVRCPFragmenter* self = new (ELeave) CAVRCPFragmenter();
	self->Construct();
	return self;
	}

TBool CAVRCPFragmenter::InFragmentedState()
	{
	return iInFragmentedState;
	}

void CAVRCPFragmenter::Reset()
	{
	iStart = KAVRCPMinVendorDependentResponseLen;
	iInFragmentedState = EFalse;
	}

void CAVRCPFragmenter::AssignPayload(const RBuf8& aPayload)
	{
	// Assign the allocated memory containing the reponse to
	// this class, allowing the caller to Close() aPayload.
	iPayload.Close();
	iPayload.Assign(aPayload);
	iInFragmentedState = ETrue;
	}

TPtr8 CAVRCPFragmenter::GetNextFragmentHeader()
	{
	if (! InFragmentedState())
		return TPtr8(0,0,0);
	
	// Fragmentation byte is 2nd byte of response; poke this
	// and set the length correctly
	if (iStart == KAVRCPMinVendorDependentResponseLen)
		{
		iPayload[1] = EFragmentStart;
		iPayload[2] = (KFragmentDataLength >> 8) & 0xff;
		iPayload[3] = (KFragmentDataLength) & 0xff;
		}
	else if (LastFragment())
		{
		iPayload[1] = EFragmentEnd;
		TInt length = iPayload.Length() - iStart;
		iPayload[2] = (length >> 8) & 0xff;
		iPayload[3] = (length) & 0xff;
		}
	else
		{
		iPayload[1] = EFragmentContinue;
		iPayload[2] = (KFragmentDataLength >> 8) & 0xff;
		iPayload[3] = (KFragmentDataLength) & 0xff;
		}
	
	// Return the first 4 bytes, which contain the PDU id,
	// fragmentation status and parameter length (2 bytes)
	return iPayload.MidTPtr(0, KAVRCPMinVendorDependentResponseLen);
	}

TPtr8 CAVRCPFragmenter::GetNextFragment()
	{
	if (! InFragmentedState())
		return TPtr8(0,0,0);
	
	// Sigh. MidTPtr() PANICs if the 2nd argument is too large.
	// Other APIs (e.g. Java, STL) just return to end of string.
	TInt fragmentSize = KFragmentDataLength;
	if (LastFragment())
		fragmentSize = iPayload.Length() - iStart;
	
	TPtr8 fragment = iPayload.MidTPtr(iStart, fragmentSize);
	
	if (LastFragment())
		Reset();
	else
		iStart += fragmentSize;
	return fragment;
	}

TBool CAVRCPFragmenter::LastFragment()
	{
	if (iStart + KFragmentDataLength > iPayload.Length())
		return ETrue;
	else
		return EFalse;
	}

TInt CAVRCPFragmenter::GetPDU()
	{
	return iPayload[0];
	}

void CAVRCPFragmenter::Construct()
	{
	Reset();
	}

