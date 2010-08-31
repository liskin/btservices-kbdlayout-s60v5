/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
*      CIrMtmui class
*
*
*/


// INCLUDE FILES
#include "irmtmui.h"
#include "debug.h"

#ifdef NO101APPDEPFIXES_NEW
#include <app/ircmtm.h>
#endif //NO101APPDEPFIXES_NEW

#include <mtmuidef.hrh>
#include <mtclreg.h>
#include <msvuids.h>
#include <msvids.h>
#include <obexconstants.h>
#include <btnotif.h>        	// Notifier UID's
#include <featmgr.h>
#include <SecondaryDisplay/obexutilssecondarydisplayapi.h>

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CIrMtmUi::CIrMtmUi(CBaseMtm& aBaseMtm, CRegisteredMtmDll& aRegisteredMtmDll)
    :   CBaseMtmUi(aBaseMtm, aRegisteredMtmDll)
    {
    }

// Two-phased constructor.
CIrMtmUi* CIrMtmUi::NewL(
    CBaseMtm& aBaseMtm, 
    CRegisteredMtmDll& aRegisteredMtmDll)
    {
    CIrMtmUi* self=new(ELeave) CIrMtmUi(aBaseMtm, aRegisteredMtmDll);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// Destructor
CIrMtmUi::~CIrMtmUi()
    {
    delete iClientRegistry;
    }


// Symbian OS default constructor can leave.
void CIrMtmUi::ConstructL()
    {
    // --- Make sure base class correctly constructed ---
    CBaseMtmUi::ConstructL();
    //todo: Need to use localised string.
    _LIT(KConnectText, "Connecting via Infrared..");
    _LIT(KSendText, "Sending message via Infrared...");
    iConnectingText.Copy(KConnectText);
    iSendingText.Copy(KSendText);
    
    iCurrentlySending = EFalse;
    }

// ---------------------------------------------------------
// GetResourceFileName(...)
// return progress status.
// ---------------------------------------------------------
//
void CIrMtmUi::GetResourceFileName( TFileName& /*aFileName*/ ) const
    { 
    FLOG( _L( "[CIrMtmUi] CIrMtmUi: GetResourceFileName\t" ) );
    //todo: This function should be removed it is using avkon resource file.

    }

// ---------------------------------------------------------
// QueryCapability(TUid aCapability, TInt& aResponse)
// return capability of mtm.
// ---------------------------------------------------------
//
TInt CIrMtmUi::QueryCapability(TUid aCapability, TInt& aResponse)
    {  // Querying of menu availablility is done in syni.
    FLOG( _L( "[IRU] CIrMtmUi: QueryCapability\t" ) );
    if (aCapability.iUid == KUidMsvMtmUiQueryCanPrintMsgValue)
        {
        aResponse=ETrue;
        return KErrNone;
        }
    return CBaseMtmUi::QueryCapability(aCapability, aResponse);
    }

// ---------------------------------------------------------
// InvokeSyncFunctionL(...)
// Invoke sync -function from BaseMTM.
// ---------------------------------------------------------
//
void CIrMtmUi::InvokeSyncFunctionL(
    TInt aFunctionId, 
    const CMsvEntrySelection& aSelection, 
    TDes8& aParameter)
    {
    FLOG( _L( "[IRU] CIrMtmUi: InvokeSyncFunctionL\t" ) );
    CBaseMtmUi::InvokeSyncFunctionL(aFunctionId, aSelection, aParameter);
    }


// ---------------------------------------------------------
// CreateL(...)
// Entry creation forbidden.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::CreateL(
    const TMsvEntry& /*aEntry*/,
    CMsvEntry& /*aParent*/, 
    TRequestStatus& /*aStatus*/)
    {
    // Entries created through utility functions.
    User::Leave(KErrNotSupported);  
    return NULL;
    }


// ---------------------------------------------------------
// OpenL(TRequestStatus& aStatus)
// handles opening entry.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::OpenL(TRequestStatus& aStatus)
    {
    FLOG( _L( "[IRU] CIrMtmUi: OpenL\t" ) );
    const TMsvEntry& context=iBaseMtm.Entry().Entry();
    const TUid type(context.iType);

    if ( (type==KUidMsvMessageEntry) &&  
         ((context.Parent()==KMsvSentEntryIdValue)
         ||(context.Parent()==KMsvGlobalInBoxIndexEntryIdValue))
       )
        {
        // Message is in the sent folder, so can't be edited
        return ViewL(aStatus);
        }

    return EditL(aStatus);
    }

