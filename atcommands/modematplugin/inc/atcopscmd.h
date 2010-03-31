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


#ifndef C_CATCOPSCMD_H
#define C_CATCOPSCMD_H

#include <e32base.h>
#include <e32cmn.h>
#include <rmmcustomapi.h>
#include <etelpckt.h>
#include <etelmm.h>
#include <mmretrieve.h>
#include "modematplugin.h"

class MCmdPluginObserver; 
class CRetrieveMobilePhoneDetectedNetworks; 

/**  Handler types for the AT commands */
enum TCmdHandlerType
    {
    ECmdHandlerTypeUndefined,
    ECmdHandlerTypeTest,  // For command "AT+COPS=?"
    ECmdHandlerTypeRead,  // For command "AT+COPS?"
    ECmdHandlerTypeSet    // For command "AT+COPS=..."
    };

/**  Keeps track of the current operation for the state machine */
enum TCurrentOperation
    {
    EIdle 										= 0, 
    EInspectModeAndProcessCommand 				= 1,
    EAutomaticallyRegisterToNetwork 			= 2,
    EManuallyRegisterToNetwork 					= 3,
    EManuallyRegisterToNetworkAndChooseAccTech 	= 4,
    EGetNetworkInfoOperatorName 				= 5, 
    ESetSystemNetworkBand 						= 6, 
    EUpdateAvailableNetworkOperators 			= 7, 
    EListAvailableNetworkOperators 				= 8    
    };

/** These are in the same order as in 3GPP TS 27.007 V8.4.1  */ 
enum TOperatorFormat
    {
    EFormatLong, 
    EFormatShort, 
    EFormatNumeric  
    }; 

/** These are in the same order as in 3GPP TS 27.007 V8.4.1  */ 
enum TNetworkRegistrationMode
    {
    EModeAutomatic, 
    EModeManual, 
    EModeDeregister, 
    EModeSetFormatParameter, 
    EModeManualAutomatic, 
    }; 

/** Currently selected access technology for outgoing replies. 
*		S60 uses definitions from RMobilePhone class, but they are 
* 	in diffent order and cannot be used directly. 
*		These are in the same order as in 3GPP TS 27.007 V8.4.1  */ 
enum TAccessTechnology
    {
    EGSM 					= 0, 
    EGSMCompact 			= 1,  
    EUDMA 					= 2, 
    EGSMwithEGPRS 			= 3, 
    EHSDPA 					= 4, 
    EHSUPA 					= 5,  
    EUDMAwithHSDPAandHSUPA 	= 6, 
    EAccTechNotSet 			= 255
    }; 

/**
 *  Class for handling commands "AT+COPS?" and "AT+COPS=..."
 *
 *  @since TB9.2
 */
