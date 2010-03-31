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
* Description:  Handles the commands "AT+COPS=..." and "AT+COPS?"
*
*/


#ifndef M_CMDPLUGINOBSERVER_H
#define M_CMDPLUGINOBSERVER_H


enum TCharacterTypes
    {
    ECharTypeCR,  // Carriage return
    ECharTypeLF,  // Line feed
    ECharTypeBS   // Backspace
    };

enum TModeTypes
    {
    EModeTypeQuiet,   // Quiet mode
    EModeTypeVerbose  // Verbose mode
    };


/**
 *  Class for accessing plugin information and common functionality
 *
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( MCmdPluginObserver )
    {

public:

    /**
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
    virtual TInt CreateReplyAndComplete( TATExtensionReplyType aReplyType,
                                         const TDesC8& aSrcBuffer=KNullDesC8,
                                         TInt aError=KErrNone ) = 0;

    /**
     * Creates a buffer for "OK" or "ERROR" reply based on the line settings
     *
     * @since TB9.2
     * @param aReplyBuffer Destination buffer for the created reply
     * @param aOkReply ETrue if "OK" reply needed,
     *                 EFalse if "ERROR" reply needed
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt CreateOkOrErrorReply( RBuf8& aReplyBuffer,
                                       TBool aOkReply ) = 0;

    /**
     * Returns the array of supported commands
     *
     * @since TB9.2
     * @param aCmd Array of supported commands
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt GetSupportedCommands( RPointerArray<HBufC8>& aCmds ) = 0;

    /**
     * Returns plugin's character value settings (from CATExtPluginBase)
     *
     * @since TB9.2
     * @param aCharType Character's type
     * @param aChar Character's value matching aCharType
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt GetCharacterValue( TCharacterTypes aCharType, TChar& aChar ) = 0;

    /**
     * Returns plugin's mode value settings (from CATExtPluginBase)
     *
     * @since TB9.2
     * @param aModeType Mode type
     * @param aMode Mode value matching aModeType
     * @return Symbian error code on error, KErrNone otherwise
     */
    virtual TInt GetModeValue( TModeTypes aModeType, TBool& aMode ) = 0;

    };


#endif  // M_CMDPLUGINOBSERVER_H
