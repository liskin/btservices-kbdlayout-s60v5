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
*       CBtMtmUi class
*
*/


// INCLUDE FILES
#include "btmtmui.h"
#include "btmtmuidebug.h"

//#include <obexmtmuilayer.h>
#ifdef NO101APPDEPFIXES
#include <MuiuMsvProgressReporterOperation.h>
#endif  //NO101APPDEPFIXES

#ifdef NO101APPDEPFIXES_NEW 
#include <app/btcmtm.h>
#endif //NO101APPDEPFIXES_NEW

#include <mtmuidef.hrh>
#include <mtclreg.h>
#include <msvuids.h>
#include <msvids.h>
#include <obexconstants.h>


// CONSTANTS
#ifdef NO101APPDEPFIXES
const TInt KBtMtmUiToFromFieldBuffer     = 80;
const TInt KBtMtmUiConnectionTimeout     = 20000000;
const TInt KBtMtmUiConnectionPutTimeout  = 0;
const TInt KBtMtmUiObexPort              = 1;
const TInt KBtMtmUiAddressMaxLength      = 3;
#endif  //NO101APPDEPFIXES

// ENUMS
enum TBtMtmAsyncCmds
    {
    EBtMtmCmdSend
    };

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CBtMtmUi::CBtMtmUi( CBaseMtm& aBaseMtm, CRegisteredMtmDll& aRegisteredMtmDll )
	:	CBaseMtmUi( aBaseMtm, aRegisteredMtmDll )
	{
    FLOG( _L( "[CBtMtmUi] CBtMtmUi: CBtMtmUi\t" ) );
	}

// Two-phased constructor.
CBtMtmUi* CBtMtmUi::NewL( CBaseMtm& aBaseMtm, CRegisteredMtmDll& aRegisteredMtmDll )
	{
    FLOG( _L( "[CBtMtmUi] CBtMtmUi: NewL\t" ) );
	CBtMtmUi* self = new( ELeave ) CBtMtmUi( aBaseMtm, aRegisteredMtmDll );
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
	}

// Destructor
CBtMtmUi::~CBtMtmUi()
	{
    FLOG( _L( "[CBtMtmUi] CBtMtmUi: ~CBtMtmUi\t" ) );
	delete iClientRegistry;
	delete iDialog;
	}

// Symbian OS default constructor can leave.
void CBtMtmUi::ConstructL()
	{
    FLOG( _L( "[CBtMtmUi] CBtMtmUi: ConstructL\t" ) );
    CBaseMtmUi::ConstructL();
    iDialog = CObexUtilsDialog::NewL(this);
	}

// ---------------------------------------------------------
// GetResourceFileName(...)
// return progress status.
// ---------------------------------------------------------
//
void CBtMtmUi::GetResourceFileName( TFileName& aFileName ) const
	{ 
	FLOG( _L( "[CBtMtmUi] CBtMtmUi: GetResourceFileName\t" ) );
	(void)aFileName;
	//todo: This function should be removed it is using avkon resource file.
	}

// ---------------------------------------------------------
// QueryCapability(TUid aCapability, TInt& aResponse)
// return capability of mtm.
// ---------------------------------------------------------
//
TInt CBtMtmUi::QueryCapability(TUid aCapability, TInt& aResponse)
	{ 
    // Querying of menu availablility is done in syni.
	FLOG( _L( "[CBtMtmUi] CBtMtmUi: QueryCapability\t" ) );
	if( aCapability.iUid == KUidMsvMtmUiQueryCanPrintMsgValue )
		{
		aResponse = ETrue;
		return KErrNone;
		}
	return CBaseMtmUi::QueryCapability( aCapability, aResponse );
	}

// ---------------------------------------------------------
// InvokeSyncFunctionL(...)
// Invoke sync -function from BaseMTM.
// ---------------------------------------------------------
//
void CBtMtmUi::InvokeSyncFunctionL( TInt aFunctionId, 
                                   const CMsvEntrySelection& aSelection, 
                                   TDes8& aParameter )
	{
	FLOG( _L( "[CBtMtmUi] CBtMtmUi: InvokeSyncFunctionL\t" ) );
	CBaseMtmUi::InvokeSyncFunctionL( aFunctionId, aSelection, aParameter );
	}


