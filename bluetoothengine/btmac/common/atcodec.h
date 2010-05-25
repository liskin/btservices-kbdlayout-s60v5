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
* Description:  Parses/Encodes AT ccommands.
*
*/


#ifndef AT_CODEC_H
#define AT_CODEC_H

//  INCLUDES
#include <e32base.h>
#include <atext.h>

enum TATId
    {
    EUnknownAT,// 0 Not known AT command
    EAT,       // 1, AT test command
    EATA,      // 2, answer a call
    EATBINP,   // 3, Command used for requesting some specific data input from the AG
    EATBLDN,   // 4, Command used for calling the last phone number dialed
    EATBRSF,   // 5, Bluetooth Retrieve Supported Features
    EATBVRA,   // 6, Bluetooth Voice Recognition Activation
    EATCCWA,   // 7, call waiting notification
    EATCHLD,   // 8, multiparty handling AT command
    EATCHUP,   // 9, Hang-up AT command
    EATCIND,   // 10, standard indicator update AT command (read and test only)
    EATCKPD,   // 11, Command issued by HS to indicate that the button has been pressed
    EATCLCC,   // 12, list of current call
    EATCLIP,   // 13, standard Calling Line Identification notification activation AT command
    EATCMEE,   // 14, Advanced error code activation
    EATCMER,   // 15, Standard event reporting activation AT command
    EATCNUM,   // 16, Subscriber number
    EATCOPS,   // 17, Operator selection
    EATD1,     // 18, place a call
    EATD2,     // 19, speed dial
    EATNREC,   // 20, Noise Reduction and Echo Canceling
    EATVGM,    // 21, Microphone volume
    EATVGS,    // 22, Speaker volume
    EATVTS,    // 23, Standard DTMF generation AT command
    EATCIEV,     // 24, notify call status change
    EATERROR,    // 25, Response Error
    EATOK,       // 26, Response OK
    EATRING,     // 27, RING unsolicited response
    EATBIA,		// 28, Set indicator status
    EATCREG,	// 29, Network registration
    EATCGSN,	// 30, Serial Number
    EATCSQ,		// 31, Signal Strength in dBm
    EATCIMI,	// 32, Mobile subscriber ID
    EATCGMI,	// 33, Manufacturer information
    EATCGMM,	// 34, Model id
    EATCGMR,	// 35, Revision id
    EATCOLP,	// 36, Outgoin call number
    EMinValidATIdRange = EAT,
    EMaxValidATIdRange = EATCOLP,
    };

enum TATType
    {
    EATUnkownType = 0,
    EATReadCmd =           0x001, // 1, Read format "AT+xxxx?"
    EATWriteCmd =          0x002, // 2, Write format "AT+xxxx=1"
    EATActionCmd =         0x004, // 4, Action format "AT+xxxx" or "ATx" (e.g. "ATD")   
    EATTestCmd =           0x008, // 8, Test command "AT+xxxx=?" and "AT"
    EATReadResult =        0x010, // 16, Response to read command
    EATWriteResult =       0x020, // 32, Response to write command
    EATActionResult =      0x040, // 64, Response to action command
    EATTestResult =        0x080, // 128, Response to test command
    EATUnsolicitedResult = 0x100, // 256, Unsolicited result code
    EATErrorResult =       0x200  // 512, error result, OK or ERROR    
    };

enum TATParamType
    {
    EATNullParam,
    EATStringParam,
    EATDQStringParam,
    EATIntParam
    };
    
const TInt KMaxATParamSize = 256;

class RReadStream;
class RWriteStream;

// CLASS DECLARATION
/**
*  TATParam represents one AT command parameter.
*/
NONSHARABLE_CLASS(TATParam)
    {
public:
    IMPORT_C TATParam();
    
    IMPORT_C TATParam(const TDesC8& aValue, TATParamType aType = EATStringParam);
    
    IMPORT_C TATParam(TInt aValue);

    /**
    * Returns the parameter as integer.
    * @since 3.0
    * @return error code.
    */
    IMPORT_C TInt Int(TInt& aValue) const;

    /**
    * Returns the parameter as text.
    * @since 3.0
    * @return the non-modifiable pointer descriptor
    */
    IMPORT_C const TDesC8& Des() const;
    
    /**
    * Returns the parameter type.
    * @since 3.0
    * @param None
    * @return Parameter type.
    */
    IMPORT_C TATParamType Type() const;
    
    /**
    * Sets the parameter value.
    * @since 3.0
    * @param aValue New value.
    * @return error code
    */
    IMPORT_C TInt SetValue(const TDesC8& aValue, TATParamType aType);

    /**
    * Sets the parameter value.
    * @since 3.0
    * @param aValue New value.
    * @return None
    */
    IMPORT_C void SetInt(TInt aValue);
    
    IMPORT_C void ExternalizeL(RWriteStream& aStream) const;
    
    IMPORT_C void InternalizeL(RReadStream& aStream);
    
private:
    void AddDQ();
    
private:
    TBuf8<KMaxATParamSize> iValue;
    TInt iValueInt;
    TATParamType iType;
    
private:
    static const TInt32 KStreamVersion1 = 0x00000001;
    };

