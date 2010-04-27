/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Main handler for incoming requests
*
*/

#ifndef ATMISCCMDPLUGIN_H
#define ATMISCCMDPLUGIN_H

#include <atextpluginbase.h>
#include <etelmm.h>

#include "atcommandparser.h"

/** Character types: carriage return, line feed or backspace */
enum TCharacterTypes
    {
    ECharTypeCR,  // Carriage return
    ECharTypeLF,  // Line feed
    ECharTypeBS   // Backspace
    };

/** Type of modes (quiet, verbose) */
enum TModeTypes
    {
    EModeTypeQuiet,   // Quiet mode
    EModeTypeVerbose  // Verbose mode
    };


class MATMiscCmdPlugin;


/**
 *  Class for common AT command handler interface
 */
NONSHARABLE_CLASS( CATCmdAsyncBase ) : public CActive 
    {
public:
    virtual ~CATCmdAsyncBase() {};
    CATCmdAsyncBase(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);

    /**
     * @see CATExtPluginBase::HandleCommand
     */
    virtual void HandleCommand( const TDesC8& aCmd,
                                RBuf8& aReply,
                                TBool aReplyNeeded ) = 0;

    /**
     * @see CATExtPluginBase::HandleCommandCancel
     */
    virtual void HandleCommandCancel() = 0;

protected:
    MATMiscCmdPlugin* iCallback;
    TAtCommandParser& iATCmdParser;
    RMobilePhone& iPhone;
    };

/**
 *  Class for accessing plugin information and common functionality
 */
NONSHARABLE_CLASS( MATMiscCmdPlugin )
    {
public:
    /**
     * Creates an AT command reply based on the reply type and completes the
     * request to ATEXT.
     *
     * @param aReplyType Type of reply
     * @param aSrcBuffer Source buffer; used only if aReplyType is EReplyTypeOther
     * @param aError Completion code. If not KErrNone then other arguments are
     *               ignored and the request is completed to ATEXT with
     *               EReplyTypeUndefined.
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt CreateReplyAndComplete( TATExtensionReplyType aReplyType,
                                         const TDesC8& aSrcBuffer=KNullDesC8,
                                         TInt aError=KErrNone ) = 0;

    /**
     * Creates a buffer for "OK" or "ERROR" reply based on the line settings
     *
     * @param aReplyBuffer Destination buffer for the created reply
     * @param aOkReply ETrue if "OK" reply needed,
     *                 EFalse if "ERROR" reply needed
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt CreateOkOrErrorReply( RBuf8& aReplyBuffer,
                                       TBool aOkReply ) = 0;

    /*
     * Complete the AT command request with AT CME error code according to given Symbian error code
     * @param aError Symbian error code 
     */
    virtual void CreateCMEReplyAndComplete(TInt aError) = 0;

    /**
     * Returns plugin's character value settings (from CATExtPluginBase)
     *
     * @param aCharType Character's type
     * @param aChar Character's value matching aCharType
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt GetCharacterValue( TCharacterTypes aCharType, TChar& aChar ) = 0;

    /**
     * Returns plugin's mode value settings (from CATExtPluginBase)
     *
     * @param aModeType Mode type
     * @param aMode Mode value matching aModeType
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt GetModeValue( TModeTypes aModeType, TBool& aMode ) = 0;
    
    /**
     * Returns plugin's response to an unsolicited request
     *
     * @param aAT the response to display to the user from unsolicited event
     * @return Symbian error code on error, KErrNone otherwise
     */  
    virtual TInt HandleUnsolicitedRequest(const TDesC8& aAT ) = 0;
    };

/**
 *  Class for selecting handlers for different AT commands
 */
NONSHARABLE_CLASS( CATMiscCmdPlugin ) : public CATExtPluginBase,
                                       public MATMiscCmdPlugin
    {
public:
    static CATMiscCmdPlugin* NewL();
    virtual ~CATMiscCmdPlugin();

private: // methods from base class CATExtPluginBase
    void ReportConnectionName( const TDesC8& aName );
    TBool IsCommandSupported( const TDesC8& aCmd );
    void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    void HandleCommandCancel();
    TInt NextReplyPartLength();
    TInt GetNextPartOfReply( RBuf8& aNextReply );
    void ReceiveUnsolicitedResult();
    void ReceiveUnsolicitedResultCancel();
    void ReportNvramStatusChange( const TDesC8& aNvram );
    void ReportExternalHandleCommandError();
    
private: // methods from base class MATMiscCmdPlugin
    virtual TInt CreateReplyAndComplete( TATExtensionReplyType aReplyType,
                                 const TDesC8& aSrcBuffer=KNullDesC8,
                                 TInt aError=KErrNone );

    virtual TInt CreateOkOrErrorReply( RBuf8& aReplyBuffer,
                               TBool aOkReply );
    virtual void CreateCMEReplyAndComplete(TInt aError);
    virtual TInt GetCharacterValue( TCharacterTypes aCharType, TChar& aChar );
    virtual TInt GetModeValue( TModeTypes aModeType, TBool& aMode );    
    virtual TInt HandleUnsolicitedRequest(const TDesC8& aAT );
    
private:    
    CATMiscCmdPlugin();
    void ConstructL();
    
    TInt CreatePartOfReply( RBuf8& aBuffer );
    
    /*
     * Utility function that connect to Etel server and establish a subsession to RMobilePhone
     * Caller must close session and subsession after use.
     * @param aTelServer returns session to ETel
     * @param aPhone returns RMobilePhone subsession
     */
    void ConnectToEtelL(RTelServer& aTelServer, RMobilePhone& aPhone);


private:
    TAtCommandParser iCommandParser;
    RTelServer iTelServer;
    RMobilePhone iPhone;
    /**
     * Current AT command handler in.
     * Used when IsCommandSupported() detects a matching handler class.
     */
    CATCmdAsyncBase* iCurrentHandler;
    
    CATCmdAsyncBase* iCLCKHandler;
    CATCmdAsyncBase* iCPWDHandler;
    CATCmdAsyncBase* iCPINHandler;
    CATCmdAsyncBase* iCUSDHandler;
    CATCmdAsyncBase* iCNUMHandler;
    CATCmdAsyncBase* iCFUNHandler;
    CATCmdAsyncBase* iCBCHandler;
    
    /**
     * Buffer for handle command's command
     * Not own.
     */
    const TDesC8* iHcCmd;

    /**
     * Buffer for handle command reply
     * Not own.
     */
    RBuf8* iHcReply;

    /**
     * Global reply buffer for the AT command replies
     */
    RBuf8 iReplyBuffer;
    };

#endif  // ATMISCCMDPLUGIN_H

