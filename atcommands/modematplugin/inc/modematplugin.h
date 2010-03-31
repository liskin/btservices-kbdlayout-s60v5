/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef C_MODEMATPLUGIN_H
#define C_MODEMATPLUGIN_H

#include <atextpluginbase.h>
#include "cmdpluginobserver.h" 


/**
 *  Class for common AT command handler interface
 *
 *  @since S60 TB9.2 
 */
NONSHARABLE_CLASS( CATCommandHandlerBase )
    {

public:

    virtual ~CATCommandHandlerBase() {};

    /**
     * Reports the support status of an AT command. This is a synchronous API.
     *
     * @param aCmd The AT command. Its format may vary depending on the
     *             specification. E.g. in BT HFP case, the command may contain
     *             a character carriage return (<cr>) in the end.
     * @return ETrue if the command is supported; EFalse otherwise.
     */
    virtual TBool IsCommandSupported( const TDesC8& aCmd ) = 0;

    /**
     * Handles an AT command. Cancelling of the pending request is done by
     * HandleCommandCancel(). The implementation in the extension plugin should
     * be asynchronous.
     *
     * The extension plugin which accepts this command is responsible to supply
     * the result codes and response and to format result codes properly, e.g.
     * in BT HFP case, the format should be <cr><lf><result code><cr><lf>
     *
     * After an extension plugin has handled or decided to reject the given AT
     * command, it must inform ATEXT by HandleCommandCompleted() with a proper
     * error code.
     *
     * @since S60 TB9.2
     * @param aCmd The AT command to be handled. Its format may vary depending
     *             on the specification. E.g. in BT HFP case, the command may
     *             contain a character carriage return (<cr>) in the end.
     * @param aReply When passed in, contains the built in answer filled by
     *               ATEXT if it is not empty; when command handling completes
     *               successfully, contains the result codes and responses to
     *               this command; Its ownership always belongs to ATEXT, plugin
     *               may reallocate its space when needed.
     * @param aReplyNeeded Reply needed if ETrue, no reply otherwise. If EFalse,
     *                     the aReply must not contain the reply, otherwise it
     *                     must contain verbose or numeric reply (ATV0/1) or an
     *                     empty string reply (with ATQ).
     * @return None
     */
    virtual void HandleCommand( const TDesC8& aCmd,
                                RBuf8& aReply,
                                TBool aReplyNeeded ) = 0;

    /**
     * Cancels a pending HandleCommand request.
     *
     * @since S60 TB9.2
     * @return None
     */
    virtual void HandleCommandCancel() = 0;

    };


/**
 *  Class for selecting handlers for different AT commands
 *
 *  @since TB9.2 
 */
