/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


// INCLUDE FILES
#include "obexutilsopaquedata.h"

// ================= MEMBER FUNCTIONS =======================
EXPORT_C CObexUtilsOpaqueData::CObexUtilsOpaqueData(const TDesC8& aDes):iData(aDes)
    {
    }


// -----------------------------------------------------------------------------
// GetString
// Get string from current location. Return error code if invalid string format.
// -----------------------------------------------------------------------------
EXPORT_C TInt CObexUtilsOpaqueData::GetString(TBuf8<KObexUtilsMaxOpaqueDataStringLen> &aString)
    {
    // reset the output string
    aString.Zero();

    if (iData.Eos())
        return KErrNotFound ;
    TChar c;
    TInt ret;
    while(c=iData.Get())
        {
        if(c=='\\')
            {
            switch(iData.Peek())
                {
            case 'x':
            case 'X':
                // parse hex number
                TUint8 val;
                iData.Inc();
                ret=iData.Val(val,EHex);
                if(ret == KErrNone)
                    aString.Append(val);
                else
                    return ret;
                break;
            case '|':
                iData.Inc();
                aString.Append('|');
                break;
            case '\\':
                iData.Inc();
                aString.Append('\\');
                break;
            default:
                // if not a decimal number, then bad format
                TInt8 dval;
                ret=iData.Val(dval);
                if(ret == KErrNone)
                    aString.Append(dval);
                else
                    return ret;
                }
            }
        else if(c=='|')
            {
            if(iData.Peek()=='|')
                {
                // delimiter "||" found, end of the string
                iData.Inc();
                break;
                }
            else
                aString.Append(c);
            }
        else
            aString.Append(c);
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// GetNumber
// Get number from current location. Return error code if invalid number format.
// -----------------------------------------------------------------------------
EXPORT_C TInt CObexUtilsOpaqueData::GetNumber(TUint &aNumber)
    {
    if (iData.Eos())
        return KErrNotFound ;

    //  skip dilimiter
    if(iData.Peek()=='|')
        {
        iData.Inc();
        if(iData.Peek()=='|')
            iData.Inc();
        else
            return KErrNotFound ;
        }

    if(iData.Peek()=='0')
        {
        iData.Inc();
        if(iData.Peek()=='x' || iData.Peek()=='X')
            {
            iData.Inc();
            return iData.Val(aNumber,EHex);
            }
        else
            {
            iData.UnGet();
            return iData.Val(aNumber,EOctal);
            }
        }
    else
        {
        return iData.Val(aNumber);
        }
    }

// -----------------------------------------------------------------------------
// IsString
// Peek if next entry is string by looking for a '\'.
// -----------------------------------------------------------------------------
EXPORT_C TBool CObexUtilsOpaqueData::IsString()
    {
    return iData.Peek()=='\\';
    }

