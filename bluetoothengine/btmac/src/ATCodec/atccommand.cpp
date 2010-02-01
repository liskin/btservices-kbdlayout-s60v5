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
* Description:  Parses/Encodes AT ccommands. 
*
*/


// INCLUDE FILES

#include <s32strm.h>
#include "atcodec.h"
#include "ATCodecDefs.h"
#include "debug.h"

const TUint KDelimiterComma = (TUint) ',';

_LIT8(KDesEqualQuestion, "=?");
_LIT8(KDesEqual, "=");
_LIT8(KDesQuestion, "?");
_LIT8(KDesSemicolon, ";");
_LIT8(KDesAT,   "AT");
_LIT8(KDesColon, ": ");
_LIT8(KDesComma, ",");

_LIT8(KCRLFFormat, "\r\n%S\r\n");
const TInt KCRLFSize = 4;

// ================= MEMBER FUNCTIONS =======================

// Destructor
EXPORT_C CATBase::~CATBase()
    {
    TRACE_FUNC
    iText.Close();
    iParamList.Close();
    }

// -----------------------------------------------------------------------------
// CATBase::Id
// -----------------------------------------------------------------------------
//
EXPORT_C TATId CATBase::Id() const
    {
    return iId;
    }

// -----------------------------------------------------------------------------
// CATBase::Type
// -----------------------------------------------------------------------------
//
EXPORT_C TATType CATBase::Type() const
    {
    return iType;
    }
    
// -----------------------------------------------------------------------------
// CATBase::ParamNum
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CATBase::ParamNum() const
    {
    return iParamList.Count();
    }