typedef RArray<TATParam> RATParamArray;

/**
*  TATCommand is used for parsing Bluetooth AT commands.
*/
NONSHARABLE_CLASS(CATBase) : public CBase
    {
public:
    IMPORT_C virtual ~CATBase();

    /**
    * Returns AT command identification.
    * @since 3.0
    * @param None
    * @return Id number.
    */
    IMPORT_C TATId Id() const;

    /**
    * Returns AT command type.
    * @since 3.0
    * @param None
    * @return Command type.
    */
    IMPORT_C TATType Type() const;     

    /**
    * Returns the number of command parameters.
    * @since 3.0
    * @param None
    * @return Number of command paramteres.
    */
    IMPORT_C TInt ParamNum() const;

    /**
    * Returns parameter at given index.
    * @since 3.0
    * @param aIndex Index.
    * @return the parameter.
    */
    IMPORT_C TInt Parameter(TInt aIndex, TATParam& aParam) const;

    /**
    * Returns parameters in a string.
    * @since 3.0
    * @return the parameter list.
    */
    IMPORT_C const RATParamArray& Parameters() const;

    /**
    * Gets the descriptor of this AT command or response. 
    */
    IMPORT_C const TDesC8& Des() const;

    IMPORT_C void ExternalizeL(RWriteStream& aStream) const;
    
    IMPORT_C void InternalizeL(RReadStream& aStream);
    
protected:
    CATBase();

    void Reset();
    
protected:
    RBuf8 iText;
    TATId iId;
    TATType iType;
    RATParamArray iParamList;
    
private:
    static const TInt32 KStreamVersion1 = 0x00000001;
    };

/**
*  TATCommand is used for parsing Bluetooth AT commands.
*/
NONSHARABLE_CLASS(CATCommand) : public CATBase
    {
public:
    /**
    * Instantiates an uninitialized AT command.
    */
    IMPORT_C static CATCommand* NewL();
    
    /**
    * Instantiates an uninitialized AT command and leaves it in cleanupstack.
    */        
    IMPORT_C static CATCommand* NewLC();
    
    /**
    * Instantiates an AT command. 
    * Function leaves if aCmd is not supported or not valid.
    */
    IMPORT_C static CATCommand* NewL(const TDesC8& aCmd);
    
    /**
    * Instantiates an AT command and leaves it in cleanupstack. 
    * Function leaves if aCmd is not supported or not valid.
    */
    IMPORT_C static CATCommand* NewLC(const TDesC8& aCmd);
            
    /**
    * 
    * @since 3.0
    * @param aText Command string
    * @return 
    */
    IMPORT_C TInt Set(const TDesC8& aText);

private:
    CATCommand();
    
    TInt Parse(const TDesC8& aText);
    
    void ParseCommandType(TPtrC8& aPtrC);
    
    TInt ParseParams(TPtrC8& aDes, TUint aParamIndex, TUint aTableIndex);
    };

/**
*  TATCommand is used for parsing Bluetooth AT commands.
*/
NONSHARABLE_CLASS(CATResult) : public CATBase
    {
public:
    /**
    * Instantiates an uninitialized AT command.
    */
    IMPORT_C static CATResult* NewL();
    
    /**
    * Instantiates an uninitialized AT command and leaves it in cleanupstack.
    */        
    IMPORT_C static CATResult* NewLC();
    
    IMPORT_C static CATResult* NewL(TATId aId, TATType aType = EATErrorResult, const RATParamArray* aParams = NULL);
    
    IMPORT_C static CATResult* NewLC(TATId aId, TATType aType = EATErrorResult, const RATParamArray* aParams = NULL);

    IMPORT_C static CATResult* NewL(TATId aId, TATType aType, const TATParam& aParam);
    
    IMPORT_C static CATResult* NewLC(TATId aId, TATType aType, const TATParam& aParam);

    /**
    * 
    */
    IMPORT_C TInt Set(TATId aId, TATType aType = EATErrorResult, const RATParamArray* aParams = NULL);

private:
    CATResult();
    
    TInt Parse(TATId aId, TATType aType, const RATParamArray* aParams);
    
    /**
    * Adds carriage return and line feed characters at
    * the beginning and the end of the string.
    * @since 3.0
    * @param aText String to be modified.
    * @return None
    */
    static TInt AddCRLF(TDes8& aText);
    };

typedef RPointerArray<CATResult> RATResultPtrArray;

IMPORT_C void ATObjArrayCleanupResetAndDestroyPushL(RATResultPtrArray& aArray);

#endif  // AT_CODEC_H

// End of File
