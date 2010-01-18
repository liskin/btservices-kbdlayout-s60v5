/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Extended Phone RemCon Handler 
*
*/


#include "btmcprotdatabuf.h"
#include "debug.h"

void TBtmcProtDataBuf::Reset()
    {
    iBuf.Zero();
    }

TInt TBtmcProtDataBuf::NextCommand(TDes8& aText)
    {
    TRACE_FUNC
	
	const TChar KCharReturn = '\r';

    TInt pos = iBuf.Locate(KCharReturn);

	if (pos == KErrNotFound)
	    {
		return KErrNotFound;
	    }
	aText.Copy(iBuf.Left(pos));
	iBuf.Delete(0, pos + 1);
	iBuf.TrimLeft();
	return KErrNone;
    }

// -------------------------------------------------------------------------------
// TBTAspATRemoteDataBuffer::Append
//
// Appends AT command characters to buffer. If buffer overflows, it is emptied
// and function returns KErrOverflow.
//-------------------------------------------------------------------------------
//
TInt TBtmcProtDataBuf::Append(const TDesC8& aText)
    {
    TRACE_FUNC
    if ( (aText.Length() + iBuf.Length()) > iBuf.MaxLength() )
	    {
		iBuf = KNullDesC8;
		return KErrOverflow;
	    }
	iBuf.Append(aText);
	return KErrNone;
    }

// End of file