// -----------------------------------------------------------------------------
// CATBase::Parameter
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CATBase::Parameter(TInt aIndex, TATParam& aParam) const
    {
    if (aIndex < 0 || aIndex >= iParamList.Count()) 
        {
        return KErrArgument;
        }
    aParam = iParamList[aIndex];
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CATBase::GetParameters
// -----------------------------------------------------------------------------
//
EXPORT_C const RATParamArray& CATBase::Parameters() const
    {
    return iParamList;
    }
 
// -----------------------------------------------------------------------------
// CATBase::Des
// -----------------------------------------------------------------------------
//
EXPORT_C const TDesC8& CATBase::Des() const
    {
    return iText;
    }

EXPORT_C void CATBase::ExternalizeL(RWriteStream& aStream) const
    {
    aStream << iText;
    aStream.WriteInt32L(KStreamVersion1);
    aStream.WriteInt32L(iId);
    aStream.WriteInt32L(iType);
    TInt count = iParamList.Count();
    aStream.WriteInt32L(count);
    for (TInt i = 0; i < count; i++)
        {
        aStream << iParamList[i];
        }
    }

EXPORT_C void CATBase::InternalizeL(RReadStream& aStream)
    {
    TInt32 version = aStream.ReadInt32L();
    
    if (version == KStreamVersion1)
        {
        iText.CreateL(KMaxATSize);
        aStream >> iText;
        iId = static_cast<TATId>(aStream.ReadInt32L());
        iType = static_cast<TATType>(aStream.ReadInt32L());
        TInt count = aStream.ReadInt32L();
        for (TInt i = 0; i < count; i++)
            {
            TATParam param;
            aStream >> param;
            iParamList.AppendL(param);
            }
        }
    }

// ----------------------------------------------------------------------------
// CATBase::CATBase
// ----------------------------------------------------------------------------
//
CATBase::CATBase()
    {
    TRACE_FUNC
    }


void CATBase::Reset()
    {
    iId = EUnknownAT;
    iType = EATUnkownType;
    iText.Close();
    iParamList.Reset();
    }

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CATCommand::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C CATCommand* CATCommand::NewL()
    {
    CATCommand* self = CATCommand::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CATCommand::NewLC
// -----------------------------------------------------------------------------
//
EXPORT_C CATCommand* CATCommand::NewLC()
    {
    CATCommand* self = new (ELeave) CATCommand;
    CleanupStack::PushL(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CATCommand::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C CATCommand* CATCommand::NewL(const TDesC8& aCmd)
    {
    CATCommand* self = CATCommand::NewLC(aCmd);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CATCommand::NewLC
// -----------------------------------------------------------------------------
//    
EXPORT_C CATCommand* CATCommand::NewLC(const TDesC8& aCmd)
    {
    CATCommand* self = new (ELeave) CATCommand;
    CleanupStack::PushL(self);
    TInt err = self->Set(aCmd);
    LEAVE_IF_ERROR(err);
    return self;
    }

// ----------------------------------------------------------------------------
// CATCommand::Set
// Only supports commands sent from BT audio accessory
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CATCommand::Set(const TDesC8& aText)
    {
    TRACE_FUNC_ENTRY;
    TRACE_INFO((_L8("'%S'"), &aText))
    Reset();
    TInt err = Parse(aText);
    if (err)
        {
        TRACE_INFO((_L("ret %d"), err))
        Reset();
        }
    TRACE_FUNC_EXIT;
    return err;
    }

// ----------------------------------------------------------------------------
// CATCommand::CATCommand
// ----------------------------------------------------------------------------
//
CATCommand::CATCommand()
    {
    TRACE_FUNC;
    Reset();
    }

// ----------------------------------------------------------------------------
// CATCommand::Parse
// ----------------------------------------------------------------------------
//
TInt CATCommand::Parse(const TDesC8& aText)
    {
    TRACE_FUNC;
    TInt err = iText.Create(aText);
    TRACE_INFO((_L("iText.Length(): %d"), iText.Length()));

    if (err != KErrNone)
        {
        return err;
        }

    iText.Trim();
    TPtrC8 at(KDesAT);
    if (iText.FindF(at) == 0)
        {
        // Command AT
        if (iText.CompareF(at) == 0)
            {
            iId = EAT;
            iType = EATTestCmd;
            return KErrNone;
            }
        TPtrC8 x;
        x.Set(iText.Mid(at.Length()));
        for (TInt i = 0; i < KATNameTableSize; i++)
            {
            TPtrC8 name(KATNameTable[i].KName);
            if (x.FindF(name) == 0)
                {
                iId = KATNameTable[i].KId;
                TRACE_INFO((_L("ATId %d"), iId))
                x.Set(x.Mid(name.Length()));
                ParseCommandType(x);
                TRACE_INFO((_L("ATType %d"), iType))
                if (iType == EATWriteCmd || iType == EATActionCmd)
                    {
                    for (TInt j = 0; j < KCommandParamTableSize; j++)
                        {
                        if (iId == KCommandParamTable[j].KId && iType == KCommandParamTable[j].KType)
                            {
                            return ParseParams(x, 0, j);
                            }
                        }
                    return x.Length();
                    }
                else
                    {
                    return x.Length();
                    }
                }
            }
        }
    return KErrArgument;
    }

// -----------------------------------------------------------------------------
// CATCommand::ParseCommandType
// -----------------------------------------------------------------------------
//
void CATCommand::ParseCommandType(TPtrC8& aPtrC)
    { 
    if (aPtrC.Find(KDesEqualQuestion) == 0)
        {
        aPtrC.Set(aPtrC.Mid(2));
        iType = EATTestCmd;
        }
    else if (aPtrC.Find(KDesEqual) == 0)
        {
        aPtrC.Set(aPtrC.Mid(1));
        iType = EATWriteCmd;
        }
    else if (aPtrC.Find(KDesQuestion) == 0)
        {
        aPtrC.Set(aPtrC.Mid(1));
        iType = EATReadCmd;
        }
    else
        {
        iType = EATActionCmd;
        }
     }

// -----------------------------------------------------------------------------
// CATCommand::ParseParamNum
// -----------------------------------------------------------------------------
//
TInt CATCommand::ParseParams(TPtrC8& aDes, TUint aParamIndex, TUint aTableIndex)
    {
    TInt ret = KErrNone;    
    while( !(aParamIndex > KCommandParamTable[aTableIndex].KParamNum || 
        aDes.Length() == 0 || aDes.Find(KDesSemicolon) == 0) )
        {        
		    TBuf8<KMaxATParamSize> paramDes(aDes);
		    TInt pos = aDes.Locate(KDelimiterComma);
		    if (pos < 0)
		        {
		        pos = aDes.Find(KDesSemicolon);
		        }   
		    if (pos > 0)
		        {
		        paramDes = aDes.Left(pos);
		        }
		    paramDes.Trim();
		    if (paramDes.Length())
		        {
		        TATParamType type = EATNullParam;
		        type = (TATParamType) KCommandParamTable[aTableIndex].KParamTypes[aParamIndex];
		        TATParam param;
		        ret = param.SetValue(paramDes, type);
		        if (ret)
		            {
		            break;
		            }
		        ret = iParamList.Append(param);
		        if (ret)
		            {
		            break;
		            }
		        }
		    if (pos == KErrNotFound || pos == aDes.Length() - 1) 
		        {
		        break;
		        }
		    else
		        {
		        aDes.Set(aDes.Mid(++pos));
		        aParamIndex++;
		        }
      }
    return ret;
    }

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CATResult::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C CATResult* CATResult::NewL()
    {
    CATResult* self = CATResult::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CATResult::NewLC
// -----------------------------------------------------------------------------
//
EXPORT_C CATResult* CATResult::NewLC()
    {
    CATResult* self = new (ELeave) CATResult;
    CleanupStack::PushL(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CATResult::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C CATResult* CATResult::NewL(TATId aId, TATType aType, const RATParamArray* aParams)
    {
    CATResult* self = CATResult::NewLC(aId, aType, aParams);
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CATResult::NewLC
// -----------------------------------------------------------------------------
//    
EXPORT_C CATResult* CATResult::NewLC(TATId aId, TATType aType, const RATParamArray* aParams)
    {
    CATResult* self = new (ELeave) CATResult;
    CleanupStack::PushL(self);
    TInt err = self->Set(aId, aType, aParams);
    LEAVE_IF_ERROR(err);
    return self;
    }

// -----------------------------------------------------------------------------
// CATResult::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C CATResult* CATResult::NewL(TATId aId, TATType aType, const TATParam& aParam)
    {
    CATResult* self = CATResult::NewLC(aId, aType, aParam);
    CleanupStack::Pop(self);
    return self;
    }
    
// -----------------------------------------------------------------------------
// CATResult::NewLC
// -----------------------------------------------------------------------------
//    
EXPORT_C CATResult* CATResult::NewLC(TATId aId, TATType aType, const TATParam& aParam)
    {
    CATResult* self = new (ELeave) CATResult;
    CleanupStack::PushL(self);
    RATParamArray array;
    TInt err = array.Append(aParam);
    if (!err)
        {
        err = self->Set(aId, aType, &array);
        }
    array.Close();
    LEAVE_IF_ERROR(err);
    return self;
    }

    
// ----------------------------------------------------------------------------
// CATResult::Set
// Only supports result code sent from BT audio gateway
// ----------------------------------------------------------------------------
//
EXPORT_C TInt CATResult::Set(TATId aId, TATType aType, const RATParamArray* aParams)
    {
    TRACE_FUNC_ENTRY;
    Reset();
    TInt err = Parse(aId, aType, aParams);
    if (err)
        {
        TRACE_INFO((_L("ret %d"), err))
        Reset();
        }
    else
    	{
    	AddCRLF(iText);
    	}
    TRACE_FUNC_EXIT;
    return err;
    }

// ----------------------------------------------------------------------------
// CATResult::CATResult
// ----------------------------------------------------------------------------
//
CATResult::CATResult()
    {
    TRACE_FUNC;
    Reset();
    }

// ----------------------------------------------------------------------------
// CATResult::Set
// Only supports result code sent from BT audio gateway
// ----------------------------------------------------------------------------
//
TInt CATResult::Parse(TATId aId, TATType aType, const RATParamArray* aParams)
    {
    TRACE_FUNC
    TInt err = iText.Create(KMaxATSize);

    if (err != KErrNone)
        {
        return err;
        }

    TRACE_INFO((_L("ATId %d, ATType %d"), aId, aType))
    for (TInt i = 0; i < KATNameTableSize; i++)
        {
        if (aId == KATNameTable[i].KId)
            {
            // Validate parameters
            for (TInt j = 0; j < KResultCodeParamTableSize; j++)
                {
                if (KResultCodeParamTable[j].KId == aId && KResultCodeParamTable[j].KType == aType)
                    {
                    if (!aParams || aParams->Count() < KResultCodeParamTable[j].KParamNum)
                        {
                        return KErrArgument;
                        }
                    else
                        {
                        for (TInt k = 0; k < KResultCodeParamTable[j].KParamNum; k++)
                            {
                            if ((*aParams)[k].Type() != KResultCodeParamTable[j].KParamTypes[k])
                                {
                                return KErrArgument;
                                }
                            }
                        }
                    }
                }
            
            
            iId = aId;
            iType = aType;
            iText.Copy(KATNameTable[i].KName);
            if (aParams && aParams->Count())
                {
                TPtrC8 colon(KDesColon);
                TPtrC8 comma(KDesComma);
                iText.Append(colon);
                for (TInt j = 0; j < aParams->Count(); j++)
                    {
                    TInt err = iParamList.Append((*aParams)[j]);
                    if (err)
                        {
                        return err;
                        }
                    if (iText.Length() + (*aParams)[j].Des().Length() > KMaxATSize)
                        {
                        return KErrArgument;
                        }
                    iText.Append((*aParams)[j].Des());
                    if (j != aParams->Count() - 1)
                        {
                        if (iText.Length() + comma.Length() > KMaxATSize)
                            {
                            return KErrArgument;
                            }
                        iText.Append(KDelimiterComma);
                        }
                    }
                }
            return KErrNone;
            }
        }
    return KErrArgument;
    }

    
TInt CATResult::AddCRLF(TDes8& aText)
    {
    if (aText.Length() + KCRLFSize > KMaxATSize)
        {
        return KErrOverflow;
        }
    TBuf8<KMaxATSize> buf(aText);
    aText.Format(KCRLFFormat, &buf);
    return KErrNone;
    }

void ATCmdArrayResetAndDestroyAndClose(TAny* aPtr)
    {
    reinterpret_cast<RATResultPtrArray*>(aPtr)->ResetAndDestroy();
    reinterpret_cast<RATResultPtrArray*>(aPtr)->Close();
    }

EXPORT_C void ATObjArrayCleanupResetAndDestroyPushL(RATResultPtrArray& aArray)
    {        
    TCleanupItem item(ATCmdArrayResetAndDestroyAndClose, &aArray);
    CleanupStack::PushL(item);
    }


// End of file
