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
* Description:  This is the implementation of application class
 *
*/


#include <btdevice.h>
#include <bt_sock.h>
#include "hidsdpclient.h"
#include "bthiddevice.h"
#include "hidsdpobserver.h"
#include "bthidtypes.h"
#include "bthidserver.pan"
#include "descriptorlistparser.h"

class TServiceAttribute
    {
public:
    TSdpAttributeID iID;
    };

// HID Service Attributes
static const TServiceAttribute gHIDServiceAttributes[] =
    {
        {
        KHIDAttrDevRelNum
        },
        {
        KHIDAttrDevSubClass
        },
        {
        KHIDAttrCountryCode
        },
        {
        KHIDAttrVirtuallyCabled
        },
        {
        KHIDAttrReconnectInitiate
        },
        {
        KHIDAttrDescriptorList
        },
        {
        KHIDAttrProfileVersion
        },
        {
        KHIDAttrNormallyConnectable
        }
    };

// HID Service Attribute count 
static const TUint gHIDServAttrCount = sizeof(gHIDServiceAttributes) / sizeof(TServiceAttribute);

// PnP Service Attributes
static const TServiceAttribute gBTPnPServiceAttributes[] =
    {
        {
        KPnPAttrVendorID
        },
        {
        KPnPAttrProductID
        }
    };

// PnP Service Attribute count 
static const TUint gBTPnPAttrCount = sizeof(gBTPnPServiceAttributes)
        / sizeof(TServiceAttribute);

// Bit Mask flags for the mandatory fields value
const TUint KDevSubClassMandFlag = 1;
const TUint KCountryCodeMandFlag = 2;
const TUint KVirtuallyCabledMandFlag = 4;
const TUint KReconnectInitMandFlag = 8;
const TUint KDescriptorListMandFlag = 16;
const TUint KProfileVersionMandFlag = 32;
const TUint KVendorIDMandFlag = 64;
const TUint KProductIDMandFlag = 128;

// Expected mandatory fields value after the hid service record.
const TUint KHidServiceMandValue = KDevSubClassMandFlag
        | KCountryCodeMandFlag | KVirtuallyCabledMandFlag
        | KReconnectInitMandFlag | KDescriptorListMandFlag
        | KProfileVersionMandFlag;

// Expected mandatory fields value after the bt pnp service record.
const TUint KPnPServiceMandValue = KHidServiceMandValue | KVendorIDMandFlag
        | KProductIDMandFlag;

CHidSdpClient* CHidSdpClient::NewL(CBTHidDevice& aDevice,
        MHidSdpObserver& aObserver)
    {
    CHidSdpClient* self = NewLC(aDevice, aObserver);
    CleanupStack::Pop(self);
    return self;
    }

