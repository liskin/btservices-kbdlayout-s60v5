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


#ifndef BTAUDIOSTREAMEROBSERVER_H_
#define BTAUDIOSTREAMEROBSERVER_H_

#include <StifLogger.h>
#include "btaudiostreaminputbase.h"

NONSHARABLE_CLASS( CBtAudioStreamerObserver ) : public MBTAudioErrorObserver
{
public:	
    static CBtAudioStreamerObserver* NewL( CStifLogger* aLog );
	~CBtAudioStreamerObserver();
	
	virtual TInt Error(const TInt aError);
	
private:
    CBtAudioStreamerObserver( CStifLogger* aLog );
    void ConstructL();
    
private:
    CStifLogger* iLog;
    
};

#endif /*BTAUDIOSTREAMEROBSERVER_H_*/
