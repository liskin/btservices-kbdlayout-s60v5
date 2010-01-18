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
* Description:  Browsing adapter between RemCon an MPX Framework. 
*
*/

// INCLUDE FILES
#include <remconinterfaceselector.h>
#include <mpxcollectionmessagedefs.h>
#include <mpxmessagegeneraldefs.h>
#include <mpxmediageneraldefs.h>
#include <mpxsubscription.h>
#include <mpxcommandgeneraldefs.h>
#include <mpxcollectioncommanddefs.h>
#include <mpxmediacontainerdefs.h>
#include <e32cmn.h>
#include <remconmediaerror.h>

#include "btrccBrowsingAdapter.h"
#include "debug.h"

#ifdef BTRCCTEST_MPXCOLLECTIONSTUB
#include <stubsrv/mpxcollectionstub.h>
#endif


// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTRCCBrowsingAdapter* CBTRCCBrowsingAdapter::NewL(CRemConInterfaceSelector& aInterfaceSelector)
    {
    CBTRCCBrowsingAdapter* self = new (ELeave) CBTRCCBrowsingAdapter();
    CleanupStack::PushL(self);
    self->ConstructL(aInterfaceSelector);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::CBTRCCBrowsingAdapter
// C++ constructor.
// -----------------------------------------------------------------------------
//
CBTRCCBrowsingAdapter::CBTRCCBrowsingAdapter()
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::ConstructL(CRemConInterfaceSelector& aInterfaceSelector)
    {
    TRACE_FUNC
    iMediaBrowseTarget = CRemConDatabaseAwareMediaBrowseTarget::NewL(
                             aInterfaceSelector, *this, *this, EFalse,
                             iMlInterface, iNpInterface, iCookie);

#ifdef BTRCCTEST_MPXCOLLECTIONSTUB
    iCollectionUtility = CMPXCollectionUtilityStubImpl::NewL(this, KMcModeDefault);    
#else
    iCollectionUtility = MMPXCollectionUtility::NewL(this, KMcModeDefault);
#endif

    iCurrentOpenLOp = EOpenLOpOpen;
    iCollectionUtility->Collection().OpenL(EMPXOpenDefault);
    }

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTRCCBrowsingAdapter::~CBTRCCBrowsingAdapter()
    {
    TRACE_FUNC
    delete iMediaBrowseTarget;
    delete iCollectionUtility; 
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::MrcdamlboGetFolderListing
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::MrcdamlboGetFolderListing(TRemConFolderScope aScope,
        TUint /*aStartItem*/, TUint /*aEndItem*/)
    {
    TRACE_FUNC

    // Check that the aScope is in filesystem scope. 
    if (aScope != EBrowseFolder)
        {
        // Not supported. 
        return; 
        }

    // ToDo: Check that we don't have any other operation ongoing. 
    // If we do, put the new operation in queue  

    TInt err = KErrNone;
    CMPXCollectionPath* path = NULL;
    TRAP(err, path = iCollectionUtility->Collection().PathL())
    
    if(!err)
        {
        TRAP(err, path->SelectAllL())
        }
    if(!err)
        {
        RArray<TMPXAttribute> attrs;
        attrs.Append( KMPXMediaGeneralId );
        attrs.Append( KMPXMediaGeneralTitle );
        TRAP(err, iCollectionUtility->Collection().MediaL(*path, attrs.Array()))        
        }
    if(!err)
        {
        iCurrentMediaLOp = EMediaLOpFolderListing;
        }
    TRACE_INFO((_L("Get folder listing %d"), err))


/*    CMPXCollectionPath* path = iCollectionUtility->Collection().PathL();
    CleanupStack::PushL( path );
    if ( path->Levels() == KVcxMpxLevelVideos 
            && path->Count() > aIndex
            && aIndex >= 0 )
        {
        // Marks the item that will be handled
        path->SelectL( aIndex );

        RArray<TMPXAttribute> dummy; // No need to set this parameter
        CleanupClosePushL( dummy );
        iCollectionUtility->Collection().MediaL( *path, dummy.Array() );
        CleanupStack::PopAndDestroy( &dummy );
        }
    CleanupStack::PopAndDestroy( path ); */
    
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::MrcdamlboGetItem
// This function is called to request for indicidual item's properties.  
// -----------------------------------------------------------------------------
//
TInt CBTRCCBrowsingAdapter::MrcdamlboGetItem(TRemConFolderScope /*aScope*/,
        const TRemConItemUid& /*aItemId*/, 
        TMediaAttributeIter& /*aIter*/, 
        TUint16 /*aMediaLibraryStateCookie*/)
    {
    TRACE_FUNC
    
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::MrcdamlboFolderUp
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::MrcdamlboFolderUp(TUint16 aMediaLibraryStateCookie)
    {   
    TRACE_FUNC
    if(aMediaLibraryStateCookie == iCookie)
        {
        TRAPD( err, iCollectionUtility->Collection().BackL() );
        if(err)
            {
            }
        // --> HandleOpenL will called
        }
    else
        {
        
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::MrcdamlboFolderDown
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::MrcdamlboFolderDown(const TRemConItemUid& aFolder,
        TUint16 aMediaLibraryStateCookie)
    {
    TRACE_FUNC
    if(aMediaLibraryStateCookie == iCookie)
        {
        TInt err = KErrNone;
        CMPXCollectionPath* path = NULL;
        TRAP( err, path = iCollectionUtility->Collection().PathL() );
        if(!err)
            {
            TMPXItemId index = path->IdOfIndex((TInt)aFolder);
            if(index != KMPXInvalidItemId)
                {
                TRAP( err, iCollectionUtility->Collection().OpenL(index) );
                // --> HandleOpenL will called.
                }
            }
        }
    else
        {
        
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::MrcdamlboGetPath
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::MrcdamlboGetPath(RPointerArray<HBufC8>& /*aPath*/)
    {
    TRACE_FUNC   
    TInt err = KErrNone;
    CMPXCollectionPath* browsePath = NULL;
    CMPXCollectionPath* container = NULL;
    
    TRAP( err, browsePath = iCollectionUtility->Collection().PathL() );
    
    // Split path into levels until root level is reached.
    TRAP( err, container = browsePath->ContainerPathL() ); 
    
    // Store the path information into received refToPath.
    // --> do something
    
    // Path information stored, inform RemCon
    //iMlInterface->MrcdamlbGetPathResult(TUint aItemCount, TUint16 aMediaLibraryStateCookie, TInt aResult);
    
    delete browsePath;
    delete container;
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::MrcdanpboGetFolderListing
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::MrcdanpboGetFolderListing(TUint /*aStartItem*/, TUint /*aEndItem*/)
    {
    TRACE_FUNC
    // Use the current path to 
//    iCollectionUtility->Collection().MediaL(path, aAttrs, )
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::MrcdanpboGetItem
// -----------------------------------------------------------------------------
//
TInt CBTRCCBrowsingAdapter::MrcdanpboGetItem(const TRemConItemUid& /*aItemId*/, TMediaAttributeIter& /*aIter*/, TUint16 /*aMediaLibraryStateCookie*/)
    {
    TRACE_FUNC
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::MrcdamlboSearch
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::MrcdamlboSearch(const TDesC8& /*aSearch*/)
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::HandleCollectionMessage
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::HandleCollectionMessage(CMPXMessage* aMsg, TInt /*aErr*/)
    {
    TRACE_FUNC
    if(iMlInterface && aMsg)
        {
        if( aMsg->IsSupported(KMPXMessageMediaGeneralCategory) &&
            aMsg->IsSupported(KMPXMessageChangeEventType) )
            {
            TInt event( aMsg->ValueTObjectL<TInt>( KMPXMessageMediaGeneralCategory ) );
            TInt op( aMsg->ValueTObjectL<TInt>( KMPXMessageChangeEventType ) );

            if( event == EMPXCollection )
                {
                if( op == EMPXItemDeleted ||
                    op == EMPXItemInserted ||
                    op == EMPXItemModified )
                    {
                    // how cookie is really composed??
                    iCookie = op;
                    iMlInterface->MrcdamlbMediaLibraryStateChange(iCookie);
                    }
                }
            }
        }
    
    /* Example from mpxdeletehelper.cpp
    if( aMessage &&
        aMessage->IsSupported(KMPXMessageGeneralEvent) &&
        aMessage->IsSupported(KMPXMessageGeneralType) )
        {
        TInt event( aMessage->ValueTObjectL<TInt>( KMPXMessageGeneralEvent ) );
        TInt op( aMessage->ValueTObjectL<TInt>( KMPXMessageGeneralType ) );

        MPX_DEBUG3( "CMPXDeleteHelper::HandleCollectionMessageL event = %d, type = %d",
                    event, op );

        if( event == TMPXCollectionMessage::EBroadcastEvent )
            {
            if( op == EMcMsgFormatStart ||
                op == EMcMsgDiskRemoved ||
                op == EMcMsgUSBMassStorageStart ||
                op == EMcMsgUSBMTPStart )
                {
                iCancelled = ETrue;
                Cancel();
                }
            }

        }*/
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::HandleOpenL
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::HandleOpenL(const CMPXMedia& /*aEntries*/, TInt /*aIndex*/, TBool /*aComplete*/, TInt aError)
    {
    // This is called when a folder has been opened. We could store a path here. 
    TRACE_FUNC
    if(!aError)
        {
        switch(iCurrentOpenLOp)
            {
            case EOpenLOpOpen:
                break;
            default:
                break;
            }
        }    
    iCurrentOpenLOp = EOpenLOpIdle;
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::HandleOpenL
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::HandleOpenL(const CMPXCollectionPlaylist& /*aPlaylist*/,TInt /*aError*/)
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::HandleCommandComplete
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::HandleCommandComplete(CMPXCommand* /*aCommandResult*/, TInt /*aError*/)
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::HandleCollectionMediaL
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::HandleCollectionMediaL(const CMPXMedia& aMedia, TInt aError)
    {
    TRACE_FUNC 
    RArray<TRemConItem> folderListing; 
    TInt cookie = 0; 
    if(!aError)
        {
        switch(iCurrentMediaLOp)
            {
            case EMediaLOpFolderListing:
                {
                TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, EMediaLOpFolderListing")))
                
                TMPXAttribute mediaArrayAttr( KMPXMediaIdContainer, EMPXMediaArrayContents );
                // Check if there is more than one item in aMedia
                if( aMedia.IsSupported( mediaArrayAttr ) )
                    {
                    TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, media array is supported.")))
                    const CMPXMediaArray* mediaArray = aMedia.ValueCObjectL<CMPXMediaArray>( mediaArrayAttr ); 
                    //User::LeaveIfNull( const_cast<CMPXMediaArray*>( mediaArray ));
                    
                    TInt error = KErrNone;
                    if(mediaArray)
                        {
                        for(TInt i = 0; i < mediaArray->Count() ; ++i)
                            {
                            CMPXMedia* ptrToElement = mediaArray->AtL(i); 
                            TMPXAttribute uidAttr(KMPXMediaIdGeneral, EMPXMediaGeneralId);
                            TMPXAttribute typeAttr(KMPXMediaIdGeneral, EMPXMediaGeneralType);
                            if( ptrToElement->IsSupported(uidAttr) && ptrToElement->IsSupported(typeAttr) ) 
                                {
                                TRemConItem item;  
                                const TUid& id = ptrToElement->ValueTObjectL<TUid>(uidAttr);
                                item.iUid = id.iUid;  
                                const TMPXGeneralType& type = ptrToElement->ValueTObjectL<TMPXGeneralType>(typeAttr);                               
                                
                                TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, id %d"), id))
                                TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, type %d"), type))                                
                                
                                if(type == EMPXGroup)
                                    {
                                    item.iType = ERemConFolderItem;
                                    }
                                else if(type == EMPXItem)
                                    {
                                    item.iType = ERemConMediaItem; 
                                    }
                                else
                                    {
                                    continue; // skip appending if the type wasn't folder or item.  
                                    }
                                TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, Append data")))
                                folderListing.Append(item); 
                                }
                            else
                                {
                                TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, Not supported, item index %d."), i))
                                }
                            }
                        TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, pass media to RemCon")))
                        
                        }
                    else
                        {
                        TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, No media!")))
                        error = KErrMediaBrowseInvalidOffset;
                        }
                    iMlInterface->MrcdamlbFolderListing(folderListing.Array(), cookie, error);
                    }
                else
                    {
                    TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, Array Content Not Supported.")))
                    // "Single" elements should handled separately ??
                    }                
                }
                break;
            }
        }
    else
        {
        TRACE_INFO((_L("CBTRCCBrowsingAdapter::HandleCollectionMediaL, error %d"), aError))
        iMlInterface->MrcdamlbFolderListing(folderListing.Array(), cookie, aError);
        //HandleErrorL(aError);
        }
    folderListing.Close(); 
    iCurrentMediaLOp = EMediaLOpIdle;
    }

// -----------------------------------------------------------------------------
// CBTRCCBrowsingAdapter::SubscribeL
// -----------------------------------------------------------------------------
//
void CBTRCCBrowsingAdapter::SubscribeL()
    {
    TRACE_FUNC
    // Create subscription items
    CMPXSubscription* subscription( CMPXSubscription::NewL() );
    CleanupStack::PushL( subscription );

    CMPXSubscriptionItem* subItem1( CMPXSubscriptionItem::NewL() );
    CleanupStack::PushL( subItem1 );   
    
    subItem1->SetTObjectValueL( KMPXMessageCollectionId, KMPXMessageGeneral );
    subItem1->SetTObjectValueL( KMPXMessageMediaGeneralCategory, EMPXCollection );
    subItem1->SetTObjectValueL( KMPXMessageChangeEventType, EMPXItemDeleted );
    subscription->AddItemL( *subItem1 );
    
    CMPXSubscriptionItem* subItem2( CMPXSubscriptionItem::CopyL( *subItem1 ));
    CleanupStack::PushL( subItem2 );
    subItem2->SetTObjectValueL( KMPXMessageChangeEventType, EMPXItemInserted );
    subscription->AddItemL( *subItem2 );
    
    CMPXSubscriptionItem* subItem3( CMPXSubscriptionItem::CopyL( *subItem1 ));
    CleanupStack::PushL( subItem3 );
    subItem3->SetTObjectValueL( KMPXMessageChangeEventType, EMPXItemModified );
    subscription->AddItemL( *subItem3 );
    
    iCollectionUtility->Collection().AddSubscriptionL( *subscription );
    
    CleanupStack::PopAndDestroy( subItem3 );
    CleanupStack::PopAndDestroy( subItem2 );
    CleanupStack::PopAndDestroy( subItem1 );
    CleanupStack::PopAndDestroy( subscription );
    
    /* Example
    // Subscribe to only a few messages from collection utility
    subItem1->SetTObjectValueL( KMPXMessageGeneralId, KMPXMessageGeneral );
    subItem1->SetTObjectValueL( KMPXMessageGeneralEvent, TMPXCollectionMessage::EBroadcastEvent );
    subItem1->SetTObjectValueL( KMPXMessageGeneralType, EMcMsgDiskRemoved );
    subscription->AddItemL( *subItem1 );

    CMPXSubscriptionItem* subItem2( CMPXSubscriptionItem::CopyL( *subItem1 ));
    CleanupStack::PushL( subItem2 );
    subItem2->SetTObjectValueL( KMPXMessageGeneralType, EMcMsgUSBMassStorageStart );
    subscription->AddItemL( *subItem2 );
    CMPXSubscriptionItem* subItem3( CMPXSubscriptionItem::CopyL( *subItem1 ));
    CleanupStack::PushL( subItem3 );
    subItem3->SetTObjectValueL( KMPXMessageGeneralType, EMcMsgUSBMTPStart );
    subscription->AddItemL( *subItem3 );
    CMPXSubscriptionItem* subItem4( CMPXSubscriptionItem::CopyL( *subItem1 ));
    CleanupStack::PushL( subItem4 );
    subItem4->SetTObjectValueL( KMPXMessageGeneralType, EMcMsgFormatStart );
    subscription->AddItemL( *subItem4 );
    CMPXSubscriptionItem* subItem5( CMPXSubscriptionItem::CopyL( *subItem1 ));
    CleanupStack::PushL( subItem5 );
    subItem5->SetTObjectValueL( KMPXMessageGeneralType, EMcMsgUSBMassStorageEnd );
    subscription->AddItemL( *subItem5 );
    CMPXSubscriptionItem* subItem6( CMPXSubscriptionItem::CopyL( *subItem1 ));
    CleanupStack::PushL( subItem6 );
    subItem6->SetTObjectValueL( KMPXMessageGeneralType, EMcMsgUSBMTPEnd );
    subscription->AddItemL( *subItem6 );
    CMPXSubscriptionItem* subItem7( CMPXSubscriptionItem::CopyL( *subItem1 ));
    CleanupStack::PushL( subItem7 );
    subItem7->SetTObjectValueL( KMPXMessageGeneralType, EMcMsgFormatEnd );
    subscription->AddItemL( *subItem7 );
    CMPXSubscriptionItem* subItem8( CMPXSubscriptionItem::NewL() );
    CleanupStack::PushL( subItem8 );
    subItem8->SetTObjectValueL( KMPXMessageGeneralId, KMPXMessageGeneral );
    subItem8->SetTObjectValueL( KMPXMessageGeneralEvent, TMPXCollectionMessage::EPathChanged );
    subItem8->SetTObjectValueL( KMPXMessageGeneralType, EMcPathChangedByOpen );
    subItem8->SetTObjectValueL( KMPXMessageGeneralData, EMcItemOpened );
    subscription->AddItemL( *subItem8 );
    iCollectionUtility->Collection().AddSubscriptionL( *subscription );
    CleanupStack::PopAndDestroy( subItem8 );
    CleanupStack::PopAndDestroy( subItem7 );
    CleanupStack::PopAndDestroy( subItem6 );
    CleanupStack::PopAndDestroy( subItem5 );
    CleanupStack::PopAndDestroy( subItem4 );
    CleanupStack::PopAndDestroy( subItem3 );
    CleanupStack::PopAndDestroy( subItem2 );
    CleanupStack::PopAndDestroy( subItem1 );
    CleanupStack::PopAndDestroy( subscription ); */    
    }