// ---------------------------------------------------------
// OpenL(...)
// handles opening entry using selection.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::OpenL(
    TRequestStatus& aStatus, 
    const CMsvEntrySelection& aSelection)
    {
    iBaseMtm.Entry().SetEntryL(aSelection.At(0));
    return OpenL(aStatus);
    }

// ---------------------------------------------------------
// CloseL(TRequestStatus& aStatus)
// Closes entry.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::CloseL(TRequestStatus& aStatus)
    {
    FLOG( _L( "[IRU] CIrMtmUi: CloseL\t" ) );
    CMsvCompletedOperation* op=CMsvCompletedOperation::NewL(
        Session(), 
        Type(), 
        KNullDesC8, 
        KMsvLocalServiceIndexEntryId, 
        aStatus);
    return op;
    }

// ---------------------------------------------------------
// CloseL(TRequestStatus& aStatus)
// Closes entry using selection.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::CloseL(
    TRequestStatus& aStatus, 
    const CMsvEntrySelection& aSelection)
    {
    iBaseMtm.Entry().SetEntryL(aSelection.At(0));
    return CloseL(aStatus);
    }

// ---------------------------------------------------------
// EditL(TRequestStatus& aStatus)
// Handles message sending.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::EditL(TRequestStatus& aStatus)
    {
    FLOG( _L( "[IRU] CIrMtmUi: EditL\t" ) );
    switch( iBaseMtm.Entry().Entry().iType.iUid )
        {
        case KUidMsvMessageEntryValue:
            {
            return LaunchEditorApplicationL( aStatus, iBaseMtm.Entry().Session() );
            /* Infrared is not supported anymore. Implementation for sending over Infrared is left for reference:
            ( add #include <MuiuMsvProgressReporterOperation.h> to the included files )
            ( add constants:
                const TInt KIrMtmUiConnectionTimeout     = 20000000;
                const TInt KIrMtmUiReceiveTimeout        = 0; )
            code starts here:

            if( iBaseMtm.Entry().Entry().Parent() != KMsvDraftEntryId &&
               iBaseMtm.Entry().Entry().Parent() != KMsvGlobalOutBoxIndexEntryId )
                {
                //   Edit/"use" entries in the Inbox
                return LaunchEditorApplicationL( aStatus, iBaseMtm.Entry().Session() );
                }
            else
                {
                FeatureManager::InitializeLibL();                    	
                if(!FeatureManager::FeatureSupported(KFeatureIdIrda))
    			    {
   					FLOG(_L("[IRSS]\t FeatMgr doesn't find IrDA, show not_supported "));
	    			RNotifier notifier;    
	    			User::LeaveIfError( notifier.Connect() );
					TBTGenericInfoNotiferParamsPckg paramsPckg;
					paramsPckg().iMessageType=EIRNotSupported;		
		            TInt status = notifier.StartNotifier(KBTGenericInfoNotifierUid, paramsPckg);
	                if ( status != KErrNone )
	                    {
	                    FTRACE(FPrint(_L("[IRSS]\t void CIrMtmUi::EditL()  ERROR: StartNotifier() failed. Code: %d "), status));
	                    }	    
	                notifier.Close();    	
	                User::Leave(KErrNone);
    	            }    	
    		    FeatureManager::UnInitializeLib();
                CIrClientMtm::STimeouts timeouts;
                timeouts.iConnectTimeout = KIrMtmUiConnectionTimeout;
                timeouts.iPutTimeout     = KIrMtmUiReceiveTimeout;

                TPckgBuf<CIrClientMtm::STimeouts> timeoutBuf( timeouts );

                CMsvEntrySelection* sel = new( ELeave ) CMsvEntrySelection();
                CleanupStack::PushL( sel );

                sel->AppendL( BaseMtm().Entry().EntryId() );

                CMsvOperationWait* waiter = CMsvOperationWait::NewLC();

                TBuf<KObexUtilsMaxCharToFromField> toFrom;
                TInt resourceId = R_IR_SEND_OUTBOX_SENDING;
                TObexUtilsUiLayer::ReadResourceL( toFrom, resourceId );

                TMsvEntry newTEntry( iBaseMtm.Entry().Entry() );
                newTEntry.iDetails.Set( toFrom );
                newTEntry.SetVisible( ETrue );
                newTEntry.SetInPreparation( ETrue );

                iBaseMtm.Entry().ChangeL( newTEntry );

                CMsvEntry* entry = 
                    BaseMtm().Session().GetEntryL( KMsvDraftEntryId );

                CleanupStack::PushL( entry );

                CMsvOperation* moveOp = entry->MoveL(
                    *sel, 
                    KMsvGlobalOutBoxIndexEntryId, 
                    waiter->iStatus );

                CleanupStack::PopAndDestroy(entry);
                
                waiter->Start();
                CActiveScheduler::Start();
                delete moveOp;

                CleanupStack::PopAndDestroy(waiter);

                TInt dialogIndex =((R_IR_SENDING_DATA & KResourceNumberMask) - KFirstResourceOffset) + KEnumStart;
                CMsvProgressReporterOperation* reporter = 
                    CMsvProgressReporterOperation::NewL( Session(), aStatus, dialogIndex, KObexUtilsCategory);
                CleanupStack::PushL( reporter );

                CMsvOperation* op = BaseMtm().InvokeAsyncFunctionL(
                    CIrClientMtm::EIrcCmdSend, 
                    *sel, 
                    timeoutBuf, 
                    reporter->RequestStatus() );

                // ownership of op transfered to reporter
                reporter->SetOperationL( op ); 
                reporter->SetTitleL(iConnectingText);
                aStatus = KRequestPending;
                
                CleanupStack::Pop(reporter);
                CleanupStack::PopAndDestroy(sel);
                // ownership of reporter transfered to caller
                return reporter; 
                }

                End of reference implementation for sending over Infrared:
                */
            }
        case KUidMsvServiceEntryValue:
        case KUidMsvAttachmentEntryValue:
        case KUidMsvFolderEntryValue:
            {
            User::Leave( KErrNotSupported );
            break;
            }
        default:
            {
            break;
            }
        }

    CMsvCompletedOperation* op = CMsvCompletedOperation::NewL(
        Session(), 
        Type(), 
        KNullDesC8, 
        KMsvLocalServiceIndexEntryId, 
        aStatus );
    FLOG( _L( "[IrMtmUi] CIrMtmUi: EditL Done\t" ) );
    
    aStatus = KRequestPending;
    return op;
    }

