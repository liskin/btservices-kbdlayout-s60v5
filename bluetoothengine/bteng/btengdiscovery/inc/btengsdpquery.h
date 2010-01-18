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


#ifndef BTENGSDPQUERY_H
#define BTENGSDPQUERY_H

#include "btengdiscovery.h"

class CBTEngSdpAttrParser;

/**  Array definition for storing SDP record handles */
typedef RPointerArray<CSdpAttrValue> RSdpAttrValueArray;


/**
 *  Class CBTEngSdpQuery
 *
 *  ?more_complete_description
 *
 *  @lib btengdiscovery.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngSdpQuery ) : public CBase, public MSdpAgentNotifier
    {

public:

    /**  ?description */
    enum TSdpQueryType
        {
        EQueryIdle,
        EServiceQuery,
        EAttrQuery,
        EServiceAttrQuery
        };

    /**
     * Two-phase constructor
     */
    static CBTEngSdpQuery* NewL( MBTEngSdpResultReceiver* aNotifier );

    /**
     * Destructor
     */
    virtual ~CBTEngSdpQuery();

    /**
     * ?description
     *
     * @since S60 v3.2
     */
    void Cancel();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @param aService ?description
     */
    void RemoteSdpQueryL( const TBTDevAddr& aAddr, const TUUID& aService );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @param aHandle ?description
     * @param aAttrId ?description
     */
    void RemoteSdpQueryL( const TBTDevAddr& aAddr, 
                           const TSdpServRecordHandle aHandle, 
                           const TSdpAttributeID aAttrId );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @param aService ?description
     * @param aAttrId ?description
     */
    void RemoteSdpQueryL( const TBTDevAddr& aAddr, const TUUID& aService, 
                           const TSdpAttributeID aAttrId );

// from base class MSdpAgentNotifier

    /**
     * From MSdpAgentNotifier.
     * Called when an service record request operation completes.
     *
     * @since S60 v3.2
     * @param aError KErrNone if successful; KErrEof if there are no more 
     *               SDP records left to be read; or an SDP error.
     * @param aHandle Service record for which the query was made.
     * @param aTotalRecordsCount Total number of matching records.
     */
    virtual void NextRecordRequestComplete( TInt aError, 
                                             TSdpServRecordHandle aHandle, 
                                             TInt aTotalRecordsCount );

    /**
     * From MSdpAgentNotifier.
     * Called when an attribute request wants to pass up a result.
     *
     * @since S60 v3.2
     * @param aHandle Service record for which the query was made.
     * @param aAttrID ID of the attribute obtained.
     * @param @param aAttrValue Attribute value obtained.
     */
    virtual void AttributeRequestResult( TSdpServRecordHandle aHandle, 
                                          TSdpAttributeID aAttrID, 
                                          CSdpAttrValue* aAttrValue );

    /**
     * From MSdpAgentNotifier.
     * Called when an attribute request wants to signal the completion 
     * of a attribute request.
     *
     * @since S60 v3.2
     * @param aHandle Service record for which the query was made.
     * @param aError An error.
     */
    virtual void AttributeRequestComplete( TSdpServRecordHandle aHandle, 
                                            TInt aError );

private:

    /**
     * C++ default constructor
     */
    CBTEngSdpQuery( MBTEngSdpResultReceiver* aNotifier );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

    /**
     * Helper function to pass back an error to our client.
     *
     * @since S60 v3.2
     * @param aError The error to return to the result observer.
     */
    void NotifyError( TInt aError );

    /**
     * Helper function to check that
     *
     * @since S60 v3.2
     * @param aAddr ?description
     * @return KErrNone if sucessful, 
     *         KErrNotReady if no SDP result notifier has been set,
     *          otherwise the error code indicating the error situation.
     */
    void CheckSdpAgentL( const TBTDevAddr& aAddr );

private: // data

    /**
     * Flag to check if an SDP query is ongoing.
     */
    TSdpQueryType iQueryType;

    /**
     * Flag to check if an SDP query is ongoing.
     */
    TBTDevAddr iBDAddr;

    /**
     * Attribute ID for second step of ServiceAttributeSearch 
     * operation; otherwise NULL.
     */
    TSdpAttributeID iRequestedAttrId;

    /**
     * Result array of discovered service record handles.
     */
    RSdpRecHandleArray iRecHandleArray;

    /**
     * Result array of discovered service record attributes.
     */
    RSdpResultArray iAttrResArray;

    /**
     * Array for temporarily storing CSdpAttrValue objects. The actual 
     * value is used through the RSdpResultArray member.
     */
    RSdpAttrValueArray iAttrValArray;

    /**
     * ?description_of_pointer_member
     * Own.
     */
    CSdpAgent* iSdpAgent;

    /**
     * Parser for decoding SDP attribute values.
     * Own.
     */
    CBTEngSdpAttrParser* iParser;

    /**
     * Reference to receiver of remote SDP query results.
     * Not own.
     */
    MBTEngSdpResultReceiver* iResultNotifier;

    };

#endif // BTENGSDPQUERY_H
