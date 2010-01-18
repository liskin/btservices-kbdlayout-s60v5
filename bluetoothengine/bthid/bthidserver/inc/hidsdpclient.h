/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares main application class.
 *
*/


#ifndef __HIDSDPCLIENT_H__
#define __HIDSDPCLIENT_H__

#include <e32base.h>
#include <btsdp.h>

class CBTHidDevice;
class MHidSdpObserver;

/*!
 This class queries the SDP database of a HID device in order to populate an
 instance of the CBTHidDevice class
 */
class CHidSdpClient : public CActive, public MSdpAgentNotifier
    {
public:

    /*!
     Create a CHidSdpClient object
     @param aDevice device details object to populate
     @param aObserver observer to report to.
     @result A pointer to the created instance of CHidSdpClient
     */
    static CHidSdpClient* NewL(CBTHidDevice& aDevice,
            MHidSdpObserver& aObserver);

    /*!
     Create a CHidSdpClient object
     @param aDevice device details object to populate
     @param aObserver observer to report to.
     @result A pointer to the created instance of CHidSdpClient
     */
    static CHidSdpClient* NewLC(CBTHidDevice& aDevice,
            MHidSdpObserver& aObserver);

    /*!
     Destroy the object and release all memory objects
     */
    ~CHidSdpClient();

    /*!
     Start the SDP inquiry.
     */
    void StartL();

    /*!
     Ask the object to destroy itself when its conventient.
     */
    void Kill();

public:
    // From CActive
    void RunL()
        {
        delete this;
        }
    ;
    void DoCancel()
        {
        }
    ;

public:
    // From MSdpAgentNotifier
    /*!
     Called when an service record request 
     (CSdpAgent::NextRecordRequestComplete()) operation completes.
     @param aError KErrNone, or an SDP error
     @param aHandle Service record for which the query was made
     @param aTotalRecordsCount Total number of matching records
     */

    void NextRecordRequestComplete(TInt aError, TSdpServRecordHandle aHandle,
            TInt aTotalRecordsCount);
    /*!
     Called by the attribute request function 
     (CSdpAgent::AttributeRequestL()) to pass the results of a successful 
     attribute request.
     @param aHandle Service record for which the query was made
     @param aAttrID ID of the attribute obtained
     @param aAttrValue Attribute value obtained
     */

    void AttributeRequestResult(TSdpServRecordHandle aHandle,
            TSdpAttributeID aAttrID, CSdpAttrValue* aAttrValue);

    /*!
     Called when an service record request 
     (CSdpAgent::NextRecordRequestComplete()) operation completes.
     @param aHandle Service record for which the query was made
     @param aError KErrNone, or an SDP error
     */

    void AttributeRequestComplete(TSdpServRecordHandle aHandle, TInt aError);

private:

    /*!
     Constructs this object
     @param aDevice device details object to populate
     @param aObserver observer to report to.
     */
    CHidSdpClient(CBTHidDevice& aDevice, MHidSdpObserver& aObserver);

    /*!
     Performs second phase construction of this object
     */
    void ConstructL();

    /*
     Reset the object and inform the observer of the result
     @param aError KErrNone if successful, or an SDP error
     */
    void Finish(TInt aError);

private:
    // Member variables
    /*! Current Parser State */
    enum TParserState
        {
        EIdle, /*!< Parser is idle */
        EHIDService, /*!< Parser is retrieving the HID Service Record */
        EBTPnPService
        /*!< Parser is retrieving the PnP Service Record */
        };

    /*! Device details object to fill from sdp query */
    CBTHidDevice& iDevice;

    /*! Observer to inform of results */
    MHidSdpObserver& iObserver;

    /*! Current state of the parser */
    TParserState iParserState;

    /*! SDP Agent used to search the SDP database */
    CSdpAgent* iAgent;

    /*! UUID pattern to filter the SDP database search */
    CSdpSearchPattern* iSdpSearchPattern;

    /*! HID Service record attribute filter */
    CSdpAttrIdMatchList* iHIDMatchList;

    /*! BT PnP Service record attribute filter */
    CSdpAttrIdMatchList* iPNPMatchList;

    /*! A bitmask of the mandatory  attributes found */
    TUint iMandAttrValue;
    };

#endif // __HIDSDPCLIENT_H__