// ---------------------------------------------------------
// CreateL(...)
// Entry creation forbidden.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::CreateL( const TMsvEntry& /*aEntry*/,
                                 CMsvEntry& /*aParent*/, 
                                 TRequestStatus& /*aStatus*/ )
	{
    FLOG( _L( "[CBtMtmUi] CBtMtmUi: CreateL\t" ) );
    // Entries created through utility functions.
	User::Leave( KErrNotSupported );
    // Return value is needed for removing compilation errors
    return NULL;
	}

// ---------------------------------------------------------
// OpenL(TRequestStatus& aStatus)
// handles opening entry.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::OpenL( TRequestStatus& aStatus )
	{
	FLOG( _L( "[CBtMtmUi] CBtMtmUi: OpenL\t" ) );
	const TMsvEntry& context = iBaseMtm.Entry().Entry();
	const TUid type( context.iType );
	if( ( type == KUidMsvMessageEntry ) &&  
         ( ( context.Parent() == KMsvSentEntryIdValue )
         ||( context.Parent() == KMsvGlobalInBoxIndexEntryIdValue ) ) )
        {
		// Message is in the sent folder, so can't be edited
		return ViewL( aStatus );
        }
	return EditL( aStatus );
	}

// ---------------------------------------------------------
// OpenL(...)
// handles opening entry using selection.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::OpenL( TRequestStatus& aStatus, 
                               const CMsvEntrySelection& aSelection )
	{
	iBaseMtm.Entry().SetEntryL( aSelection.At(0) );
	return OpenL( aStatus );
	}

