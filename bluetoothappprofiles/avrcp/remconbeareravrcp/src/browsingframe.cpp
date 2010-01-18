// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "browsingframe.h"


/** Verifies the length of a frame is correct.  Checking:
1) Header is present
2) Data length indicated is present

This does not semantically check that the data expected for
the PDU ID is present.

@param aFrame The frame to verify
@leave KErrCorrupt if the length of the frame is incorrect
*/
void AvrcpBrowsing::BrowsingFrame::VerifyFrameL(const TDesC8& aFrame)
	{
	if((aFrame.Length() < AvrcpBrowsing::KHeaderLength) ||
	  (aFrame.Length() < (AvrcpBrowsing::KHeaderLength + ParamLength(aFrame))))
		{
		User::Leave(KErrCorrupt);
		}
	}


/** Retreives the PDU ID from this frame.

@pre The integrity of the frame must have been verified (@see 
	 BrowsingFrame::VerifyFrameL) as the length is not checked
	 before retrieving the PDU ID.
@param aFrame The frame to retrieve the PDU ID for.
@return The PDU ID of aFrame
*/
AvrcpBrowsing::TPduId AvrcpBrowsing::BrowsingFrame::PduId(const TDesC8& aFrame)
	{
	return aFrame[0];
	}

/** Retreives the parameter length from this frame.

@pre The integrity of the frame must have been verified (@see 
	 BrowsingFrame::VerifyFrameL) as the length is not checked
	 before retrieving the parameter length.
@param aFrame The frame to retrieve the parameter length for.
@return The parameter length of aFrame
*/
TInt AvrcpBrowsing::BrowsingFrame::ParamLength(const TDesC8& aFrame)
	{
	TInt paramLength = 0;
	paramLength += aFrame[2];
	paramLength += ((TUint)aFrame[1]) << 8;
	
	return paramLength;
	}

/** Retreives the payload from this frame.

@pre The integrity of the frame must have been verified (@see 
	 BrowsingFrame::VerifyFrameL) as the length is not checked
	 before retrieving the payload.
@param aFrame The frame to retrieve the payload for.
@param aPayload On return the payload of aFrame
*/
void AvrcpBrowsing::BrowsingFrame::Payload(const TDesC8& aFrame, TPtrC8& aPayload)
	{
	aPayload.Set(aFrame.Mid(AvrcpBrowsing::KHeaderLength, AvrcpBrowsing::BrowsingFrame::ParamLength(aFrame)));
	}

