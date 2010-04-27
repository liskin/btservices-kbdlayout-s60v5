// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// This file contains the implementation of the AT command parser and common utilities
// 
//

#include "atcommandparser.h"
#include <hash.h>

#include "debug.h"


// Max buffer length for an MD5 digest
const TInt KSCPMaxHashLength( 32 );


TAtCommandParser::TAtCommandParser()
    : iCmdType(EUnknown), iCmdHandlerType(ECmdHandlerTypeUndefined)  
    {
    }

TAtCommandParser::TAtCommandParser(const TDesC8& aCmd)
    : iCmdType(EUnknown), iCmdHandlerType(ECmdHandlerTypeUndefined)  
    {
    ParseAtCommand(aCmd);
    }


void TAtCommandParser::ParseAtCommand(const TDesC8& aCmd)
    {
    TRACE_FUNC_ENTRY
    iCmd.Assign(aCmd);
    iCmd.Mark();
    TChar c = 0;
    // First extract the AT command "AT+COMMAND"
    while(!iCmd.Eos() && !(c=='=' || c=='?'))
        {
        iCmd.Inc();
        c = iCmd.Peek();
        }
          
    TPtrC8 token = iCmd.MarkedToken();   
    
    _LIT8(KAtCfun, "AT+CFUN");
    _LIT8(KAtCbc, "AT+CBC");
    _LIT8(KAtClck, "AT+CLCK");
    _LIT8(KAtCpwd, "AT+CPWD");
    _LIT8(KAtCpin, "AT+CPIN");
    _LIT8(KAtCusd, "AT+CUSD");
    _LIT8(KAtCnum, "AT+CNUM");
    
    Trace(KDebugPrintS, "token: ", &token);
    // Determine the AT command type
    if(!token.Compare(KAtCfun))
        {
        iCmdType = ECmdAtCfun;
        }
    else if(!token.Compare(KAtCbc))
        {
        iCmdType = ECmdAtCbc;
        }
    else if(!token.Compare(KAtClck))
        {
        iCmdType = ECmdAtClck;
        }
    else if(!token.Compare(KAtCpwd))
        {
        iCmdType = ECmdAtCpwd;
        }
    else if(!token.Compare(KAtCpin))
        {
        iCmdType = ECmdAtCpin;
        }
    else if(!token.Compare(KAtCusd))
        {
        iCmdType = ECmdAtCusd;
        }
    else if(!token.Compare(KAtCnum))
        {
        iCmdType = ECmdAtCnum;
        }
    else
        {
        iCmdType = EUnknown;
        TRACE_FUNC_EXIT
        return;
        }
        
    // Now find out the AT command handler type
    if(iCmd.Eos())
        {
        iCmdHandlerType = ECmdHandlerTypeBase;
        }
    else if(iCmd.Peek() == '?')
        {
        iCmdHandlerType = ECmdHandlerTypeRead;
        }
    else if(iCmd.Peek() == '=')
        {
        iCmd.Inc();
        if(iCmd.Peek() == '?')
            {
            iCmd.Inc();
            iCmdHandlerType = ECmdHandlerTypeTest;
            }
        else
            {
            iCmdHandlerType = ECmdHandlerTypeSet;
            }
        }
    else
        {
        iCmdHandlerType = ECmdHandlerTypeUndefined;
        }
    TRACE_FUNC_EXIT
    }

TAtCommandParser::TCommandType TAtCommandParser::Command() const
    {
    return iCmdType;
    }

TAtCommandParser::TCommandHandlerType TAtCommandParser::CommandHandlerType() const
    {
    return iCmdHandlerType;
    }

TPtrC8 TAtCommandParser::NextParam()
    {
    TRACE_FUNC_ENTRY
    iCmd.SkipSpace(); // Skip front spaces
    iCmd.Mark();
    TChar chr = 0;
    
    if(!iCmd.Eos())
        {
        chr = iCmd.Peek();
        while(!iCmd.Eos() && chr != ',' && !chr.IsSpace() && !chr.IsControl())
            {// Stop at any of those chars: comma, space or control
            iCmd.Inc();
            chr = iCmd.Peek();
            }
        }
    
    // Extract the token at this point            
    TPtrC8 retVal = iCmd.MarkedToken();
    
    // Skip comma, space and control chars
    while(!iCmd.Eos() && (chr == ',' || chr.IsSpace() || chr.IsControl()))
        {
        iCmd.Inc();
        chr = iCmd.Peek();
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

TPtrC8 TAtCommandParser::NextTextParam(TInt& aError)
    {
    TPtrC8 param = NextParam();
    
    if (param.Compare(KNullDesC8) == 0)
        {
        aError = KErrNotFound;
        return param; 
        }
    
    if(param.Length()<2 
            || param[0] != '"'
            || param[param.Length()-1] != '"')
        {
        aError = KErrArgument;
        return param.Left(0);
        }
    aError = KErrNone;
    return param.Mid(1, param.Length() - 2);
    }

TInt TAtCommandParser::NextIntParam(TInt& aValue)
    {
    TRACE_FUNC_ENTRY
    TInt retVal =KErrNone;
    TPtrC8 param = NextParam();
    if (param.Compare(KNullDesC8) == 0)
        {
        retVal = KErrNotFound;
        }
    else
        {
        TLex8 lex(param);
        retVal = lex.Val(aValue);
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

TInt TAtCommandParser::HashSecurityCode(const TDesC8& aPasscode, TDes8& aHashCode)
    {
    TRACE_FUNC_ENTRY
    TInt ret = KErrNone;

    // Get MD5 Hash
    // see remotemgmt component CSCPServer::HashISACode() for encoding algorithm
    CMD5* hashObject = NULL;
    TRAP( ret, hashObject = CMD5::NewL() );

    if(ret != KErrNone)
        {
        TRACE_FUNC_EXIT
        return ret;
        }
    
    RBuf pwdBuffer;
    ret = pwdBuffer.Create(KSCPMaxHashLength);
    if(ret != KErrNone)
        {
        delete hashObject;
        TRACE_FUNC_EXIT
        return ret;
        }
    pwdBuffer.Copy(aPasscode); // convert to TDes16

    // add TDes16 to a binary buffer
    TUint16* inputPtr = const_cast<TUint16*>( pwdBuffer.Ptr() ); 
    TPtrC8 inputData( reinterpret_cast<TUint8*>(inputPtr), pwdBuffer.Length()*2 );
    
    TPtrC8 hash = hashObject->Final( inputData );
    delete hashObject;

    pwdBuffer.Zero();
    pwdBuffer.Copy(hash);

    // Compute the hash sum as four 32-bit integers.
    TInt64 hashSum = *(reinterpret_cast<TInt32*>(&pwdBuffer[0])) + 
                     *(reinterpret_cast<TInt32*>(&pwdBuffer[4])) + 
                     *(reinterpret_cast<TInt32*>(&pwdBuffer[8])) +
                     *(reinterpret_cast<TInt32*>(&pwdBuffer[12]));
    pwdBuffer.Close();
     
    // Create a five-digit security code from this number
    TInt isaCode = ( hashSum % 90000 ) + 10000;

    // save encoded security code to TDes
    aHashCode.Zero();
    aHashCode.AppendNum(isaCode);

    TRACE_FUNC_EXIT
    return KErrNone;
    }