// ---------------------------------------------------------
// CloseL(TRequestStatus& aStatus)
// Closes entry.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::CloseL( TRequestStatus& aStatus )
	{
	FLOG( _L( "[CBtMtmUi] CBtMtmUi: CloseL\t" ) );
	CMsvCompletedOperation* op = CMsvCompletedOperation::NewL( Session(), 
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
CMsvOperation* CBtMtmUi::CloseL(
    TRequestStatus& aStatus, 
    const CMsvEntrySelection& aSelection)
	{
	iBaseMtm.Entry().SetEntryL( aSelection.At(0) );
	return CloseL( aStatus );
	}

// ---------------------------------------------------------
// EditL(...)
// Handles message sending using selection.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::EditL( TRequestStatus& aStatus, const CMsvEntrySelection& aSelection )
	{
    FLOG( _L( "[BtMtmUi] CBtMtmUi: EditL 1 \t" ) );
	iBaseMtm.Entry().SetEntryL( aSelection.At(0) );
	return EditL( aStatus );
	}

// ---------------------------------------------------------
// EditL(TRequestStatus& aStatus)
// Handles message sending over Bluetooth.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::EditL( TRequestStatus& aStatus )
	{
	FLOG( _L( "[BtMtmUi] CBtMtmUi: EditL 2 \t" ) );
	switch( iBaseMtm.Entry().Entry().iType.iUid )
		{
	    case KUidMsvMessageEntryValue:
		    {
		    if( iBaseMtm.Entry().Entry().Parent() != KMsvDraftEntryId &&
		       iBaseMtm.Entry().Entry().Parent() != KMsvGlobalOutBoxIndexEntryId )
			    {
			    //   Edit/"use" entries in the Inbox
			    return LaunchEditorApplicationL( aStatus, iBaseMtm.Entry().Session() );   
			    }
#ifdef NO101APPDEPFIXES
		    else
			    {
			    TInt resourceId;
                HBufC* password = HBufC::NewL(1);
                CleanupStack::PushL( password );  // 1st push
                BaseMtm().LoadMessageL();

                TBTDevAddr address;

                CBtClientMtm::SBtcCmdSendParams sendparams;
			    sendparams.iTimeouts.iConnectTimeout    = KBtMtmUiConnectionTimeout;
			    sendparams.iTimeouts.iPutTimeout        = KBtMtmUiConnectionPutTimeout;
			    sendparams.iRemoteObexPort              = KBtMtmUiObexPort;
			    sendparams.iConnectPassword             = password;

                iDiscovery = CBTEngDiscovery::NewL(this);
				CBTDevice* iDevice = CBTDevice::NewL( );
				

                if ( iDiscovery->SearchRemoteDevice( iDevice ) == KErrNone )
                    {
                    TInt retVal=0;
				    iWaiter.Start();
				    
				    if ( iState ==KErrNone)
				        {
					    address = iDevice->BDAddr();
				        FLOG( _L( "[BtMtmUi] CBtMtmUi:SearchRemoteChannelL \t" ) );
                        retVal = iDiscovery->RemoteProtocolChannelQuery(address, TUUID(KBTSdpObjectPush));
                        if(retVal == KErrNone)
                            {
                            iWaiter.Start();
                            }
                        }
                    if ( retVal != KErrNone ||iState  != KErrNone )
                        {
                        //resourceId = R_BT_DEV_NOT_AVAIL;                        
						//TObexUtilsUiLayer::ShowInformationNoteL( resourceId );
                        //todo: Need to use Localized string.
                        _LIT(KText, "Cannot establish Bluetooth connection");
                        iDialog->ShowInformationNoteL(KText);
                        CleanupStack::PopAndDestroy(3); // BtDevice, BtConnection, password
                        return CMsvCompletedOperation::NewL(
                             Session(), 
                             Type(), 
                             KNullDesC8, 
                             KMsvLocalServiceIndexEntryId, 
                            aStatus );
                        }                     
                    sendparams.iRemoteObexPort = iClientChannel;

			        TPtrC8 ptr8 = address.Des();
                    TBuf16<KBtMtmUiAddressMaxLength> convertedaddress16;//48 bits
                    // Bt address conversion
			        TInt i;
                    for( i=0; i<3; i++ )
                        {
			            TUint16 word = ptr8[( i*2 )+1];
			            word = ( TUint16 )( word << 8 );
			            word = ( TUint16 )( word + ptr8[i*2] );
			            convertedaddress16.Append( &word, 1 );
			            }

			        BaseMtm().AddAddresseeL( convertedaddress16 );
                    }
                else
                    {
                    CleanupStack::PopAndDestroy(3); // BtDevice, BTConnection, password
				    FLOG( _L( "[BTMTMUI] CBtMtmUi:CMsvCompletedOperation NewL\t" ) );
                    return CMsvCompletedOperation::NewL(
                        Session(), 
                        Type(), 
                        KNullDesC8, 
                        KMsvLocalServiceIndexEntryId, 
                        aStatus );
                    }

			    CMsvEntrySelection* sel = new( ELeave ) CMsvEntrySelection();
			    CleanupStack::PushL( sel );					// 4th push
			    sel->AppendL( BaseMtm().Entry().EntryId() );
			    
			    CMsvOperationWait* waiter = CMsvOperationWait::NewLC();	// 5th push
                
                TBuf<KBtMtmUiToFromFieldBuffer> toFrom;
                resourceId = R_BT_SEND_OUTBOX_SENDING;                
				TObexUtilsUiLayer::ReadResourceL( toFrom, resourceId );
                                
                BaseMtm().SaveMessageL();
			    
                TMsvEntry newTEntry( iBaseMtm.Entry().Entry() );
			    newTEntry.iDetails.Set( toFrom );
                newTEntry.SetVisible( ETrue );
                newTEntry.SetInPreparation( ETrue );

                iBaseMtm.Entry().ChangeL( newTEntry );

			    CMsvEntry* entry = 
                    BaseMtm().Session().GetEntryL( KMsvDraftEntryId );

                CleanupStack::PushL( entry );  // 6th push

			    CMsvOperation* moveOp = entry->MoveL(
                    *sel,
                    KMsvGlobalOutBoxIndexEntryId, 
                    waiter->iStatus );

                CleanupStack::PopAndDestroy( entry );

			    waiter->Start();
			    CActiveScheduler::Start();
			    delete moveOp;

                CMsvProgressReporterOperation* reporter 
                    = CMsvProgressReporterOperation::NewL( Session(), aStatus );
			    CleanupStack::PushL( reporter );  // 6th push

   			    TPckgBuf<CBtClientMtm::SBtcCmdSendParams> paramBuf( sendparams );
			    
  			    CMsvOperation* op = BaseMtm().InvokeAsyncFunctionL(
                    EBtMtmCmdSend, 
                    *sel,
                    paramBuf, 
                    reporter->RequestStatus() );

                // ownership of op transfered to reporter
		        reporter->SetOperationL( op ); 

                aStatus = KRequestPending;

                CleanupStack::Pop( reporter );
                CleanupStack::PopAndDestroy(3);  // waiter, sel,  password
                return reporter;
			    }
#endif  //NO101APPDEPFIXES
		    }
	    case KUidMsvServiceEntryValue:
	    case KUidMsvAttachmentEntryValue:
	    case KUidMsvFolderEntryValue:
            {
		    User::Leave( KErrNotSupported );
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
    aStatus = KRequestPending;
	return op;
	}

// ---------------------------------------------------------
// ViewL(TRequestStatus& aStatus)
// Handles viewing of received object.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::ViewL( TRequestStatus& aStatus )
	{
	FLOG( _L( "[CBtMtmUi] CBtMtmUi: ViewL\t" ) );
	return LaunchEditorApplicationL( aStatus, iBaseMtm.Entry().Session() );
	}

// ---------------------------------------------------------
// ViewL(...)
// Handles viewing of received object using selection.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::ViewL( TRequestStatus& aStatus, 
                               const CMsvEntrySelection& aSelection )
	{
    FLOG( _L( "[CBtMtmUi] CBtMtmUi: ViewL 1\t" ) );
	iBaseMtm.Entry().SetEntryL( aSelection.At(0) );
	return ViewL( aStatus );
	}

// ---------------------------------------------------------
// CopyFromL(...)
// Not supported.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::CopyFromL( const CMsvEntrySelection& /*aSelection*/, 
                                   TMsvId /*aTargetId*/, 
                                   TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
    // Return value is needed for removing compilation errors
    return NULL;
	}

// ---------------------------------------------------------
// MoveFromL(...)
// Not supported.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::MoveFromL( const CMsvEntrySelection& /*aSelection*/, 
                                   TMsvId /*aTargetId*/, 
                                   TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
    // Return value is needed for removing compilation errors
    return NULL;
	}

// ---------------------------------------------------------
// MoveToL(...)
// Not supported.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::MoveToL( const CMsvEntrySelection& /*aSelection*/, 
                                 TRequestStatus& /*aStatus*/)
	{
    User::Leave( KErrNotSupported );
    // Return value is needed for removing compilation errors
    return NULL;
	}

