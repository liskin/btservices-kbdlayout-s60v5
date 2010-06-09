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

#ifndef ATCOMMANDPARSER_H
#define ATCOMMANDPARSER_H

#include <e32std.h>


/**
 *  This class parses a given AT command into command type, command handler type and parameters.
 * 
 */
NONSHARABLE_CLASS(TAtCommandParser)
    {
public:
    /**
     * AT command handler type which determines how the command is interpreted.
     */
    enum TCommandHandlerType
        {
        ECmdHandlerTypeUndefined = KErrNotFound,
        ECmdHandlerTypeBase      = 0x01,  // For command "AT+COMMAND"
        ECmdHandlerTypeSet       = 0x02,  // For command "AT+COMMAND="
        ECmdHandlerTypeRead      = 0x04,  // For command "AT+COMMAND?"
        ECmdHandlerTypeTest      = 0x08,  // For command "AT+COMMAND=?"
        };
    
    /**
     * Extended AT commands supported by this parser.
     */
    enum TCommandType
        {
        EUnknown = KErrNotFound,
        ECmdAtCfun = 0,
        ECmdAtCbc,
        ECmdAtClck,
        ECmdAtCpwd,
        ECmdAtCpin,
        ECmdAtCusd,
        ECmdAtCnum,
        ECmdAtCmee
        };
public:
    TAtCommandParser();
    
    /**
     * @param aCmd AT command to be parsed
     */
    TAtCommandParser(const TDesC8& aCmd);
    
    /**
     * Parse a new AT command. Previously parsed AT command is lost.
     * @param aCmd AT command to be parsed
     */
    void ParseAtCommand(const TDesC8& aCmd);
    
    /**
     * @return AT command type @see CAtCommandParser::TCommandType 
     */
    TCommandType Command() const;
    
    /**
     * @return AT Command handler type @see CAtCommandParser::TCommandHandlerType
     */
    TCommandHandlerType CommandHandlerType() const;
    
    /**
     * @return Next available parameter. KNullDesC8 if no more parameters.
     */
    TPtrC8 NextParam();
    
    /**
     * Gets the integer value of the next parameter.
     * 
     * @param aValue the integer value of the parameter
     * @return Symbian system wide error codes
     */
    TInt NextIntParam(TInt& aValue);
    
    /**
     * Get the next text string without quote.
     * This function validate the parameter is a valid string.
     * if the parameter is absent, it returns KErrNotFound.
     * if the parameter is an invalid string such as not in a pair of double quotes, it returns KErrArgument
     * @param aParam the text string without quotes
     * @return  Symbian system wide error codes
     */
    TInt NextTextParam(TPtrC8& aParam);

    /**
     * Get ISA hash code for security code
     * Phone lock code is encoded by using ISA hash. This hash algorithm is implemented in remotemgmt component
     * This function is copied and modified from CSCPServer::HashISACode()
     * @param aPasscode passcode to be encoded
     * @param aHashCode encoded output
     * @return Symbian system wide error codes
     */
    TInt HashSecurityCode(const TDesC8& aPasscode, TDes8& aHashCode);
    
private:
    TLex8 iCmd;
    TCommandType iCmdType;
    TCommandHandlerType iCmdHandlerType;
    };

#endif // ATCOMMANDPARSER_H