NONSHARABLE_CLASS( CModemAtPlugin ) : public CATExtPluginBase, public MCmdPluginObserver 
    {

public:

    /**
     * Two-phased constructor.
     * @return Instance of self
     */
    static CModemAtPlugin* NewL();

    /**
    * Destructor.
    */
    virtual ~CModemAtPlugin();

private:

    CModemAtPlugin();

    void ConstructL();

    /**
     * Reports connection identifier name to the extension plugin.
     *
     * @since TB9.2 
     * @param aName Connection identifier name
     * @return None
     */
    void ReportConnectionName( const TDesC8& aName );

    /**
     * Reports the support status of an AT command. This is a synchronous API.
     *
     * @param aCmd The AT command. Its format may vary depending on the
     *             specification. E.g. in BT HFP case, the command may contain
     *             a character carriage return (<cr>) in the end.
     * @return ETrue if the command is supported; EFalse otherwise.
     */
    TBool IsCommandSupported( const TDesC8& aCmd );

    /**
     * Handles an AT command. Cancelling of the pending request is done by
     * HandleCommandCancel(). The implementation in the extension plugin should
     * be asynchronous.
     *
     * The extension plugin which accepts this command is responsible to supply
     * the result codes and response and to format result codes properly, e.g.
     * in BT HFP case, the format should be <cr><lf><result code><cr><lf>
     *
     * After an extension plugin has handled or decided to reject the given AT
     * command, it must inform ATEXT by HandleCommandCompleted() with a proper
     * error code.
     *
     * @since TB9.2 
     * @param aCmd The AT command to be handled. Its format may vary depending
     *             on the specification. E.g. in BT HFP case, the command may
     *             contain a character carriage return (<cr>) in the end.
     * @param aReply When passed in, contains the built in answer filled by
     *               ATEXT if it is not empty; when command handling completes
     *               successfully, contains the result codes and responses to
     *               this command; Its ownership always belongs to ATEXT, plugin
     *               may reallocate its space when needed.
     * @param aReplyNeeded Reply needed if ETrue, no reply otherwise. If EFalse,
     *                     the aReply must not contain the reply, otherwise it
     *                     must contain verbose or numeric reply (ATV0/1) or an
     *                     empty string reply (with ATQ).
     * @return None
     */
    void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );

    /**
     * Cancels a pending HandleCommand request.
     *
     * @since TB9.2 
     * @return None
     */
    void HandleCommandCancel();

    /**
     * Next reply part's length.
     * The value must be equal or less than KDefaultCmdBufLength.
     * When the reply from this method is zero, ATEXT stops calling
     * GetNextPartOfReply().
     *
     * @since TB9.2 
     * @return Next reply part's length if zero or positive
     */
    TInt NextReplyPartLength();

    /**
     * Gets the next part of reply initially set by HandleCommandComplete().
     * Length of aNextReply must be equal or less than KDefaultCmdBufLength.
     *
     * @since TB9.2 
     * @param aNextReply Next reply
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt GetNextPartOfReply( RBuf8& aNextReply );

    /**
     * Receives unsolicited results. Cancelling of the pending request is done
     * by ReceiveUnsolicitedResultCancel(). The implementation in the extension
     * plugin should be asynchronous.
     *
     * @since TB9.2 
     * @return None
     */
    void ReceiveUnsolicitedResult();

    /**
     * Cancels a pending ReceiveUnsolicitedResult request.
     *
     * @since TB9.2 
     * @return None
     */
    void ReceiveUnsolicitedResultCancel();

    /**
     * Reports NVRAM status change to the plugins.
     *
     * @since TB9.2 
     * @param aNvram New NVRAM status. Each call of this function is a result
     *               of DUN extracting the form notified by
     *               CATExtCommonBase::SendNvramStatusChange(). Each of the
     *               settings from SendNvramStatusChange() is separated to
     *               one call of ReportNvramStatusChange().
     * @return None
     */
    void ReportNvramStatusChange( const TDesC8& aNvram );

    /**
     * Reports about external handle command error condition.
     * This is for cases when for example DUN decided the reply contained an
     * error condition but the plugin is still handling the command internally.
     * Example: in command line "AT+TEST;ATDT1234" was given. "AT+TEST" returns
     * "OK" and "ATDT" returns "CONNECT". Because "OK" and "CONNECT" are
     * different reply types the condition is "ERROR" and DUN ends processing.
     * This solution keeps the pointer to the last AT command handling plugin
     * inside ATEXT and calls this function there to report the error.
     * It is to be noted that HandleCommandCancel() is not sufficient to stop
     * the processing as the command handling has already finished.
     *
     * @since TB9.2 
     * @return None
     */
    void ReportExternalHandleCommandError();

    /**
     * Creates part of reply from the global reply buffer to the destination
     * buffer. Used with APIs which need the next part of reply in multipart
     * reply requests.
     *
     * @since TB9.2 
     * @param aDstBuffer Destination buffer; the next part of reply is stored to
     *                   this buffer.
     * @return None
     */
    TInt CreatePartOfReply( RBuf8& aDstBuffer );

// from base class MCmdPluginObserver

    /**
     * From MCmdPluginObserver.
     * Creates an AT command reply based on the reply type and completes the
     * request to ATEXT. Uses iReplyBuffer for reply storage.
     *
     * @since TB9.2 
     * @param aReplyType Type of reply
     * @param aDstBuffer Destination buffer; used for the API requiring the
     *                   AT command reply
     * @param aSrcBuffer Source buffer; used only if aReplyType is EReplyTypeOther
     * @param aError Completion code. If not KErrNone then other arguments are
     *               ignored and the request is completed to ATEXT with
     *               EReplyTypeUndefined.
     * @return None
     */
    TInt CreateReplyAndComplete( TATExtensionReplyType aReplyType,
                                 const TDesC8& aSrcBuffer=KNullDesC8,
                                 TInt aError=KErrNone );

    /**
     * From MCmdPluginObserver.
     * Creates a buffer for "OK" or "ERROR" reply based on the line settings
     *
     * @since TB9.2 
     * @param aReplyBuffer Destination buffer for the created reply
     * @param aOkReply ETrue if "OK" reply needed,
     *                 EFalse if "ERROR" reply needed
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt CreateOkOrErrorReply( RBuf8& aReplyBuffer,
                               TBool aOkReply );

    /**
     * From MCmdPluginObserver.
     * Returns the array of supported commands
     *
     * @since TB9.2 
     * @param aCmd Array of supported commands
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt GetSupportedCommands( RPointerArray<HBufC8>& aCmds );

    /**
     * From MCmdPluginObserver.
     * Returns plugin's character value settings (from CATExtPluginBase)
     *
     * @since TB9.2 
     * @param aCharType Character's type
     * @param aChar Character's value matching aCharType
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt GetCharacterValue( TCharacterTypes aCharType, TChar& aChar );

    /**
     * From MCmdPluginObserver.
     * Returns plugin's mode value settings (from CATExtPluginBase)
     *
     * @since TB9.2 
     * @param aModeType Mode type
     * @param aMode Mode value matching aModeType
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt GetModeValue( TModeTypes aModeType, TBool& aMode );

private:  // data

    /**
     * AT commands handler array. Used for mapping HandleCommand() to
     * IsCommandSupported() and to limit the number of string comparisons.
     */
    RPointerArray<CATCommandHandlerBase> iHandlers;

    /**
     * Current AT command handler in iHandlers.
     * Used when IsCommandSupported() detects a matching handler class.
     */
    CATCommandHandlerBase* iHandler;

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

#endif  // C_MODEMPLUGIN_H