// ---------------------------------------------------------
// CopyToL(...)
// Not supported.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::CopyToL( const CMsvEntrySelection& /*aSelection*/, 
                                 TRequestStatus& /*aStatus*/)
	{
    User::Leave( KErrNotSupported );
    // Return value is needed for removing compilation errors
    return NULL;
	}

// ---------------------------------------------------------
// DisplayProgressSummary(const TDesC8& aProgress) const
// calls leaving DisplayProgressSummaryL -function and
// traps the error.
// ---------------------------------------------------------
//
TInt CBtMtmUi::DisplayProgressSummary( const TDesC8& aProgress ) const
    {
    TInt retVal = KErrNone;
    TRAP( retVal, retVal = DisplayProgressSummaryL( aProgress ) );
    return retVal;
    }

// ---------------------------------------------------------
// DisplayProgressSummary(const TDesC8& aProgress) const
// calls leaving DisplayProgressSummaryL -function and
// traps the error.
// ---------------------------------------------------------
//
TInt CBtMtmUi::DisplayProgressSummaryL( const TDesC8& aProgress ) const
    {
    #ifndef NO101APPDEPFIXES_NEW
    (void) aProgress;
    #endif //NO101APPDEPFIXES_NEW

    #ifdef NO101APPDEPFIXES_NEW
    FLOG( _L( "[CBtMtmUi] CBtMtmUi:DisplayProgressSummaryL\t" ) );
    TInt resourceId;
    if( ( !aProgress.Length() ) || ( aProgress.Size() == sizeof( TMsvLocalOperationProgress ) ) )
        {
        // Probably a CMsvCompletedOperation
        return KErrCancel;
        }
	TPckgBuf<TObexMtmProgress> paramPack;
	paramPack.Copy( aProgress );
	TObexMtmProgress& progress = paramPack();

	const TObexMtmProgress::TSendState progressType	= progress.iSendState;
	const TInt error = progress.iError;
    if ( error == KErrInUse )
		{
        //resourceId = R_BT_DEV_NOT_AVAIL;
		//TObexUtilsUiLayer::ShowInformationNoteL( resourceId );
        //todo: Need to use Localized string.
        _LIT(KText, "Cannot establish Bluetooth connection");
        iDialog->ShowInformationNoteL(KText);

	    return KErrNone;
		}
    
	switch( progressType )
		{
	    case TObexMtmProgress::EDisconnected:
            {
            FLOG( _L( "[CBtMtmUi] CBtMtmUi:DisplayProgressSummaryL: EDisconnected\t" ) );
            // Allowed to send again.
            //resourceId = R_BT_DATA_SENT;
			//TObexUtilsUiLayer::ShowInformationNoteL( resourceId );
            //todo: Need to use Localized string.
            _LIT(KText, "Message sent!");
            iDialog->ShowInformationNoteL(KText);

		    break;
            }
        case TObexMtmProgress::ESendError:
            {
            FLOG( _L( "[CBtMtmUi] CBtMtmUi:DisplayProgressSummaryL: ESendError\t" ) );
            if( error == KErrIrObexClientNoDevicesFound )
                {
                //resourceId = R_BT_DEV_NOT_AVAIL;
				//TObexUtilsUiLayer::ShowInformationNoteL( resourceId );
                //todo: Need to use Localized string.
                _LIT(KText, "Cannot establish Bluetooth connection");
                iDialog->ShowInformationNoteL(KText);

                }
            else
                {
                //resourceId = R_BT_FAILED_TO_SEND;
				//TObexUtilsUiLayer::ShowErrorNoteL( resourceId );
                //todo: Need to use Localized string.
                _LIT(KText, "Failed to send message");
                iDialog->ShowErrorNoteL(KText);

                }
            break;
            }
        case TObexMtmProgress::ESendComplete:
            {
            FLOG( _L( "[CBtMtmUi] CBtMtmUi:DisplayProgressSummaryL: ESendComplete\t" ) );
            break;
            }
        case TObexMtmProgress::EInitialise:
	    case TObexMtmProgress::EConnect:
	    case TObexMtmProgress::EConnectAttemptComplete:
	    case TObexMtmProgress::ESendObject:
	    case TObexMtmProgress::ESendNextObject:
            {
            FLOG( _L( "[CBtMtmUi] CBtMtmUi:DisplayProgressSummaryL: ESendObject\t" ) );
            return KErrCancel;
            }
        case TObexMtmProgress::EUserCancelled:
            {
            FLOG( _L( "[CBtMtmUi] CBtMtmUi:DisplayProgressSummaryL: EUserCancelled\t" ) );
            return KErrCancel;
            }
	    default:
            {
            FTRACE( FPrint(_L( "[CBtMtmUi] CBtMtmUi:DisplayProgressSummaryL: Default\t %d" ), progressType ) );
            return KErrCancel;
            }
        }
  #endif //NO101APPDEPFIXES_NEW
	return KErrNone;
	}