// ---------------------------------------------------------
// EditL(...)
// Handles message sending using selection.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::EditL( TRequestStatus& aStatus, 
                               const CMsvEntrySelection& aSelection)
    {
    iBaseMtm.Entry().SetEntryL( aSelection.At(0) );
    return EditL( aStatus );
    }

// ---------------------------------------------------------
// ViewL(TRequestStatus& aStatus)
// Handles viewing of received object.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::ViewL( TRequestStatus& aStatus )
    {
    FLOG( _L( "[IRU] CIrMtmUi: ViewL\t" ) );
    return LaunchEditorApplicationL(aStatus, iBaseMtm.Entry().Session());
    }

// ---------------------------------------------------------
// ViewL(...)
// Handles viewing of received object using selection.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::ViewL( TRequestStatus& aStatus, 
                               const CMsvEntrySelection& aSelection )
    {
    iBaseMtm.Entry().SetEntryL(aSelection.At(0));
    return ViewL(aStatus);
    }

// ---------------------------------------------------------
// CopyFromL(...)
// Not supported.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::CopyFromL( const CMsvEntrySelection& /*aSelection*/, 
                                   TMsvId /*aTargetId*/, 
                                   TRequestStatus& /*aStatus*/)
    {
    User::Leave(KErrNotSupported);
    return NULL;
    }

// ---------------------------------------------------------
// MoveFromL(...)
// Not supported.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::MoveFromL( const CMsvEntrySelection& /*aSelection*/, 
                                   TMsvId /*aTargetId*/, 
                                   TRequestStatus& /*aStatus*/)
    {
    User::Leave(KErrNotSupported);
    return NULL;
    }

// ---------------------------------------------------------
// MoveToL(...)
// Not supported.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::MoveToL( const CMsvEntrySelection& /*aSelection*/, 
                                 TRequestStatus& /*aStatus*/)
    {
    User::Leave(KErrNotSupported);
    return NULL;
    }

// ---------------------------------------------------------
// CopyToL(...)
// Not supported.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::CopyToL( const CMsvEntrySelection& /*aSelection*/, 
                                 TRequestStatus& /*aStatus*/ )
    {
    User::Leave(KErrNotSupported);
    return NULL;
    }

