/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The parameter of an AT command. 
*
*/


// INCLUDE FILES

#include <s32strm.h>
#include "atcodec.h"
#include "debug.h"

const TUint KCharDoubleQuote = (TUint) '"';
_LIT8(KDesDoubleQuote, "\"");

// ================= MEMBER FUNCTIONS =======================

EXPORT_C TATParam::TATParam()
    : iValue(KNullDesC8), iValueInt(0), iType(EATNullParam)
    {
    }
    
EXPORT_C TATParam::TATParam(const TDesC8& aValue, TATParamType aType)
    : iValueInt(0), iType(aType)
    {
    iValue.Copy(aValue.Left(iValue.MaxLength()));   // to prevent panic for exceeding iValue length
    iValue.Trim();
    if (EATDQStringParam == aType)
        AddDQ();
    TRACE_INFO((_L8("param type %d, '%S'"), iType, &iValue))
    }

EXPORT_C TATParam::TATParam(TInt aValue)
    : iValueInt(aValue), iType(EATIntParam)
    {
    iValue.Num(aValue, EDecimal);
    TRACE_INFO((_L8("param type %d, '%S'"), iType, &iValue))
    }

// -----------------------------------------------------------------------------
// TATParam::Int
// -----------------------------------------------------------------------------
EXPORT_C TInt TATParam::Int(TInt& aValue) const
    {
    if (iType == EATIntParam)
        {
        aValue = iValueInt;
        return KErrNone;
        }
    return KErrNotFound;
    }

// -----------------------------------------------------------------------------
// TATParam::Str
// -----------------------------------------------------------------------------
EXPORT_C const TDesC8& TATParam::Des() const
    {
    return iValue;
    }

// -----------------------------------------------------------------------------
// TATParam::Type
// -----------------------------------------------------------------------------
EXPORT_C TATParamType TATParam::Type() const
    {
    return iType;
    }

// -----------------------------------------------------------------------------
// TATParam::SetValue
// -----------------------------------------------------------------------------
EXPORT_C TInt TATParam::SetValue(const TDesC8& aValue, TATParamType aType)
    {
    if (aValue.Length() > KMaxATParamSize)
        {
        return KErrArgument;
        }
        
    iType = aType;
    TInt err = KErrNone;
    switch (iType)
        {
        case EATNullParam:
            {
            iValue.Zero();
            break;
            }
        case EATIntParam:
            {
            TLex8 lex(aValue);
            err = lex.Val(iValueInt);
            if (!err)
                {
                iValue.Copy(aValue);
                iValue.Trim();
                }
            break;
            }
        case EATStringParam:
            {
            iValue.Copy(aValue);
            iValue.Trim();
            break;
            }
        case EATDQStringParam:
            {
            iValue.Copy(aValue);
            iValue.Trim();
            AddDQ();
            break;
            }
        }
    TRACE_INFO((_L8("param type %d, '%S'"), iType, &iValue))
    return err;
    }

// -----------------------------------------------------------------------------
// TATParam::SetValue
// -----------------------------------------------------------------------------
EXPORT_C void TATParam::SetInt(TInt aValue)
    {
    iValueInt = aValue;
    iValue.Num(aValue, EDecimal);
    iType = EATIntParam;
    }

EXPORT_C void TATParam::ExternalizeL(RWriteStream& aStream) const
    {
    aStream << iValue;
    aStream.WriteInt32L(KStreamVersion1);
    aStream.WriteInt32L(iValueInt);
    aStream.WriteInt32L(iType);
    }
        
EXPORT_C void TATParam::InternalizeL(RReadStream& aStream)
    {
    TInt32 version = aStream.ReadInt32L();
    
    if (version == KStreamVersion1)
        {
        aStream >> iValue;
        iValueInt = aStream.ReadInt32L();
        iType = static_cast<TATParamType>(aStream.ReadInt32L());
        }
    }

void TATParam::AddDQ()
    {
    if (iValue.Length())
        {
        if (iValue[0] != KCharDoubleQuote)
            {
            iValue.Insert(0, KDesDoubleQuote);
            }
        if (iValue[iValue.Length() - 1] != KCharDoubleQuote)
            {
            iValue.Append(KDesDoubleQuote);
            }
        }
    else
        {
        iValue.Append(KDesDoubleQuote);
        iValue.Append(KDesDoubleQuote);
        }
    }

// End of file