// ---------------------------------------------------------
// GetProgress(...)
// return progress status.
// ---------------------------------------------------------
//
TInt CBtMtmUi::GetProgress( const TDesC8& aProgress,
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
	
   #ifdef  NO101APPDEPFIXES_NEW
    TInt resourceId;    

	FLOG( _L( "[CBtMtmUi] CBtMtmUi: GetProgress\t" ) );	
	TPckgBuf<TObexMtmProgress> paramPack;
	paramPack.Copy( aProgress );
	TObexMtmProgress& progress = paramPack();
    const TObexMtmProgress::TSendState progressType	= progress.iSendState;
	aTotalEntryCount	= progress.iTotalEntryCount;
	aEntriesDone		= progress.iEntriesDone;
	aCurrentEntrySize	= progress.iCurrentEntrySize; 
	aCurrentBytesTrans	= progress.iCurrentBytesTrans;

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
            resourceId = R_BT_CONNECTING;
			TRAPD( retVal, TObexUtilsUiLayer::ReadResourceL( aReturnString, resourceId ) );
						retVal=retVal; //avoid warning
            // Leave causes progress note to be empty. Not fatal
		    break;
            }
	    case TObexMtmProgress::ESendObject:
	    case TObexMtmProgress::ESendNextObject:
	    case TObexMtmProgress::ESendComplete:
            {
            resourceId = R_BT_SENDING_DATA;
			TRAPD( retVal, TObexUtilsUiLayer::ReadResourceL( aReturnString, resourceId ) );
            // Leave causes progress note to be empty. Not fatal
            retVal=retVal; //avoid warning
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
CMsvOperation* CBtMtmUi::LaunchEditorApplicationL( TRequestStatus& aStatus, 
                                                  CMsvSession& aSession)
	{
	FLOG( _L( "[CommonMtmUi] CBtMtmUi: LaunchEditorApplicationL\t" ) );
	(void)aStatus;
	(void)aSession;
	//CMsvEntry* message;
	//message = &iBaseMtm.Entry();
	//todo: need to replace this 
	//return TObexUtilsUiLayer::LaunchEditorApplicationOperationL( aSession, message, aStatus );
	return NULL;
	}

// ---------------------------------------------------------
// GetClientMtmLC(TUid aMtmType)
// Handles MTM registration.
// ---------------------------------------------------------
//
CBaseMtm* CBtMtmUi::GetClientMtmLC( TUid aMtmType )
	{
    FLOG( _L( "[CBtMtmUi] CBtMtmUi: GetClientMtmLC\t" ) );
	if( !iClientRegistry )
        {
		iClientRegistry = CClientMtmRegistry::NewL( Session() );
        }
	CBaseMtm* mtm = iClientRegistry->NewMtmL( aMtmType );
	CleanupStack::PushL( mtm );
	return mtm;
	}

// ---------------------------------------------------------
// DeleteFromL(TUid aMtmType)
// passes entry selection to Base for deletion.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::DeleteFromL( const CMsvEntrySelection& aSelection, 
                                     TRequestStatus& aStatus )
	{
	return CBaseMtmUi::DeleteFromL( aSelection, aStatus );
	}

