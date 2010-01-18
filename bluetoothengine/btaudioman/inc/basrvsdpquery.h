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
* Description: 
*       Implementations of SDP queries
*
*/


#ifndef BASRVSDPQUERY_H
#define BASRVSDPQUERY_H

#include <e32base.h>
#include <btsdp.h>
#include <bt_sock.h>
#include <btmanclient.h>
#include "BTAccInfo.h"
#include "basrvactive.h"

class TBTDevAddr;
class CSdpSearchPattern;


class TBTSdpAttrInt
    {
public:
    TUUID iUUID;   // The UUID of the service. Must be specified by caller.
    TUint16 iAttrID;  // The Attribute ID. Must be specified by the caller.
    TUint iAttrValue; // The value of the attribute if this is found in the device.
    };

class MBasrvSdpQuerier
    {
public:

    /**
     * Indicates GetAccInfo completed.
     * @param aAcc the accinfo if the operation succeeded.
     * @param aErr the completion status
     */
    virtual void GetAccInfoCompletedL(TInt aErr, const TAccInfo* aAcc, const TBTDeviceClass* aCod) = 0;

    };

/**
* The base class for various SDP query strategies
*/
class CBasrvSdpQuery : public CBase, public MSdpAgentNotifier, public MBasrvActiveObserver,
    public MSdpAttributeValueVisitor
   {
public:
    
   static CBasrvSdpQuery* NewL(MBasrvSdpQuerier& aQuerier);
    
   ~CBasrvSdpQuery();

   void QueryAccInfoL(const TBTDevAddr& aDevAddr, TBool aTolerateSdpError);
    
private:    // From MSdpAgentNotifier

	/** 
	* Called when an service record request operation completes.
	*
	* @param aError KErrNone if successful;
	*               KErrEof if there are no more SDP records left to be read; or an SDP error.
	* @param aHandle Service record for which the query was made
	* @param aTotalRecordsCount Total number of matching records
	*/
    void NextRecordRequestComplete(
        TInt aError, 
        TSdpServRecordHandle aHandle,
        TInt aTotalRecordsCount);

	/**
	* Called when an attribute request (CSdpAgent::AttributeRequestL()) wants to 
	* pass up a result.
	*
	* @param aHandle Service record for which the query was made
	* @param aAttrID ID of the attribute obtained
	* @param aAttrValue Attribute value obtained
    */
    void AttributeRequestResult(
        TSdpServRecordHandle aHandle, 
        TSdpAttributeID aAttrID, 
        CSdpAttrValue* aAttrValue);

	/** 
	* Called when an attribute request (CSdpAgent::AttributeRequestL()) wants to 
	* signal the completion of a attribute request.
	*
	* @param aHandle Service record for which the query was made
	* @param aError an error
	*/
    void AttributeRequestComplete(TSdpServRecordHandle aHandle, TInt aError);
    // from base class MSdpAttributeValueVisitor

    /**
     * From MSdpAttributeValueVisitor.
     * Called to pass an attribute value.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void VisitAttributeValueL( CSdpAttrValue &aValue, TSdpElementType aType );

    /**
     * From MSdpAttributeValueVisitor.
     * Called to indicate the start of a list of attribute values. 
     * This call is followed by a call to VisitAttributeValueL() 
     * for each attribute value in the list, and concluded by 
     * a call to EndList().
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void StartListL( CSdpAttrValueList &aList );

    /**
     * From MSdpAttributeValueVisitor.
     * Called to indicate the end of a list of attribute values.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    void EndListL();
    
private:
    // From MBasrvActiveObserver
    
    void RequestCompletedL(CBasrvActive& aActive);
    
    void CancelRequest(CBasrvActive& aActive);


private:  

    void ConstructL();

    CBasrvSdpQuery(MBasrvSdpQuerier& aQuerier);

    void QueryCompleteL(TInt aStatus);
    
    void DoNextServiceRecordRequestL();

    void NextRecordRequestCompleteL(
        TInt aError, 
        TSdpServRecordHandle aHandle, 
        TInt aTotalRecordsCount);

    void AttributeRequestResultL(
        TSdpServRecordHandle aHandle, 
        TSdpAttributeID aAttrID, 
        CSdpAttrValue* aAttrValue);

    void AttributeRequestCompleteL(TSdpServRecordHandle aHandle, TInt aError);

    void MakeAccInfo(TAccInfo& aInfo);

private:

    MBasrvSdpQuerier& iQuerier; // not own

    CSdpAgent* iSdpAgent;
    CSdpSearchPattern* iSdpSearchPattern;
    RArray<TBTSdpAttrInt> iServiceAttrs;

    TInt iCursor;
    TBTDevAddr iAddr;
    
    CBasrvActive* iActive;
    RBTRegServ iRegServ;
    RBTRegistry iReg;
	CBTRegistryResponse* iResponse;
    TBool iTolerateSdpError;
    };
#endif      // BASRVSDPQUERY_H


