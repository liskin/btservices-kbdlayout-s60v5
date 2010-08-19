/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BT HFP AT extension API
*
*/


// INCLUDE FILES
#include "HfpAtCmdHandler.h"
#include "debug.h"

enum TActiveRequestId
    {
    EHandleCommandRequest = 1,
    EReceiveEventRequest,
    };

CHFPAtCmdHandler* CHFPAtCmdHandler::NewL(MATExtObserver& aObserver)
	{
    CHFPAtCmdHandler* self = new (ELeave) CHFPAtCmdHandler(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
	}

CHFPAtCmdHandler::~CHFPAtCmdHandler()
	{
    TRACE_FUNC
    delete iCommander;
    
    DeletePluginHandlers();

    delete iEcomListen;
    iEcomListen = NULL;

       if ( iATExtClient.Handle() )
        {
        iATExtClient.Close();
        }
   iCmdBuffer.Close();
   iSystemReply.Close();
   delete iRecvBuffer;
   delete iReplyBuffer;
}

void CHFPAtCmdHandler::HandleCommand(const TDesC8& aAT, const TDesC8& aReply)
    {
    TRACE_INFO((_L8("default reply '%S'"), &aReply))
    iCmdBuffer.Copy(aAT);
    iReplyBuffer->Zero();
    if (aReply.Length())
        {
        iReplyBuffer->Copy(aReply);
        iSystemReply.Copy(aReply);
        } 
    iATExtClient.HandleCommand(iCommander->iStatus, 
            iCmdBuffer, *iReplyBuffer, iRemainingReplyLengthPckg, iReplyTypePckg);
    iCommander->GoActive(); 
    }

void CHFPAtCmdHandler::RequestCompletedL(CBtmcActive& aActive, TInt aErr)
    {
    TRACE_FUNC_ENTRY
    TInt err = aErr;
    switch (aActive.ServiceId())
        {
        case EHandleCommandRequest:
            {
            if(err == KErrNone)
                {
                if (iRemainingReplyLengthPckg())
                    {
                    TRACE_INFO((_L8("reply '%S'"), &iReplyBuffer))
                    iObserver.ATExtHandleReplyReceivedL(err, *iReplyBuffer);
                    do 
                        {
                        TRACE_INFO((_L8("iRemainingReplyLength '%d'"), iRemainingReplyLengthPckg()))
                        RBuf8 reply;
                        reply.CreateL(iRemainingReplyLengthPckg());
                        err = iATExtClient.GetNextPartOfReply(*iRecvBuffer, iRemainingReplyLengthPckg());
                        if (!err)
                            {
                            reply.Insert(0, *iRecvBuffer);
                            }
                        TRACE_INFO((_L8("reply '%S'"), &reply))
                        iObserver.ATExtHandleReplyReceivedL(err, reply);
                        reply.Close();
                        }
                    while (iRemainingReplyLengthPckg());
                    }
                else
                    {
                    TRACE_INFO((_L8("reply '%S'"), iReplyBuffer))
                    iObserver.ATExtHandleReplyReceivedL(err, *iReplyBuffer);
                    }
                }
            else
                {
                iObserver.ATExtHandleReplyReceivedL(err, iSystemReply);
                }
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

void CHFPAtCmdHandler::CancelRequest(TInt aServiceId)
    {
    switch (aServiceId)
        {
        case EHandleCommandRequest:
            {
            iATExtClient.CancelHandleCommand();
            break;
            }
        }
    }

TInt CHFPAtCmdHandler::HandleRunError(TInt /*aErr*/)
    {
    return KErrNone;
    }

CHFPAtCmdHandler::CHFPAtCmdHandler(MATExtObserver& aObserver) 
    : iObserver(aObserver), 
      iRemainingReplyLengthPckg(iRemainingReplyLength),
      iReplyTypePckg( iReplyType )
	{
	TRACE_FUNC
	}
	
_LIT8(KHFPAtCmd, "HFP");

void CHFPAtCmdHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    
    CleanupClosePushL( iATExtClient );
    TInt err = iATExtClient.Connect(EHfpATExtension, KHFPAtCmd);
    LEAVE_IF_ERROR(err)
    
    // Create the plugin handlers
    CreatePluginHandlersL();
    // Create the listeners
    CHFPAtEcomListen* ecomListen = CHFPAtEcomListen::NewLC( &iATExtClient, this );
    ecomListen->IssueRequest();
    
    CleanupStack::Pop( ecomListen );
    CleanupStack::Pop( &iATExtClient );
    iEcomListen = ecomListen;
    
    StartUrc();
    iCommander = CBtmcActive::NewL(*this, CActive::EPriorityStandard, EHandleCommandRequest);
    iCmdBuffer.CreateL(KDefaultCmdBufLength);
    iRecvBuffer = new (ELeave) TBuf8<KDefaultCmdBufLength>();
    iReplyBuffer = new (ELeave) TBuf8<KDefaultCmdBufLength>();
    iSystemReply.CreateL(KDefaultUrcBufLength);
    TRACE_FUNC_EXIT
    }
    

// ---------------------------------------------------------------------------
// Creates plugin handlers for this class
// ---------------------------------------------------------------------------
//
void CHFPAtCmdHandler::CreatePluginHandlersL()
    {
    TRACE_INFO( _L("CHFPAtCmdHandler::CreatePluginHandlersL()") );
    if ( !iATExtClient.Handle() )
        {
        TRACE_INFO(_L("CHFPAtCmdHandler::CreatePluginHandlersL() complete") );
        User::Leave( KErrGeneral );
        }

    // Next create the URC handlers
    TInt i;
    TInt numOfPlugins = iATExtClient.NumberOfPlugins();
    for ( i=0; i<numOfPlugins; i++ )
        {
        AddOneUrcHandlerL();
        }
    TRACE_INFO( _L("CHFPAtCmdHandler::CreatePluginHandlersL() complete") );
    }

// ---------------------------------------------------------------------------
// From class MHFPAtEcomListen.
// Notifies about new plugin installation
// ---------------------------------------------------------------------------
//
TInt CHFPAtCmdHandler::NotifyPluginInstallation( TUid& /*aPluginUid*/ )
    {
    TRACE_INFO( _L("CHFPAtCmdHandler::NotifyPluginInstallation()" ) );
    CHFPAtUrcHandler* urcHandler = NULL;
    TRAPD( retTrap, urcHandler=AddOneUrcHandlerL() );
    if ( retTrap != KErrNone )
        {
        TRACE_INFO( _L("CHFPAtCmdHandler::NotifyPluginInstallation() (trapped!) complete" ) );
        return retTrap;
        }
    TInt retTemp = urcHandler->IssueRequest();
    if ( retTemp != KErrNone )
        {
        TRACE_INFO( _L("CHFPAtCmdHandler::NotifyPluginInstallation() (issuerequest) complete" ) );
        return retTemp;
        }
    TUid ownerUid = urcHandler->OwnerUid();
    iATExtClient.ReportListenerUpdateReady( ownerUid, EEcomTypeInstall );

    if ( retTemp != KErrNone )
        {
        TRACE_INFO( _L("CHFPAtCmdHandler::NotifyPluginInstallation() (recreate) complete" ));
        return retTemp;
        }
    TRACE_INFO( _L("CHFPAtCmdHandler::NotifyPluginInstallation() complete" ) );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MHFPAtEcomListen.
// Notifies about existing plugin uninstallation
// ---------------------------------------------------------------------------
//
TInt CHFPAtCmdHandler::NotifyPluginUninstallation( TUid& aPluginUid )
    {
    TRACE_INFO( _L("CHFPAtCmdHandler::NotifyPluginUninstallation()" ) );
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=count-1; i>=0; i-- )
        {
        TUid ownerUid = iUrcHandlers[i]->OwnerUid();
        if ( ownerUid == aPluginUid )
            {
            delete iUrcHandlers[i];
            iUrcHandlers.Remove( i );
            iATExtClient.ReportListenerUpdateReady( ownerUid,
                                                 EEcomTypeUninstall );
            }
        }

    TRACE_INFO( _L("CHFPAtCmdHandler::NotifyPluginUninstallation() complete" ) );
    return KErrNone;
    }
    
// ---------------------------------------------------------------------------
// Instantiates one URC message handling class instance and adds it to the URC
// message handler array
// ---------------------------------------------------------------------------
//
CHFPAtUrcHandler* CHFPAtCmdHandler::AddOneUrcHandlerL()
    {
    TRACE_INFO( _L("CHFPAtCmdHandler::AddOneUrcHandlerL()") );
    CHFPAtUrcHandler* urcHandler = CHFPAtUrcHandler::NewLC( &iATExtClient,
                                                            iObserver );
    iUrcHandlers.AppendL( urcHandler );
    CleanupStack::Pop( urcHandler );
    TRACE_INFO( _L("CHFPAtCmdHandler::AddOneUrcHandlerL() complete") );
    return urcHandler;
    }
    
// ---------------------------------------------------------------------------
// Starts URC message handling
// ---------------------------------------------------------------------------
//
TInt CHFPAtCmdHandler::StartUrc()
    {
    TRACE_INFO( _L("CHFPAtCmdHandler::StartUrc()") );
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        TInt retTemp = iUrcHandlers[i]->IssueRequest();
        if ( retTemp!=KErrNone && retTemp!=KErrNotReady )
            {
            TRACE_INFO( _L("CHFPAtCmdHandler::StartUrc() (ERROR) complete") );
            return retTemp;
            }
        }
    TRACE_INFO( _L("CHFPAtCmdHandler::StartUrc() complete") );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops URC message handling
// ---------------------------------------------------------------------------
//
TInt CHFPAtCmdHandler::StopUrc()
    {
    TRACE_INFO( _L("CHFPAtCmdHandler::StopUrc()") );
    TInt i;
    TInt retVal = KErrNone;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        retVal = iUrcHandlers[i]->Stop();
        }
    TRACE_INFO( _L("CHFPAtCmdHandler::StopUrc() complete") );
    return retVal;
    }

// ---------------------------------------------------------------------------
// Deletes all instantiated URC message handlers
// ---------------------------------------------------------------------------
//
void CHFPAtCmdHandler::DeletePluginHandlers()
    {
    TRACE_INFO( _L("CHFPAtCmdHandler::DeletePluginHandlers()") );
    TInt i;
    TInt count = iUrcHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        delete iUrcHandlers[i];
        iUrcHandlers[i] = NULL;
        }
    iUrcHandlers.Reset();
    iUrcHandlers.Close();
    TRACE_INFO(  _L("CHFPAtCmdHandler::DeletePluginHandlers() complete") );
    }

//  End of File