// ---------------------------------------------------------
// DisplayProgressSummary(const TDesC8& aProgress) const
// calls leaving DisplayProgressSummaryL -function and
// traps the error.
// ---------------------------------------------------------
//
TInt CIrMtmUi::DisplayProgressSummary( const TDesC8& aProgress ) const
    {
    FLOG( _L( "[IRU] CIrMtmUi: DisplayProgressSummary\t" ) );
    TInt retVal = KErrNone;
    TRAP( retVal, retVal = DisplayProgressSummaryL( aProgress ) );
    return retVal;
    }

// ---------------------------------------------------------
// TInt CIrMtmUi::DisplayProgressSummaryL(const TDesC8& aProgress) const
// Shows connection summary.
// ---------------------------------------------------------
//
TInt CIrMtmUi::DisplayProgressSummaryL( const TDesC8& aProgress ) const
    {
    #ifndef NO101APPDEPFIXES_NEW
    (void) aProgress;
    #endif //NO101APPDEPFIXES_NEW
    
    #ifdef NO101APPDEPFIXES_NEW
    FLOG( _L( "[IRU] CIrMtmUi: DisplayProgressSummaryL\t" ) );
    TInt resourceId;
    if( ( !aProgress.Length() ) || 
         ( aProgress.Size() == sizeof( TMsvLocalOperationProgress ) ) )
        {
        // Probably a CMsvCompletedOperation
        return KErrCancel;
        }
    TPckgBuf<TObexMtmProgress> paramPack;
    paramPack.Copy( aProgress );
    TObexMtmProgress& progress = paramPack();

    const TObexMtmProgress::TSendState progressType = progress.iSendState;
    const TInt error = progress.iError;

    if( error == KErrInUse )
        {
        resourceId = R_IR_CANNOT_ESTABLISH;
        TObexUtilsUiLayer::ShowInformationNoteL( resourceId );
        return KErrNone;
        }

    switch( progressType )
        {
        case TObexMtmProgress::EDisconnected:
            {
            // Allowed to send again.
            CONST_CAST( CIrMtmUi*, this )->iCurrentlySending = EFalse;
            resourceId = R_IR_DATA_SENT;
            TObexUtilsUiLayer::ShowInformationNoteL( resourceId );
            break;
            }
        case TObexMtmProgress::ESendError:
            {
            if( error == KErrIrObexClientNoDevicesFound )
                {
                resourceId = R_IR_CANNOT_ESTABLISH;
                TObexUtilsUiLayer::ShowInformationNoteL( resourceId );
                }
            else
                {
                resourceId = R_IR_SENDING_FAILED;
                TObexUtilsUiLayer::ShowErrorNoteL( resourceId );
                }
            break;
            }
        case TObexMtmProgress::ESendComplete:
            {
            break;
            }
        case TObexMtmProgress::EInitialise:
        case TObexMtmProgress::EConnect:
        case TObexMtmProgress::EConnectAttemptComplete:
        case TObexMtmProgress::ESendObject:
        case TObexMtmProgress::ESendNextObject:
            {
            return KErrCancel;
            }
        case TObexMtmProgress::EUserCancelled:
            {
            return KErrCancel;
            }
        default:
            {
            return KErrCancel;
            }
        }
    FLOG( _L( "[IRU] CIrMtmUi: DisplayProgressSummaryL Done\t" ) );
    #endif //NO101APPDEPFIXES_NEW
    return KErrNone;
    }

