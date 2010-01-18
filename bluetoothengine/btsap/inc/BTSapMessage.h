/*
* Copyright (c) 2004-2005 Nokia Corporation and/or its subsidiary(-ies).
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
*     This class handles SAP message encoding and decoding
*
*/

#ifndef BT_SAP_MESSAGE_H
#define BT_SAP_MESSAGE_H

//  INCLUDES
#include <e32std.h>

const TInt KMaxMsgSize = 4096;
const TInt KMinMsgSize = 275; // biggest size of apdu_req
const TInt KParaLenModulo = 4;

const TInt KMsgIDRange = 18;
const TInt KParaIDRange = 8;
const TInt KMsgIDOffset = 0;
const TInt KParaNumOffset = 1;
const TInt KHeaderResvLen = 2;
const TInt KPayloadOffset = 4;
const TInt KParaIDLen = 1;
const TInt KParaResvLen = 1;
const TInt KParaLenLen = 2;
const TInt KParaResvValue = 0x00;
const TInt KHeaderResvValue = 0x0000;

enum TMsgID
    {
    EConnectRequest,
    EConnectResponse,
    EDisconnectRequest,
    EDisconnectResponse,
    EDisconnectIndication,
    ETransferAPDURequest,
    ETransferAPDUResponse,
    ETransferATRRequest,
    ETransferATRResponse,
    EPowerSimOffRequest,
    EPowerSimOffResponse,
    EPowerSimOnRequest,
    EPowerSimOnResponse,
    EResetSimRequest,
    EResetSimResponse,
    ETransferCardReaderStatusRequest,
    ETransferCardReaderStatusResponse,
    EStatusIndication,
    EErrorResponse
    };

enum TParaID
    {
    EParaMaxMsgSize,
    EParaConnectionStatus,
    EParaResultCode,
    EParaDisconnectionType,
    EParaCommandAPDU,
    EParaResponseAPDU,
    EParaATR,
    EParaCardReaderStatus,
    EParaStatusChange,
    EParaCommandAPDU7816 = 0x10  // This parameter's ID is not sequential
    };

enum TResultCode
    {
    EResultCodeOK,
    EResultCodeUnknown,
    EResultCodeNotAccessible,
    EResultCodePowerOff,
    EResultCodeCardRemoved,
    EResultCodeAlreadyPowerOn,
    EResultCodeDataNotAvailable,
    EResultCodeReserved = 100
    };

enum TCardStatus
    {
    ECardStatusError,
    ECardStatusReset,
    ECardStatusNotAccessible,
    ECardStatusRemoved,
    ECardStatusInserted,
    ECardStatusRecovered,
    ECardStatusReserved = 100
    };

enum TConnectionStatus
    {
    EConnectionOK,
    EConnectionErrReject,
    EConnectionErrNotSupported,
    EConnectionErrTooSmall
    };
    
enum TValidationResult
    {
    EValidFormat,
    EInvalidSegmented,
    EInvalidMsgID,
    EInvalidReservedBytes,
    EInvalidParameterID,
    EInvalidPaddingBytes,
    EInvalidParaNumTooSmall,
    EInvalidUnknown
    };

const TInt KMapErrCode[] = 
    {
    KErrNone,
    KErrAlreadyExists,
    KErrDisconnected,
    KErrArgument,
    KErrNotReady,
    KErrInUse,
    KErrCompletion,
    KErrGeneral
    };

const TResultCode KMapResultCode[] =
    {
    EResultCodeOK,
    EResultCodeAlreadyPowerOn,
    EResultCodePowerOff,
    EResultCodeUnknown,
    EResultCodeCardRemoved,
    EResultCodeNotAccessible,
    EResultCodeCardRemoved,
    EResultCodeDataNotAvailable
    };

const TMsgID KMapRequestID[] =
    {
    EConnectRequest,
    EDisconnectRequest,
    ETransferAPDURequest,
    ETransferATRRequest,
    EPowerSimOffRequest,
    EPowerSimOnRequest,
    EResetSimRequest,
    ETransferCardReaderStatusRequest,
    };

const TMsgID KMapMsgID[] =
    {
    EConnectRequest,
    ETransferAPDURequest,
    ETransferAPDURequest
    };

const TParaID KMapParaID[] =
    {
    EParaMaxMsgSize,
    EParaCommandAPDU,
    EParaCommandAPDU7816
    };


// CLASS DECLARATION

/**
* SAP message format handler class.
*/
class TBTSapMessage
    {
public:
    TBTSapMessage();

    void SetMsgID(TMsgID aMsgID);

    TMsgID MsgID();

    void AddParameter(TParaID aParaID, TInt aValue, TInt aLen = 1);

    void AddParameter(TParaID aParaID, const TDesC8& aValue);

    TInt GetParameter(TParaID aParaID, TDes8& aValue);

    TInt GetParameter(TParaID aParaID, TInt& aValue);

    // Copy data
    void SetData(const TDes8& aData);

    TInt AppendData(const TDes8& aData);

    void Reset();

    TDes8& Data();

    TBool IsEmpty();

    TValidationResult Validate();

    TResultCode ToResultCode(TInt errorCode);

    TUint8 GetParaNum();

    TInt GetParaID(const TUint8 index, TParaID &aParaID);

private:
    TInt FindParameter(TParaID aParaID, TInt& aValuePos, TInt& aValueLen);

    void AddParameter(TParaID aParaID, const TUint8* aValue, TInt aLen);

    TBool IsValidParaID(const TParaID aParaID, const TMsgID aMsgID) const;
    
    TBool IsValidRequestID(const TMsgID aRequestID) const;

private:
    TBuf8<KMaxMsgSize> iData;
    };


#endif      // BT_SAP_MESSAGE_H   
            
// End of File