// ---------------------------------------------------------
// DeleteServiceL(...)
// has no service, just complete it.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::DeleteServiceL( const TMsvEntry& /*aService*/, 
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
CMsvOperation* CBtMtmUi::ReplyL( TMsvId /*aDestination*/, 
                                TMsvPartList /*aPartlist*/, 
                                TRequestStatus& /*aCompletionStatus*/ )
	{
	User::Leave( KErrNotSupported );
    // Return value is needed for removing compilation errors
    return NULL;
	}

// ---------------------------------------------------------
// ForwardL(...)
// Forward is forbidden.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::ForwardL( TMsvId /*aDestination*/, 
                                  TMsvPartList /*aPartList*/, 
                                  TRequestStatus& /*aCompletionStatus*/ )
	{
	User::Leave( KErrNotSupported );
    // Return value is needed for removing compilation errors
    return NULL;
	}

// ---------------------------------------------------------
// CancelL(...)
// No MS way of sending a message from the outbox, so no need to cancel.
// Just complete it.
// ---------------------------------------------------------
//
CMsvOperation* CBtMtmUi::CancelL( TRequestStatus& aStatus, 
                                 const CMsvEntrySelection& /*aSelection*/)
    { 
	return CMsvCompletedOperation::NewL(
        Session(), 
        Type(), 
        KNullDesC8, 
        KMsvLocalServiceIndexEntryId, 
        aStatus, 
        KErrNone);
    }
    
// -----------------------------------------------------------------------------
//  CBtMtmUi::ServiceSearchComplete
// -----------------------------------------------------------------------------
//	
void CBtMtmUi::ServiceSearchComplete( const RSdpRecHandleArray& /*aResult*/, 
                                         TUint /*aTotalRecordsCount*/, TInt /*aErr */)
    {
    }

// -----------------------------------------------------------------------------
// CBtMtmUi::AttributeSearchComplete(
// -----------------------------------------------------------------------------
//	
void CBtMtmUi::AttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, 
                                           const RSdpResultArray& aAttr, 
                                           TInt aErr )
    {
   iState=aErr;     
    if (aErr==KErrNone)
        {            
        RSdpResultArray results=aAttr;    
        iDiscovery->ParseRfcommChannel(results,iClientChannel);                        
        }   
    iWaiter.AsyncStop();
    }
// -----------------------------------------------------------------------------
// CBtMtmUi::ServiceAttributeSearchComplete
// -----------------------------------------------------------------------------
//	
void CBtMtmUi::ServiceAttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, 
                                                          const RSdpResultArray& /*aAttr*/, 
                                                          TInt /*aErr*/ )
    {
    }
// -----------------------------------------------------------------------------
// CBtMtmUi::DeviceSearchComplete
// -----------------------------------------------------------------------------
//	
void CBtMtmUi::DeviceSearchComplete( CBTDevice* /*aDevice*/, TInt aErr )
    {
    iState=aErr;        
    iWaiter.AsyncStop();
    }            

void CBtMtmUi::DialogDismissed(TInt /*aButtonId*/)
    {
    
    }

// End of File