CHidSdpClient* CHidSdpClient::NewLC(CBTHidDevice& aDevice,
        MHidSdpObserver& aObserver)
    {
    CHidSdpClient* self = new (ELeave) CHidSdpClient(aDevice, aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CHidSdpClient::CHidSdpClient(CBTHidDevice& aDevice,
        MHidSdpObserver& aObserver) :
    CActive(EPriorityStandard), iDevice(aDevice), iObserver(aObserver),
            iParserState(EIdle)
    {
    }

CHidSdpClient::~CHidSdpClient()
    {
    delete iAgent;

    delete iHIDMatchList;

    delete iPNPMatchList;

    delete iSdpSearchPattern;
    }

void CHidSdpClient::ConstructL()
    {
    // Create the service search pattern object.
    iSdpSearchPattern = CSdpSearchPattern::NewL();

    // Create the attribute match list for HIDP Service.
    iHIDMatchList = CSdpAttrIdMatchList::NewL();

    TUint i;

    // Populate the list with the attributes we want to find.
    for (i = 0; i < gHIDServAttrCount; i++)
        {
        iHIDMatchList->AddL(TAttrRange(gHIDServiceAttributes[i].iID));
        }

    //Create the attribute match list for the BT PnP Service.
    iPNPMatchList = CSdpAttrIdMatchList::NewL();

    // Populate the list with the attributes we want to find.
    for (i = 0; i < gBTPnPAttrCount; i++)
        {
        iPNPMatchList->AddL(TAttrRange(gBTPnPServiceAttributes[i].iID));
        }
    }

void CHidSdpClient::Finish(TInt aError)
    {
    iParserState = EIdle;
    iObserver.HidSdpSearchComplete(aError);
    }

void CHidSdpClient::StartL()
    {
    // Delete any current agent.
    delete iAgent;
    iAgent = 0;

    // Reset the mandatory value bitmask.
    iMandAttrValue = 0;

    // Create a SDP Agent for the given BT Address.
    iAgent = CSdpAgent::NewL(*this, iDevice.iAddress);

    // Fill the record filter with the UUID of the HIDP Service.
    iSdpSearchPattern->AddL(KHidPServiceUUID);

    // Set the initial record filter.
    iAgent->SetRecordFilterL(*iSdpSearchPattern);

    // Request the next record.
    iAgent->NextRecordRequestL();

    // Set that we are inquiring for the HIDP Service.
    iParserState = EHIDService;
    }

void CHidSdpClient::Kill()
    {
    CActiveScheduler::Add(this);
    iStatus = KRequestPending;
    SetActive(); // Wait to be completed...
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, KErrNone); // ...which we do ourself
    }

void CHidSdpClient::NextRecordRequestComplete(TInt aError,
        TSdpServRecordHandle aHandle, TInt /*aTotalRecordsCount*/)
    {

    TInt error = aError;

    if (aError == KErrNone)
        {
        switch (iParserState)
            {
            // If this is supposed to be the HIDP Service Record.
            case EHIDService:
                // Request the attributes from the HIDP Service.
            TRAP( error,
                    iAgent->AttributeRequestL(aHandle, *iHIDMatchList);
            )
                break;

                // If this is supposed to be the BT Pnp Service Record.
            case EBTPnPService:
                // Request the attributes from the BT Pnp Service.
            TRAP( error,
                    iAgent->AttributeRequestL(aHandle, *iPNPMatchList);
            )
                error = KErrNone; //PNP not found but we don't care! FOR RUNNING WITH IAR TESTER!
                break;

            default:
                User::Panic(KBTHIDServer, EBadState);
                break;
            }
        }

    // If we have had any error, inform the observer as we have finished.
    if (error != KErrNone)
        {
        Finish(error);
        }
    }

void CHidSdpClient::AttributeRequestResult(TSdpServRecordHandle /*aHandle*/,
        TSdpAttributeID aAttrID, CSdpAttrValue* aAttrValue)
    {
    switch (iParserState)
        {
        // If this is supposed to be a HIDP Service Attribute.
        case EHIDService:
            // Check its type and assign to the appropriate member in the 
            // device object.
            switch (aAttrValue->Type())
                {
                case ETypeUint:
                    switch (aAttrID)
                        {
                        case KHIDAttrDevRelNum:
                            iDevice.iDeviceReleaseNumber = aAttrValue->Uint();
                            break;
                        case KHIDAttrDevSubClass:
                            iDevice.iDeviceSubClass = aAttrValue->Uint();
                            // Record that this mandatory item was found
                            iMandAttrValue |= KDevSubClassMandFlag;
                            break;
                        case KHIDAttrCountryCode:
                            iDevice.iCountryCode = aAttrValue->Uint();
                            iMandAttrValue |= KCountryCodeMandFlag;
                            break;
                        case KHIDAttrProfileVersion:
                            iDevice.iProfileVersion = aAttrValue->Uint();
                            iMandAttrValue |= KProfileVersionMandFlag;
                            break;
                        default:
                            // This is an attribute we didn't request,
                            // or wrong type.
                            break;
                        }
                    break;

                case ETypeBoolean:
                    switch (aAttrID)
                        {
                        case KHIDAttrVirtuallyCabled:
                            iDevice.iVirtuallyCabled = aAttrValue->Bool();
                            iMandAttrValue |= KVirtuallyCabledMandFlag;
                            break;
                        case KHIDAttrReconnectInitiate:
                            iDevice.iReconnectInit = aAttrValue->Bool();
                            iMandAttrValue |= KReconnectInitMandFlag;
                            break;
                        case KHIDAttrNormallyConnectable:
                            iDevice.iNormallyConnectable = aAttrValue->Bool();
                            break;
                        default:
                            // This is an attribute we didn't request 
                            // or wrong type.
                            break;
                        }
                    break;

                case ETypeDES:
                    switch (aAttrID)
                        {
                        case KHIDAttrDescriptorList:
                            {
                                TRAP_IGNORE(
                                        CDescriptorListParser* visitor = new CDescriptorListParser(*(iDevice.iDescList));
                                        if( visitor )
                                            {
                                            CleanupStack::PushL(visitor);
                                            aAttrValue->AcceptVisitorL(*visitor);
                                            CleanupStack::PopAndDestroy(visitor);
                                            }
                                )
                            // Don't care about any error that could occur
                            iMandAttrValue |= KDescriptorListMandFlag;
                            }
                            break;
                        default:
                            // This is an attribute we didn't request 
                            // or wrong type.
                            break;
                        }
                    break;

                default:
                    // This is an attribute we didn't request or wrong type.
                    break;
                }

            break;

            // If this is supposed to be a BT Pnp Service Attribute.
        case EBTPnPService:
            // Check its type and assign to the appropriate member in the 
            // device object.
            switch (aAttrValue->Type())
                {
                case ETypeUint:
                    switch (aAttrID)
                        {
                        case KPnPAttrVendorID:
                            iDevice.iVendorID = aAttrValue->Uint();
                            iMandAttrValue |= KVendorIDMandFlag;
                            break;
                        case KPnPAttrProductID:
                            iDevice.iProductID = aAttrValue->Uint();
                            iMandAttrValue |= KProductIDMandFlag;
                            break;
                        default:
                            // This is an attribute we didn't request,
                            // or wrong type.
                            break;
                        }
                    break;

                default:
                    // This is an attribute we didn't request or wrong type.
                    break;
                }

            break;

        default:
            // We shouldn't get this state.
            User::Panic(KBTHIDServer, EBadState);
            break;
        }

    // Ownership has been transferred
    delete aAttrValue;
    }

void CHidSdpClient::AttributeRequestComplete(
        TSdpServRecordHandle /*aHandle*/, TInt aError)
    {
    if (aError == KErrNone)
        {
        switch (iParserState)
            {
            // If this is supposed to be the HIDP Service Record.
            case EHIDService:
                // If we found all the attributes we requested.
                if (iMandAttrValue == KHidServiceMandValue)
                    {
                    // Start a new sdp agent to request the BT Pnp 
                    // Service Record.
                    delete iAgent;
                    iAgent = 0;
                    TRAPD( res,
                            iAgent = CSdpAgent::NewL(*this, iDevice.iAddress);
                            iSdpSearchPattern->Reset();
                            iSdpSearchPattern->AddL(KPnPServiceUUID);
                            iAgent->SetRecordFilterL(*iSdpSearchPattern);
                            iAgent->NextRecordRequestL();
                            iParserState = EBTPnPService;
                    )

                    // If we failed, inform the observer of the error.
                    if (res != KErrNone)
                        {
                        Finish(res);
                        }
                    }
                else
                    {
                    // Not all attributes were found so inform the observer.
                    Finish(KErrNotFound);
                    }
                break;

                // If this is supposed to be the BT PnP Service Record.
            case EBTPnPService:
                // If we found all the attributes we requested.
                if (iMandAttrValue == KPnPServiceMandValue)
                    {
                    // Report success to the observer.
                    Finish(KErrNone);
                    }
                else
                    {
                    // Not all attributes were found so inform the observer.
                    Finish(KErrNone); //We don't care if all info wasn't found!
                    }
                break;

            default:
                // We shouldn't get this state.
                User::Panic(KBTHIDServer, EBadState);
                break;
            }
        }
    else
        {
        // Inform the observer of the error.
        Finish(aError);
        }
    }

