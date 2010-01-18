/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Helper class for performing SDP queries.
*
*/



#include <btsdp.h>

#include "btengsdpquery.h"
#include "btengdiscovery.h"
#include "btengsdpattrparser.h"
#include "debug.h"


/**  Array granularity for storing SDP attribute values (the number 
  *  of attributes returned from an attribute search is expected 
  *  to be small).
  */
const TInt KBTEngDefaultGranularity = 2;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSdpQuery::CBTEngSdpQuery( MBTEngSdpResultReceiver* aNotifier )
:   iAttrValArray( KBTEngDefaultGranularity ),
    iResultNotifier( aNotifier )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iParser = CBTEngSdpAttrParser::NewL( &iAttrResArray );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSdpQuery* CBTEngSdpQuery::NewL( MBTEngSdpResultReceiver* aNotifier )
    {
    CBTEngSdpQuery* self = new( ELeave ) CBTEngSdpQuery( aNotifier );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSdpQuery::~CBTEngSdpQuery()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    delete iSdpAgent;
    delete iParser;
    iResultNotifier = NULL;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::Cancel()
    {
    TRACE_FUNC_ENTRY
    if( iQueryType != EQueryIdle )
        {
            // Only call the agent when a search is active, as it 
            // involves some cleanup which is not necessary for every call.
        iSdpAgent->Cancel();
        iQueryType = EQueryIdle;
        }
    iRecHandleArray.Close();
    iAttrResArray.Close();
    iAttrValArray.ResetAndDestroy();
    iRequestedAttrId = 0;
    }


// ---------------------------------------------------------------------------
// Service query
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::RemoteSdpQueryL( const TBTDevAddr& aAddr, const TUUID& aService )
    {
    TRACE_FUNC_ENTRY
    CheckSdpAgentL( aAddr );
    if( iQueryType == EQueryIdle )
        {
        iQueryType = EServiceQuery;
        iAttrResArray.Close();
        iAttrValArray.ResetAndDestroy();
        iRequestedAttrId = 0;
        }
    iRecHandleArray.Close();
    CSdpSearchPattern* searchPattern = CSdpSearchPattern::NewL();
    CleanupStack::PushL( searchPattern );
    searchPattern->AddL( aService );
    iSdpAgent->SetRecordFilterL( *searchPattern );  // Copies searchpattern.
    iSdpAgent->NextRecordRequestL();
    CleanupStack::PopAndDestroy( searchPattern );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Attribute query
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::RemoteSdpQueryL( const TBTDevAddr& aAddr, 
    const TSdpServRecordHandle aHandle, const TSdpAttributeID aAttrId )
    {
    TRACE_FUNC_ENTRY
    CheckSdpAgentL( aAddr );
        // this could be part of a ServiceSearchAttribute query.
    if( iQueryType == EQueryIdle )
        {
        iQueryType = EAttrQuery;
        iRecHandleArray.Close();
        }
    iRequestedAttrId = aAttrId;
    iAttrResArray.Close();
    iAttrValArray.ResetAndDestroy();
    iSdpAgent->AttributeRequestL( aHandle, aAttrId );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ServiceAttribute query
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::RemoteSdpQueryL( const TBTDevAddr& aAddr, const TUUID& aService, 
    const TSdpAttributeID aAttrId )
    {
    TRACE_FUNC_ENTRY
    if( iQueryType != EQueryIdle )
        {
        User::Leave( KErrInUse );
        }
    iQueryType = EServiceAttrQuery;
    iRequestedAttrId = aAttrId;
    iRecHandleArray.Close();
    iAttrResArray.Close();
    iAttrValArray.ResetAndDestroy();
        // First query for the requested service.
    RemoteSdpQueryL( aAddr, aService );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MSdpAgentNotifier.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::NextRecordRequestComplete( TInt aError, 
    TSdpServRecordHandle aHandle, TInt aTotalRecordsCount )
    {
    TRACE_FUNC_ENTRY
    if( aError == KErrNone && aTotalRecordsCount > 0 )
        {
            // Store the result and wait for the next.
        iRecHandleArray.Append( aHandle );
        TRAP( aError, iSdpAgent->NextRecordRequestL() );
        }
    else if( aError == KErrEof )
        {
            // The last notification does not contain any result anymore.
        if( iQueryType == EServiceAttrQuery )
            {
            if( iRecHandleArray.Count() > 0 )
                {
                    // Request the attribute for all matching service records.
                TRAP( aError, RemoteSdpQueryL( iBDAddr, iRecHandleArray[ 0 ], 
                                                iRequestedAttrId ) );
                }
            else
                {
                    // No matches, inform the client.
                iQueryType = EQueryIdle;
                iResultNotifier->ServiceAttributeSearchComplete( aHandle, 
                                                                  iAttrResArray, 
                                                                  aError );
                }
            }
        else
            {
                // This was a ServiceSearch, inform our client of the result.
            TInt recCount = iRecHandleArray.Count();
            if( recCount > 0 )
                {
                    // KErrEof is used to indicate no matching records.
                aError = KErrNone;
                }
            iQueryType = EQueryIdle;
            iResultNotifier->ServiceSearchComplete( iRecHandleArray, recCount, 
                                                     aError );
            }
        }
    NotifyError( aError );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MSdpAgentNotifier.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::AttributeRequestResult( TSdpServRecordHandle aHandle, 
    TSdpAttributeID aAttrID, CSdpAttrValue* aAttrValue )
    {
    TRACE_FUNC_ENTRY
        // Ownership of CSdpAttrValue is passed here. Store the object so 
        // that the result array can point to string values rather than 
        // copy them (the object is anyway already constructed).
    iAttrValArray.Append( aAttrValue );
    iParser->SetAttributeID( aAttrID );
        // Adds the attribute values synchronously to the result array.
    TRAP_IGNORE( aAttrValue->AcceptVisitorL( *iParser ) );
    (void) aHandle;
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MSdpAgentNotifier.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::AttributeRequestComplete( TSdpServRecordHandle aHandle, 
    TInt aError )
    {
    TRACE_FUNC_ENTRY
    if( aError )
        {
        NotifyError( aError );
        return;
        }

    if( iQueryType == EAttrQuery )
        {
            // This was a AttributeSearch, inform our client of the result.
        iQueryType = EQueryIdle;
        iResultNotifier->AttributeSearchComplete( aHandle, iAttrResArray, aError );
        }
    else if( iQueryType == EServiceAttrQuery )
        {
        TInt index = iRecHandleArray.Find( aHandle );
        if( index <= KErrNotFound )
            {
            NotifyError( KErrCorrupt );
            return;
            }
            // Pop the handle, since we don't need it anymore.
        iRecHandleArray.Remove( index );
        if( iRecHandleArray.Count() == 0 )
            {
                // This was the last result.
            aError = KErrEof;
            iQueryType = EQueryIdle;
            }
        iResultNotifier->ServiceAttributeSearchComplete( aHandle, iAttrResArray, 
                                                          aError );
        if( aError != KErrEof && iQueryType == EServiceAttrQuery )
            {
                // Request the attribute from the next matching record (if 
                // not cancelled in the callback, hence the check for the
                // querytype) This will reset the attribute result array.
            TRAP( aError, RemoteSdpQueryL( iBDAddr, iRecHandleArray[ 0 ], 
                                            iRequestedAttrId ) );
            NotifyError( aError );  // Does nothing if there is no error.
            }
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::NotifyError( TInt aError )
    {
    if( aError == KErrNone || aError == KErrEof || iResultNotifier == NULL )
        {
        return;
        }
    TSdpQueryType query = iQueryType;
    iQueryType = EQueryIdle;    // Need to set this before calling clients, 
                                // as they may call us straight again.
    if( query == EServiceQuery )
        {
        iResultNotifier->ServiceSearchComplete( RSdpRecHandleArray(), 0, aError );
        }
    else if( query == EAttrQuery )
        {
        iResultNotifier->AttributeSearchComplete( 0, RSdpResultArray(), aError );
        }
    if( query == EServiceAttrQuery )
        {
        iResultNotifier->ServiceAttributeSearchComplete( 0, RSdpResultArray(), 
                                                          aError );
        }
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpQuery::CheckSdpAgentL( const TBTDevAddr& aAddr )
    {
    if( !iSdpAgent || aAddr != iBDAddr )
        {
        delete iSdpAgent;
        iSdpAgent = NULL;
        iBDAddr = aAddr;
        iSdpAgent = CSdpAgent::NewL( *this, iBDAddr );
        }
    else
        {
        iSdpAgent->Cancel();
        }
    if( iQueryType != EQueryIdle && iQueryType != EServiceAttrQuery )
        {
            // Another query is ongoing and has not been cancelled.
        User::Leave( KErrInUse );
        }
    }
