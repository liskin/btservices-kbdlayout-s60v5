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
* Description: 
*     
*
*/


#ifndef BTMCOBSERVER_H
#define BTMCOBSERVER_H

#include <e32base.h>

class MBtmcObserver
    {
    public:

        virtual void SlcIndicateL(TBool aSlc) = 0;
        
        virtual void SendProtocolDataL(const TDesC8& aData) = 0;
        
    };

#endif // BTMCOBSERVER_H
