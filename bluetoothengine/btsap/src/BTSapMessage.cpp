/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
*     This class handles SAP message encoding/decoding
*
*
*/


// INCLUDE FILES
#include "BTSapMessage.h"
#include "debug.h"

TBTSapMessage::TBTSapMessage()
    {
    }

// ---------------------------------------------------------
// SetMsgID
// ---------------------------------------------------------
void TBTSapMessage::SetMsgID(TMsgID aMsgID)
    {
    iData.Zero();
    // MsgID
    iData.Append(aMsgID);
    // para number
    iData.Append(0);
    // reserve
    iData.AppendFill(0, 2);
    }

// ---------------------------------------------------------
// MsgID
// ---------------------------------------------------------
TMsgID TBTSapMessage::MsgID()
    {
    return TMsgID(iData[KMsgIDOffset]);
    }

void TBTSapMessage::AddParameter(TParaID aParaID, TInt aValue, TInt aLen)
    {
    if (aLen == 2)
        {
        TBuf8<KParaLenLen> value;
        value.Append(aValue >> 8);
        value.Append(aValue & 0xff);
        AddParameter(aParaID, value);
        }
    else
        {
        TUint8 value = (TUint8)aValue;
        AddParameter(aParaID, &value, 1);
        }
    }

void TBTSapMessage::AddParameter(TParaID aParaID, const TDesC8& aValue)
    {
    AddParameter(aParaID, aValue.Ptr(), aValue.Length());
    }

void TBTSapMessage::AddParameter(TParaID aParaID, const TUint8* aValue, TInt aLen)
    {
    // increase the number of parameters
    iData[KParaNumOffset] ++;
    //iData.Replace(1, 1, paraNum);

    // parameter ID
    iData.Append(aParaID);
    // reserve
    iData.AppendFill(0, KParaResvLen);
    // length
    //TUint16 paraLen = (TUint16)aLen;
    iData.Append(aLen >> 8); // high byte
    iData.Append(aLen & 0xff); // low byte

    // value
    iData.Append(aValue, aLen);

    //padding
    TInt reminder = aLen % KParaLenModulo;
    if (reminder > 0)
        {
        iData.AppendFill(0, KParaLenModulo - reminder);
        }
    }

TInt TBTSapMessage::GetParameter(TParaID aParaID, TDes8& aValue)
    {
    TInt valuePos = 0;
    TInt valueLen = 0;
    TInt retVal = FindParameter(aParaID, valuePos, valueLen);

    if (retVal == KErrNone)
        {
        aValue.Copy(iData.Mid(valuePos, valueLen));
        }

    return retVal;
    }

TInt TBTSapMessage::GetParameter(TParaID aParaID, TInt& aValue)
    {
    TInt valuePos = 0;
    TInt valueLen = 0;
    TInt retVal = FindParameter(aParaID, valuePos, valueLen);

    if (retVal == KErrNone)
        {
        if (valueLen == 1)
            {
            aValue = iData[valuePos];
            }
        else if (valueLen == 2)
            {
            aValue = (iData[valuePos] << 8) + iData[valuePos + 1];
            }
        else
            {
            retVal = KErrArgument;
            }
        }

    return retVal;
    }

TInt TBTSapMessage::FindParameter(TParaID aParaID, TInt& aValuePos, TInt& aValueLen)
    {
    TInt retVal = KErrNotFound;
    TInt pos = KPayloadOffset;
    TInt valueLenPos = 0;

    while (pos < iData.Length())
        {
        valueLenPos = pos + KParaIDLen + KParaResvLen;
        aValueLen = iData[valueLenPos]  * 0xff + iData[valueLenPos + 1];

        if (iData[pos] == aParaID)
            {
            aValuePos = valueLenPos + KParaLenLen;
            retVal = KErrNone;
            break;
            }

        TInt paddingLen = (KParaLenModulo - (aValueLen % KParaLenModulo)) % KParaLenModulo;
        pos += KParaIDLen + KParaResvLen + KParaLenLen + aValueLen + paddingLen;
        }

    return retVal;
    }

void TBTSapMessage::SetData(const TDes8& aData)
    {
    iData = aData;
    }

TInt TBTSapMessage::AppendData(const TDes8& aData)
    {
    if ( iData.Length() + aData.Length() <= iData.MaxLength() )
        {
        iData.Append(aData);
        }
    else
        {
        return EInvalidSegmented;
        }

    return KErrNone;
    }

void TBTSapMessage::Reset()
    {
    iData.Zero();
    }

TBool TBTSapMessage::IsEmpty()
    {
    return (iData.Length() == 0);
    }

TDes8& TBTSapMessage::Data()
    {
    return iData;
    }