NONSHARABLE_CLASS( CATCOPSCmd ) : public CActive, public CATCommandHandlerBase    
{

public:

    /**
     * Two-phased constructor.
     * @param aCallback Callback
     * @return Instance of self
     */
    static CATCOPSCmd* NewL( MCmdPluginObserver* aCallback );

    /**
    * Destructor.
    */
    virtual ~CATCOPSCmd();

protected:
    /**
    * From CActive. Called when asynchronous request completes.
    * @since TB9.2
    * @param None
    * @return None
    */
    virtual void RunL();
         
    virtual void DoCancel();

private:

    CATCOPSCmd( MCmdPluginObserver* aCallback );

    void ConstructL();

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


private: 


    void HandleCommandTest( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    void HandleCommandRead( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    void HandleCommandSet( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );

    void HandleError(); 

    /**
     * This method parses the parameter part of command and populates 
     * iParamArray class member. Quotation marks are removed in process. 
     *
     * @since TB9.2
     * @param aCmd The AT command. 
     * @return Leaves in out of memory case. 
     */
    void ExtractParametersL(const TDesC8& aCmd); 

    /**
     * This method strips all quotation parms from the string passed in. 
     *
     * @since TB9.2
     * @param aParameter one parameter from AT command as extracted by ExtractParametersL. 
     * @return None
     */
    void RemoveQuotationMarks(TPtr8& aParameter); 

    /**
     * This method returns the selected mode in aMode. 
     * It also checks that there are enough parameters for the mode in question. 
     *
     * @since TB9.2
     * @param aParameter one parameter from AT command as extracted by ExtractParametersL. 
     * @param aMode contains the converted parameter if method completed successfully. 
     * @return 	KErrArgument if mode is invalid or there is not enough parameters. 
     *					KErrNone if iParamArray contains all the parameters the mode requires. 
     */
    TInt GetModeAndCheckParameterCount(const TDesC8& aParameter, TNetworkRegistrationMode &aMode); 

    /**
     * This method converts an AT command parameter to numeric format value and checks it is valid. 
     *
     * @since TB9.2
     * @param aParameter one parameter from AT command as extracted by ExtractParametersL. 
     * @param aFormat contains the converted parameter if method completed successfully. 
     * @return 	KErrArgument if format is invalid. 
     *					KErrNone if format is valid. 
     */
    TInt GetFormatFromParameter(const TDesC8& aParameter, RMmCustomAPI::TOperatorNameType &aFormat);  

    /**
     * This method converts an AT command parameter to numeric access technology value and checks it is valid. 
     *
     * @since TB9.2
     * @param aParameter one parameter from AT command as extracted by ExtractParametersL. 
     * @param aAccTech contains the converted parameter if method completed successfully. 
     * @return 	KErrArgument if acc. tech. is invalid. 
     *					KErrNone if acc. tech. is valid. 
     */
    TInt GetAccTechFromParameter(const TDesC8& aParameter, TAccessTechnology &aAccTech); 
    
    /**
     * This method converts an AT command parameter to ETel compatible operator values 
     *
     * @since TB9.2
     * @param aDetectedNetworks contains the list of networks. May be NULL. 
     * @param aFormat contains the format (numeric/text) of operator parameter. 
     * @param aOperatorParameter contains the operator parameter string. 
     * @param aMcc contains the converted parameter if method completed successfully. 
     * @param aMnc contains the converted parameter if method completed successfully. 
     * @return 	KErrArgument 	if operator parameter invalid, 
     *					KErrNotFound 	if operator list exists but the operator is not in it, 
     *												or the operator list is missing. (Required if aFormat is text.) 
     *					KErrNone if conversion succeeds. aMcc and aMnc contain ETel compatible operator values. 
     */
    TInt ConvertOperatorToMccMnc(const CMobilePhoneNetworkListV2 *aDetectedNetworks,
                                            const RMmCustomAPI::TOperatorNameType aFormat, 
                                            const TBuf<KMaxOperatorNameLength>& aOperatorParameter, 
                                            RMobilePhone::TMobilePhoneNetworkCountryCode& aMcc, 
                                            RMobilePhone::TMobilePhoneNetworkIdentity& aMnc); 

    /**
     * This method initiates an automatic network registration.  
     *
     * @since TB9.2
     * @return 	None
     */
    void AutomaticNetworkRegistration(); 

    /**
     * This method initiates a manual network registration.  
     *
     * @since TB9.2
     * @param aMcc contains the country code part of ETel operator info. 
     * @param aMnc contains the network code part of ETel operator info. 
     * @return 	None
     */
    void ManualNetworkRegistration(const RMobilePhone::TMobilePhoneNetworkCountryCode& aMcc, 
                                   const RMobilePhone::TMobilePhoneNetworkIdentity& aMnc); 

    /**
     * This method initiates a manual network registration and access technology selection.  
     *
     * @since TB9.2
     * @param aMcc contains the country code part of ETel operator info. 
     * @param aMnc contains the network code part of ETel operator info. 
     * @param aAccTech contains the access technology in ETel compatible format. 
     * @return 	None
     */
    void ManualNetworkRegistration(	const RMobilePhone::TMobilePhoneNetworkCountryCode& aMcc, 
                                    const RMobilePhone::TMobilePhoneNetworkIdentity& aMnc, 
                                    const TAccessTechnology aAccTech); 

    /**
     * This is a helper function used by RunL 
     *
     * @since TB9.2
     * @return 	standard Symbian OS return code. 
     */
    TInt InspectModeAndProcessCommand(); 

    /**
     * This method contructs a response for the test command. 
     *
     * @since TB9.2
     * @return 	None. Leaves with standar symbian OS error code on error.  
     */
    void ConstructNetworkListResponseL(); 

    /**
     * This method contructs a response for the read command. 
     *
     * @since TB9.2
     * @return 	standard Symbian OS return code. 
     */
    TInt ConstructNetworkInfoResponse(); 

    /**
     * This helper method converts the ETel access technology into 
     * 3GPP TS 27.007 V8.4.1 compatible format. 
     *
     * @since TB9.2
     * @param aAccTech contains the access technology in ETel compatible format. 
     * @return 	Standard Symbian OS return code. 
     */
    TInt SolveAccessTechnology(RMobilePhone::TMobilePhoneNetworkAccess &aAccessTech); 

    /**
     * This helper method finalises the response and sends it. 
     *
     * @since TB9.2
     * @param aIsOK tells whether to create an OK or ERROR response. 
     * @param aReply contains the response string to be sent before OK, if any. 
     * @return 	None
     */
    void CreateReply(TBool aIsOK, const TDesC8 &aReply = KNullDesC8); 

private:  // data

    /**
     * Callback to call when accessing plugin information
     */
    MCmdPluginObserver* iCallback;

    /**
     * Handler type for the three AT commands
     */
    TCmdHandlerType iCmdHandlerType;

    /**
     * Telephony server instance. 
     */
	RTelServer iServer;

    /**
     * Phone instance, used for network selection. 
     */
	RMobilePhone iPhone;

	/**
     * Telephony Custom API instance, used for setting 
     * the access technology (only GSM and UDMA supported).  
     */
    RMmCustomAPI iCustomApi;

    /**
     * Current network info collected via the RMobilePhone is stored here.   
     */
    RMobilePhone::TMobilePhoneNetworkInfoV2 iNetworkInfo; 

    /**
     * The parameters extracted from the AT command are stored here. 
     */
	RPointerArray<HBufC8> iParamArray; 

    /**
     * Used for getting the static packet capabilities of the phone. 
     * This is needed for supporting more detailed access technology. 
     */
	RPacketService iPacketService; 

    /**
     * The format of the operator parameter is stored here. 
     * it is used both for interpreting the incoming operator and 
     * responding with correctly formatted outgoing operator info. 
     * this can be done because it is specified that the latest incoming 
     * format info is also used for responses from that onwards. 
     */
	RMmCustomAPI::TOperatorNameType iFormat; 

    /**
     * The requested/current network registration mode is stored here. It is used 
     * also when responding to the Read command. This can be done because the two 
     * operations are completely independent. 
     */
	TNetworkRegistrationMode iRegistrationMode; 

    /**
     * The requested/current access tehcnology is stored here. It is used 
     * also when responding to the Read and Test commands. This can be done 
     * because the operations are completely independent. 
     */
	TAccessTechnology iAccTech; 

    /**
     * The incoming reply buffer is kept here. Not owned. 
     */
  RBuf8* iReply; 

    /**
     * The currently running operation is kept here. 
     */
  TCurrentOperation iCurrentOperation; 

    /**
     * The country code part of ETel compatible operator info. 
     */
    RMobilePhone::TMobilePhoneNetworkCountryCode iMcc;  

    /**
     * The network code part of ETel compatible operator info. 
     */
    RMobilePhone::TMobilePhoneNetworkIdentity iMnc; 

    /**
     * Used for retrieving a list of networks the phone detected. 
     */
    CRetrieveMobilePhoneDetectedNetworks *iRetrieveDetectedNetworks; 

    /**
     * Used for reading the list the CRetrieveMobilePhoneDetectedNetworks returned. 
     */
    CMobilePhoneNetworkListV2 *iDetectedNetworks; 
}; 

#endif  // C_LCLISTALLCMD_H
