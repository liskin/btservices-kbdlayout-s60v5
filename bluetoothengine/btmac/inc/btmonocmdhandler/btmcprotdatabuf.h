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
* Description:
*
*/


#ifndef BTMCPROTDATABUF_H
#define BTMCPROTDATABUF_H

#include "atcodec.h"

NONSHARABLE_CLASS(TBtmcProtDataBuf)
    {
    public:

	    void Reset();

        /**
        * Extracts the first AT command in buffer.
        * @param aText decriptor where the command will be stored.
        * @return Error code.
        */
	    TInt NextCommand(TDes8& aData);

        /**
        * Appends the given command to buffer.
        * @param aText Text to be appended.
        * @return Error code.
        */
        TInt Append(const TDesC8& aData);

    private:

	    TBuf8<KMaxATSize> iBuf;
    };


#endif // BTMCPROTDATABUF_H

// End of File
 