TValidationResult TBTSapMessage::Validate()
    {
    TValidationResult retVal = EInvalidUnknown;
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: IsValid: %d %d >>"), iData.Length(),  iData[KMsgIDOffset]));

    if (iData.Length() < KParaLenModulo || (iData.Length() % KParaLenModulo) > 0)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: short or /4 ***")));
        retVal = EInvalidSegmented;
        }
    else if (!IsValidRequestID(TMsgID(iData[KMsgIDOffset])))
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: msgID wrong ***")));
        retVal = EInvalidMsgID;
        }
    else
        {
        TInt reserve = (iData[KParaNumOffset + 1] << 8) + iData[KParaNumOffset +2];

        if (reserve != KHeaderResvValue)
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: header rsvd not 0 ***")));
            retVal = EInvalidReservedBytes;
            }
        else
            {
            TInt paraNum = GetParaNum();
            TInt pos = KPayloadOffset;
            TInt valueLenPos;
            TInt valueLen = 0;
            TInt i = 0;

            for (; i < paraNum; i ++)
                {
                if (pos >= iData.Length())
                    {
                    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: paraNum big ***")));
                    retVal = EInvalidSegmented;
                    break;
                    }
                else if (!IsValidParaID(TParaID(iData[pos]), TMsgID(iData[KMsgIDOffset])))
                    {
                    // parameter ID is out of range
                    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: paraID wrong ***")));
                    retVal = EInvalidParameterID;
                    break;
                    }
                else if (iData[pos + KParaResvLen] != KParaResvValue)
                    {
                    // reserved field is not equal to 0
                    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: para rsvd not 0 ***")));
                    retVal = EInvalidReservedBytes;
                    break;
                    }
                else
                    {
                    valueLenPos = pos + KParaIDLen + KParaResvLen;
                    valueLen = (iData[valueLenPos]  << 8) + iData[valueLenPos + 1];
                    pos = valueLenPos + KParaLenLen + valueLen;

                    if (pos > iData.Length())
                        {
                        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: paraNum big ***")));
                        retVal = EInvalidSegmented;
                        break;
                        }

                    TInt paddingLen = (KParaLenModulo - (valueLen % KParaLenModulo)) % KParaLenModulo;

                    for (TInt k = 0; k < paddingLen; k ++)
                        {
                        if (iData[pos++] != KParaResvValue)
                            {
                            BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: padding not 0 ***")));
                            retVal = EInvalidPaddingBytes;
                            break;
                            }
                        }
                    }
                }

            if (i == paraNum)
                {
                if (pos == iData.Length())
                    {
                    // no problem so far
                    retVal = EValidFormat;
                    }
                else
                    {
                    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TBTSapMessage: paraNum small ***")));
                    retVal = EInvalidParaNumTooSmall;
                    }
                }

            if(paraNum == 0)
                {
                // Check if the message needs a parameter
                for(i = 0; i < sizeof(KMapParaID) / sizeof(KMapParaID[0]); i++)
                    {
                    if(KMapMsgID[i] == TMsgID(iData[KMsgIDOffset]))
                        {
                        retVal = EInvalidParaNumTooSmall;
                        break;
                        }
                    }
                }
            }
        }

    return retVal;
    }

TResultCode TBTSapMessage::ToResultCode(TInt aErrCode)
    {
    TUint size = sizeof(KMapErrCode) / sizeof(KMapErrCode[0]);
    TUint i = 0;
    for (; i < size; i++)
        {
        if (KMapErrCode[i] == aErrCode)
            {
            break;
            }
        }

    return (i < size ? KMapResultCode[i] : EResultCodeDataNotAvailable);
    }

TBool TBTSapMessage::IsValidParaID(const TParaID aParaID, const TMsgID aMsgID) const
    {
    TUint size = sizeof(KMapParaID) / sizeof(KMapParaID[0]);
    TUint i = 0;

    for (; i < size; i++)
        {
        if (KMapParaID[i] == aParaID)
            {
            break;
            }
        }

    return (i < size ? KMapMsgID[i] == aMsgID : EFalse);
    }

TBool TBTSapMessage::IsValidRequestID(const TMsgID aRequestID) const
    {
    TUint size = sizeof(KMapRequestID) / sizeof(KMapRequestID[0]);
    TUint i = 0;

    for (; i < size; i++)
        {
        if (KMapRequestID[i] == aRequestID)
            {
            break;
            }
        }

    return (i < size);
    }

TUint8 TBTSapMessage::GetParaNum()
    {
    return iData[KParaNumOffset];
    }

TInt TBTSapMessage::GetParaID(const TUint8 index, TParaID &aParaID)
    {
    TInt   retVal = KErrNotFound;
    TInt   pos = KPayloadOffset;
    TInt   valueLenPos = 0;
    TUint8 currentIndex = 0;
    TUint8 valueLen;

    while (pos < iData.Length())
        {
        valueLenPos = pos + KParaIDLen + KParaResvLen;
        valueLen = iData[valueLenPos]  * 0xff + iData[valueLenPos + 1];

        if (currentIndex == index)
            {
            aParaID = (TParaID) iData[pos];
            retVal = KErrNone;
            break;
            }

        TInt paddingLen = (KParaLenModulo - (valueLen % KParaLenModulo)) % KParaLenModulo;
        pos += KParaIDLen + KParaResvLen + KParaLenLen + valueLen + paddingLen;
        currentIndex++;
        }

    return retVal;
    }

//  End of File  
