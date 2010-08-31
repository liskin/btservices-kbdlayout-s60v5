/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ObexServicemanager opcodes, panic enumerations and other 
                 definitions.
*
*/


#ifndef OBEXSERVICEMAN_H
#define OBEXSERVICEMAN_H

//  INCLUDES

#include <e32base.h>
#include "SrcsClSv.h"

/**
* Function to start server.
*/
IMPORT_C void RunServerL();

/**
* Utility to panic the client.
*/
void PanicClient(const RMessage2& aMessage, TInt aPanic);

/**
* Utility to panic server.
*/
void PanicServer(TInt aPanic);

/**
* Provides server startup parameters.
*/
class TSrcsStart
{
public:
    /**
    * Constructor
    */
    TSrcsStart(TRequestStatus& aStatus);

    /**
    * Default constructor
    * 
    * @since s60 3.2
    */
    inline TSrcsStart() {};

    /**
    * Provides startup parameters
    *
    * @since s60 3.2
    */
    TPtrC AsCommand() const;

    /**
    * Signalling from Server start.
    *
    * @since s60 3.2
    */
    IMPORT_C void SignalL();

    /**
    * Initialize from memory
    *
    * @since s60 3.2
    */
    TInt Set(const TDesC& aData);

public:     // Data
    TThreadId iId;
    TRequestStatus* iStatus;
};

/**
* Number of interfaces and length of descriptors registered by SRCS
*/
class TSrcsUsbDescriptorInfo
	{
public:
	TInt iNumInterfaces;
	TInt iLength;
	};

#endif // SRCSCLSV_H

