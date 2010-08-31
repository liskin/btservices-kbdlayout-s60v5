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


#include "btsendserviceprovider.h"
#include <btserviceapi.h>
#include "btsssendlisthandler.h"
#include <btfeaturescfg.h>  // For Enterprise security settings
#include <btnotif.h>    // For Enterprise security notifier




CBtSendServiceProvider* CBtSendServiceProvider::NewL()
    {
    CBtSendServiceProvider* self = new( ELeave ) CBtSendServiceProvider();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
    
void CBtSendServiceProvider::ConstructL()
    {
    // Check features setting - if not completely enabled with enterprise settings then we are not allowed to send anything.
    // Fail here at the first fence, otherwise there are a number of other areas that need to be considered.
    if(BluetoothFeatures::EnterpriseEnablementL() != BluetoothFeatures::EEnabled)
        {
        RNotifier notifier;
        User::LeaveIfError(notifier.Connect());
        CleanupClosePushL(notifier);
        User::LeaveIfError(notifier.StartNotifier(KBTEnterpriseItSecurityInfoNotifierUid, KNullDesC8));
        CleanupStack::PopAndDestroy(&notifier);
        
        //@TODO in old code we don't continue further what needs to be done here
        }
    iBTSendingService = CBTServiceAPI::NewL();
    iConverter = CBTSSSendListHandler::NewL();
    }

CBtSendServiceProvider::CBtSendServiceProvider()
    {

    }


CBtSendServiceProvider::~CBtSendServiceProvider()
    {
    if(iConverter)
        {
        delete iConverter;
        iConverter= NULL;
        }
    if(iBTSendingService)
        {
        delete iBTSendingService;
        iBTSendingService = NULL;
        }
    }


int CBtSendServiceProvider::send(const QList<QVariant> &arguments)
    {
    TInt error = KErrNone; 
    
    CBTServiceParameterList* parameterList = NULL;
    TRAP(error,parameterList= CBTServiceParameterList::NewL());
    if (error)
        return error;
    iConverter->ConvertList( &arguments, parameterList);
    
    delete iConverter;
    iConverter = NULL;

    TRAP(error,iBTSendingService->StartSynchronousServiceL( EBTSendingService, parameterList ));
    if(error)
        return error;
    return error;
    }
