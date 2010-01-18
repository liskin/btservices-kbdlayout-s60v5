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

#ifndef AVRCPFRAGMENTER_H
#define AVRCPFRAGMENTER_H

/**
@file
@internalComponent
@released
*/

#define KAVRCPMinVendorDependentResponseLen 4

#include <e32base.h>

NONSHARABLE_CLASS(CAVRCPFragmenter) : public CBase
	{
public:
	static CAVRCPFragmenter* NewL();
	void Reset();
	void AssignPayload(const RBuf8& aPayload);
	TBool InFragmentedState();
	TPtr8 GetNextFragmentHeader();
	TPtr8 GetNextFragment();
	TBool LastFragment();
	TInt GetPDU();
	~CAVRCPFragmenter() { iPayload.Close(); }
		
private:
	CAVRCPFragmenter() {}
	void Construct();
	
private:
	RBuf8 iPayload;
	TBool iInFragmentedState;
	TInt iStart;
	};

#endif // AVRCPFRAGMENTER_H