// ---------------------------------------------------------
// GetProgress(...)
// return progress status.
// ---------------------------------------------------------
//
TInt CIrMtmUi::GetProgress( const TDesC8& aProgress,
                           TBuf<EProgressStringMaxLen>& aReturnString, 
                           TInt& aTotalEntryCount, 
                           TInt& aEntriesDone,
                           TInt& aCurrentEntrySize, 
                           TInt& aCurrentBytesTrans ) const
    {
    #ifndef NO101APPDEPFIXES_NEW
    (void) aProgress;
    (void) aReturnString;
    (void) aTotalEntryCount;
    (void) aEntriesDone;
    (void) aCurrentEntrySize;
    (void) aCurrentBytesTrans;	
    #endif //NO101APPDEPFIXES_NEW
    
    #ifdef NO101APPDEPFIXES_NEW
    FLOG( _L( "[CIrMtmUi] CIrMtmUi: GetProgress\t" ) );
    TPckgBuf<TObexMtmProgress> paramPack;
    paramPack.Copy( aProgress );
    TObexMtmProgress& progress = paramPack();
    const TObexMtmProgress::TSendState progressType = progress.iSendState;
    aTotalEntryCount    = progress.iTotalEntryCount;
    aEntriesDone        = progress.iEntriesDone;
    aCurrentEntrySize   = progress.iCurrentEntrySize; 
    aCurrentBytesTrans  = progress.iCurrentBytesTrans;

    if( aTotalEntryCount > 1 ) // This is for progress bar multisending
        {
        aCurrentEntrySize = 0;
        }
    switch( progressType )
        {
        case TObexMtmProgress::ENullOp:
        case TObexMtmProgress::ESendError:
            {
            aReturnString = KNullDesC;
            break;
            }
        case TObexMtmProgress::EInitialise:
        case TObexMtmProgress::EConnect:
        case TObexMtmProgress::EConnectAttemptComplete:
            {
            aReturnString = iConnectingText;
            break;
            }
        case TObexMtmProgress::ESendObject:
        case TObexMtmProgress::ESendNextObject:
        case TObexMtmProgress::ESendComplete:
            {
            aReturnString = iSendingText;
            break;
            }
        case TObexMtmProgress::EDisconnected:
            {
            break;
            }
        default:
            {
            return KErrCancel;
            }
        }
    FLOG( _L( "[CBtMtmUi] CBtMtmUi: GetProgress Done\t" ) );
    #endif //NO101APPDEPFIXES_NEW
    return KErrNone;
    }

// ---------------------------------------------------------
// LaunchEditorApplicationL(...)
// Launch viewer for selected object. The first is object's 
// recognition(document handler).
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::LaunchEditorApplicationL( TRequestStatus& /*aStatus*/,
                                                  CMsvSession& /*aSession*/)
    {
	FLOG( _L( "[CIrMtmUi] CIrMtmUi: LaunchEditorApplicationL\t" ) );
	//CMsvEntry* message;
	//message = &iBaseMtm.Entry();
	//todo: Need to use proper API for launching editor application, now returning NULL.
	return NULL;
    }


// ---------------------------------------------------------
// GetClientMtmLC(TUid aMtmType)
// Handles MTM registration.
// ---------------------------------------------------------
//
CBaseMtm* CIrMtmUi::GetClientMtmLC( TUid aMtmType )
    {
    if (!iClientRegistry)
        {
        iClientRegistry = CClientMtmRegistry::NewL( Session() );
        }
    CBaseMtm* mtm=iClientRegistry->NewMtmL( aMtmType );
    CleanupStack::PushL( mtm );
    return mtm;
    }

// ---------------------------------------------------------
// DeleteFromL(TUid aMtmType)
// passes entry selection to Base for deletion.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::DeleteFromL( const CMsvEntrySelection& aSelection, 
                                     TRequestStatus& aStatus)

    {
    return CBaseMtmUi::DeleteFromL(aSelection, aStatus);
    }

// ---------------------------------------------------------
// DeleteServiceL(...)
// IR has no service, just complete it.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::DeleteServiceL( const TMsvEntry& /*aService*/, 
                                        TRequestStatus& aStatus)

    {
    return CMsvCompletedOperation::NewL(
        Session(), 
        Type(), 
        KNullDesC8, 
        KMsvLocalServiceIndexEntryId, 
        aStatus);
    }

// ---------------------------------------------------------
// ReplyL(...)
// Reply is forbidden.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::ReplyL( TMsvId /*aDestination*/, 
                                TMsvPartList /*aPartlist*/, 
                                TRequestStatus& /*aCompletionStatus*/)
    {
    User::Leave(KErrNotSupported);
    // Return value is needed for removing compilation errors
    return NULL;
    }

// ---------------------------------------------------------
// ForwardL(...)
// Forward is forbidden.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::ForwardL( TMsvId /*aDestination*/, 
                                  TMsvPartList /*aPartList*/, 
                                  TRequestStatus& /*aCompletionStatus*/ )
    {
    User::Leave(KErrNotSupported);
    // Return value is needed for removing compilation errors
    return NULL;
    }

// ---------------------------------------------------------
// CancelL(...)
// No MS way of sending a message from the outbox, so no need to cancel.
// Just complete it.
// ---------------------------------------------------------
//
CMsvOperation* CIrMtmUi::CancelL( TRequestStatus& aStatus, 
                                 const CMsvEntrySelection& /*aSelection*/ )
    { 
    return CMsvCompletedOperation::NewL(
        Session(), 
        Type(), 
        KNullDesC8, 
        KMsvLocalServiceIndexEntryId, 
        aStatus, 
        KErrNone);
    }

// End of File
