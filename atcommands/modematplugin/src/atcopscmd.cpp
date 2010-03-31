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
* Description:  Handles the commands "AT+COPS?", "AT+COPS=?" and "AT+COPS=..."
*
*/


#include <mmtsy_names.h>
#include "atcopscmd.h"
#include "cmdpluginobserver.h"
#include "debug.h"

_LIT8( KCOPSTestCmd, "AT+COPS=?");
_LIT8( KCOPSReadCmd, "AT+COPS?");
_LIT8( KCOPSSetCmd,  "AT+COPS=");

_LIT8(KSupportedModesStr, ",(0,1,3,4)");
_LIT8(KSupportedFormatsStr, ",(0,1,2)"); 

// The parameters are in predefined indexes in an incoming AT command. 
const TInt KModeParameterIndex     			= 0;  
const TInt KFormatParameterIndex   			= 1;  
const TInt KOperatorParameterIndex 			= 2;  
const TInt KAccessTechnologyParameterIndex  = 3;  

const TInt KMinimumParameterCountWhenModePresent       = 1;  
const TInt KMinimumParameterCountWhenFormatPresent     = 2;  
const TInt KMinimumParameterCountWhenOperatorPresent   = 3;  
const TInt KMinimumParameterCountWhenAccTechPresent    = 4;  

// These parameter lengths are derived from 3GPP TS 27.007 V8.4.1
const TInt KShortOperatorNameFormatLength 			= 10; 
const TInt KLongOperatorNameFormatLength 			= 20; 
const TInt KNumericOperatorNameFormatLength 		= 5; 
const TInt KMaxNetworkTestResponseAdditionalSize 	= 17; // The maximun length of parts of fixed length. 
const TInt KMaxNetworkReadResponseAdditionalSize 	= 28 ; // The maximun length of parts fixed length.

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CATCOPSCmd* CATCOPSCmd::NewL( MCmdPluginObserver* aCallback )
    {
    CATCOPSCmd* self = new (ELeave) CATCOPSCmd( aCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CATCOPSCmd::~CATCOPSCmd()
    {
    iParamArray.ResetAndDestroy(); 
    iParamArray.Close(); 
    iPacketService.Close(); 
    iCustomApi.Close(); 
    iPhone.Close(); 
    iServer.Close();
    delete iDetectedNetworks;
    delete iRetrieveDetectedNetworks; 
    }

// ---------------------------------------------------------------------------
// CATCOPSCmd::CATCOPSCmd
// ---------------------------------------------------------------------------
//
CATCOPSCmd::CATCOPSCmd( MCmdPluginObserver* aCallback ) :
    CActive(EPriorityStandard),
    iCallback( aCallback ), 
	iFormat(RMmCustomAPI::EOperatorNameMccMnc), 
    iRegistrationMode(EModeAutomatic), 
    iAccTech(EAccTechNotSet), 
    iCurrentOperation(EIdle)
    {
    iCmdHandlerType = ECmdHandlerTypeUndefined;
    }

// ---------------------------------------------------------------------------
// CATCOPSCmd::ConstructL
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::ConstructL()
    {
    if ( !iCallback )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add(this);
    LEAVE_IF_ERROR(iServer.Connect());
    LEAVE_IF_ERROR(iServer.LoadPhoneModule(KMmTsyModuleName));
    LEAVE_IF_ERROR(iPhone.Open(iServer, KMmTsyPhoneName));
    LEAVE_IF_ERROR(iCustomApi.Open(iPhone));
    LEAVE_IF_ERROR(iPacketService.Open(iPhone));
    iRetrieveDetectedNetworks = CRetrieveMobilePhoneDetectedNetworks::NewL(iPhone); 
    }

// ---------------------------------------------------------------------------
// Reports the support status of an AT command. This is a synchronous API.
// ---------------------------------------------------------------------------
//
TBool CATCOPSCmd::IsCommandSupported( const TDesC8& aCmd )
    {
    TRACE_FUNC_ENTRY
    TInt retTemp = KErrNone;

		// First test if "test" command, because the pattern is similar with the "set" command, 
		// this is just one extra question mark longer than that. 
    retTemp = aCmd.Compare( KCOPSTestCmd );
    if ( retTemp == 0 )
        {
        iCmdHandlerType = ECmdHandlerTypeTest;
        TRACE_FUNC_EXIT
        return ETrue;
        }

    retTemp = aCmd.Compare( KCOPSReadCmd );
    if ( retTemp == 0 )
        {
        iCmdHandlerType = ECmdHandlerTypeRead;
        TRACE_FUNC_EXIT
        return ETrue;
        }

	// Test if the beginning matches the test command pattern. We're skipping parameters 
	// here on purpose, because "set" handler will create an error reply later if 
	// parameters are not valid. 
    retTemp = aCmd.Left(KCOPSSetCmd().Length()).Compare(KCOPSSetCmd);
    if ( retTemp == 0 )
        {
        iCmdHandlerType = ECmdHandlerTypeSet;
        TRACE_FUNC_EXIT
        return ETrue;
        }

    iCmdHandlerType = ECmdHandlerTypeUndefined;
    TRACE_FUNC_EXIT
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles an AT command. Cancelling of the pending request is done by
// HandleCommandCancel(). The implementation in the extension plugin should
// be asynchronous.
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::HandleCommand( const TDesC8& aCmd,
                                   RBuf8& aReply,
                                   TBool aReplyNeeded )
    {
    TRACE_FUNC_ENTRY

    if ( !aReplyNeeded )
        {
        TRACE_FUNC_EXIT
        return;
        }

    if(iCurrentOperation != EIdle)  
        {
        // only one call at time allowed. If another one is passed in, 
        // then cancel the previous and reply with an error.   
        HandleCommandCancel(); 
        CreateReply(EFalse); 
        }
    
    if ( iCmdHandlerType == ECmdHandlerTypeUndefined )
        {
		CreateReply(EFalse); 
		}

    if ( iCmdHandlerType == ECmdHandlerTypeTest )
        {
        // Collect network data and complete in RunL 
        iRetrieveDetectedNetworks->StartV2(iStatus); 
        iCurrentOperation = EListAvailableNetworkOperators; 
        SetActive(); 
        TRACE_FUNC_EXIT
        return;
        }

/*
Read command returns the current mode, the currently selected operator 
and the current Access Technology. If no operator is selected, <format>, 
<oper> and < AcT>  are omitted.
*/
    if ( iCmdHandlerType == ECmdHandlerTypeRead )
        {
        // Collect data in two steps. First read operator name. Continue in RunL() 
        RMobilePhone::TMobilePhoneNetworkSelectionV1 selection; 
        RMobilePhone::TMobilePhoneNetworkSelectionV1Pckg nwSelectionSetting(selection); 
        iPhone.GetNetworkSelectionSetting(nwSelectionSetting);
        switch(selection.iMethod)
            {
            case RMobilePhone::ENetworkSelectionAutomatic: 
                iRegistrationMode = EModeAutomatic;
                break; 
            case RMobilePhone::ENetworkSelectionManual: 
                iRegistrationMode = EModeManual;
                break; 
            default: 
                // Cannot get a known selection mode! 
                TRACE_INFO(_L("CATCOPSCmd::HandleCommand() -- Cannot get a known selection mode!"));
                CreateReply(EFalse); 
                TRACE_FUNC_EXIT
                return; 
            }
        RMobilePhone::TMobilePhoneNetworkInfoV2Pckg nwInfo(iNetworkInfo); 
        iPhone.GetCurrentNetwork(iStatus, nwInfo); 
        iCurrentOperation = EGetNetworkInfoOperatorName; 
		SetActive(); 
        TRACE_INFO((_L("CATCOPSCmd::HandleCommand() starting operation (%d)"), iCurrentOperation));
        TRACE_FUNC_EXIT
        return;
        }

    // Getting this far means ECmdHandlerTypeSet. There must be parameter(s), too.   
    TRAPD(err, ExtractParametersL(aCmd)); 

    // Check that we got some parameters, at least the "mode". If not, return an error: 
	if(iParamArray.Count() < KMinimumParameterCountWhenModePresent) 
        {
        // Return error response, there were no parameters! 
        TRACE_INFO(_L("CATCOPSCmd::HandleCommand() -- no parameters!"));
        CreateReply(EFalse); 
        TRACE_FUNC_EXIT
        return; 
        }

	// At least the mode parameter is present at this point. Inspect it and check other parameters. 
	TNetworkRegistrationMode mode; 
	err = GetModeAndCheckParameterCount(iParamArray[KModeParameterIndex]->Des(), mode); 
	if(err != KErrNone) 
		{
        // Return error response, invalid mode or other parameters! 
		TRACE_INFO(_L("CATCOPSCmd::HandleCommand() -- invalid mode or other parameters!"));
        CreateReply(EFalse); 
	    TRACE_FUNC_EXIT
        return; 
		}

    // At this point the iRegistrationMode is stored and the parameters are valid. 
	iRegistrationMode = mode; 
	TRACE_INFO(( _L("CATCOPSCmd::HandleCommand() mode stored (%d)"), iRegistrationMode));
		
	if(iParamArray.Count() > 1) 
		{
		// If also format is present, extract it and store for later reference. 
        RMmCustomAPI::TOperatorNameType format;
        err = GetFormatFromParameter(iParamArray[KFormatParameterIndex]->Des(), format); 
		if(err != KErrNone) 
		    {
		    // Return an error, invalid format. 
			// Previously set format is still in use. 
	 		TRACE_INFO(_L("CATCOPSCmd::HandleCommand() -- invalid format!"));
            CreateReply(EFalse); 
	 		TRACE_FUNC_EXIT
			return; 
			}
		// Format parameter is OK, keep it.
		iFormat = format;  
		TRACE_INFO(( _L("CATCOPSCmd::HandleCommand() format stored (%d)"), iFormat));
		}

	// We're done with the required parameters, it's time to start processing the command. 
	// So do a self complete and continue in RunL(): 
	iReply = &aReply;  // Store the reply for later reference in RunL. 
	iCurrentOperation = EInspectModeAndProcessCommand; 
	TRequestStatus *status = &iStatus;  
	User::RequestComplete(status, KErrNone); 
	SetActive(); 
	TRACE_INFO((_L("CATCOPSCmd::HandleCommand() starting operation (%d)"), iCurrentOperation));
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Parses the aCmd parameter and stores results in iParamArray.
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::ExtractParametersL(const TDesC8& aCmd) 
	{
    TRACE_FUNC_ENTRY

    TRACE_INFO(( _L8("CATCOPSCmd::ExtractParameters() extracting (%S)"), &aCmd));

    TPtrC8 parameters = aCmd.Right(aCmd.Length() - KCOPSSetCmd().Length()); 

    iParamArray.ResetAndDestroy(); 

    // Parse the parameters into the parameter array: 
    TInt separatorPos;  
    while((separatorPos = parameters.Locate(',')) != KErrNotFound)
        {
        TRACE_INFO(( _L("CATCOPSCmd::ExtractParameters() separator position (%d)"), separatorPos));
        TPtrC8 param = parameters.Left(separatorPos);
        parameters.Set(parameters.Right(parameters.Length() - (separatorPos + 1))); // Remove the extracted part + separator 
        HBufC8 *heapParam = param.AllocL();    
        CleanupStack::PushL( heapParam );
        // Strip the quotation marks from the parameter: 
        TPtr8 ptr = heapParam->Des(); 
        RemoveQuotationMarks(ptr); 
        TRACE_INFO(( _L8("CATCOPSCmd::ExtractParameters() appending (%S)"), &ptr));
        iParamArray.Append(heapParam); 
        CleanupStack::Pop( heapParam );
        }

    // Finally append the last piece of parameters: 
    HBufC8 *param = parameters.AllocL(); 
    CleanupStack::PushL( param );
    TPtr8 ptr = param->Des(); 
    RemoveQuotationMarks(ptr); 
    TRACE_INFO(( _L8("CATCOPSCmd::ExtractParameters() appending (%S)"), &ptr));
    iParamArray.Append(param); 
    CleanupStack::Pop( param );

    TRACE_FUNC_EXIT
	}

// ---------------------------------------------------------------------------
// Strips all quotation parms from the string passed in. 
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::RemoveQuotationMarks(TPtr8& aParameter) 
    {
    TRACE_FUNC_ENTRY
    // Strip the quotation marks from the parameter: 
    TInt quotePos;  
    while((quotePos = aParameter.Locate('"')) != KErrNotFound)
    {
        aParameter.Delete(quotePos,1); 
    }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Returns the selected mode in aMode and checks the parameter count. 
// ---------------------------------------------------------------------------
//
TInt CATCOPSCmd::GetModeAndCheckParameterCount(const TDesC8& aParameter, TNetworkRegistrationMode &aMode) 
	{
  TRACE_FUNC_ENTRY
	TLex8 lex;
	lex.Assign(aParameter);
	TInt mode(0); 

	TInt err = lex.Val(mode); 
	TRACE_INFO(( _L("CATCOPSCmd::GetModeAndCheckParameterCount() mode (%d)"), mode));

	if( err != KErrNone )
		{
        TRACE_INFO(_L("CATCOPSCmd::GetModeAndCheckParameterCount() TLex error!)"));
        TRACE_FUNC_EXIT
        return KErrArgument; 
		}			

  if(mode < EModeAutomatic || mode > EModeManualAutomatic || mode == EModeDeregister)
      {
      // Not a valid mode. 
      TRACE_FUNC_EXIT
      return KErrArgument; 
      }

  if( (mode == EModeManual || mode == EModeManualAutomatic)  &&  iParamArray.Count() < KMinimumParameterCountWhenOperatorPresent )
      {
      // Valid modes but not enough parameters. At least format and operator needed.  
      TRACE_INFO(( _L("CATCOPSCmd::GetModeAndCheckParameterCount() not enough parameters (%d)"), iParamArray.Count()));
      TRACE_FUNC_EXIT
      return KErrArgument; 
      }
  if( mode == EModeSetFormatParameter && iParamArray.Count() < KMinimumParameterCountWhenFormatPresent )
      {
      // Valid mode, but not enough parameters. Format is needed.  
      TRACE_INFO(_L("CATCOPSCmd::GetModeAndCheckParameterCount() no format parameter)"));
      TRACE_FUNC_EXIT
      return KErrArgument; 
      }

  // Valid mode and enough parameters. 
  aMode = static_cast<TNetworkRegistrationMode>(mode);

	TRACE_FUNC_EXIT
	return KErrNone; 
	}

// ---------------------------------------------------------------------------
// Converts an AT command parameter to numeric format value and checks it is valid. 
// ---------------------------------------------------------------------------
//
TInt CATCOPSCmd::GetFormatFromParameter(const TDesC8& aParameter, RMmCustomAPI::TOperatorNameType &aFormat) 
	{
    TRACE_FUNC_ENTRY
    TLex8 lex;
    lex.Assign(aParameter);
    TInt format(0); 
    TInt err = lex.Val(format); 

    if(err != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument; 
        }
    switch(format)
        {
        case EFormatLong: // long by 3GPP TS 27.007 V8.4.1  
            TRACE_INFO(_L("Format is long by 3GPP TS 27.007 V8.4.1"));
            aFormat = RMmCustomAPI::EOperatorNameNitzFull; 
            break; 
        case EFormatShort: // short by 3GPP TS 27.007 V8.4.1 
            TRACE_INFO(_L("Format is short by 3GPP TS 27.007 V8.4.1"));
            aFormat = RMmCustomAPI::EOperatorNameNitzShort; 
            break; 
        case EFormatNumeric: // numeric by 3GPP TS 27.007 V8.4.1 
            TRACE_INFO(_L("Format is numeric by 3GPP TS 27.007 V8.4.1"));
            aFormat = RMmCustomAPI::EOperatorNameMccMnc; 
            // Operator is numeric, conver it into S60 style. 
            break; 
        default: 
            TRACE_FUNC_EXIT
            return KErrArgument;
        }

    TRACE_FUNC_EXIT
    return KErrNone; 
    }

// ---------------------------------------------------------------------------
// Converts an AT command parameter to numeric access technology value and checks it is valid. 
// ---------------------------------------------------------------------------
//
TInt CATCOPSCmd::GetAccTechFromParameter(const TDesC8& aParameter, TAccessTechnology& aAccTech) 
    {
    TRACE_FUNC_ENTRY
    TLex8 lex;
    lex.Assign(aParameter);
    TInt accTech(0); 
    TInt err = lex.Val(accTech); 

    if(err != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument; 
        }

    if(accTech != EGSM && accTech != EUDMA) // The only allowed access technologies. 
        {
        TRACE_FUNC_EXIT
        return KErrArgument; 
        }
 
    aAccTech = static_cast<TAccessTechnology>(accTech);

    TRACE_FUNC_EXIT
    return KErrNone; 
    }

// ---------------------------------------------------------------------------
// Converts an AT command parameter to ETel compatible operator values 
// ---------------------------------------------------------------------------
//
TInt CATCOPSCmd::ConvertOperatorToMccMnc(const CMobilePhoneNetworkListV2 *aDetectedNetworks, 
                                        const RMmCustomAPI::TOperatorNameType aFormat, 
                                        const TBuf<KMaxOperatorNameLength>& aOperatorParameter, 
                                        RMobilePhone::TMobilePhoneNetworkCountryCode& aMcc, 
                                        RMobilePhone::TMobilePhoneNetworkIdentity& aMnc) 
    {
    TRACE_FUNC_ENTRY

    if(aFormat == RMmCustomAPI::EOperatorNameMccMnc) 
        {
        // Check first that there are at least five characters passed in. 
       	TChar nextChar; 
        if(aOperatorParameter.Length() < 5)
        	{
			return KErrArgument; 
        	}
        for(int i = 0; i < 5; ++i)
        	{
            nextChar = aOperatorParameter[i]; 
            if(!nextChar.IsDigit()) 
                {
                return KErrArgument; 
                }
      		}
        // Operator is in three digit country code + two digit network code format.  
	    // Must be converted to ETel style. The possible extra will be simply discarded. 
     	TRACE_INFO(_L("CATCOPSCmd::ConvertOperatorToMccMnc() operator is all digits, convert it into ETel data types."));
	    aMcc.Copy(aOperatorParameter.Left(3)); 
	    aMnc.Copy(aOperatorParameter.Right(2)); 
	    }
    else  // The short or long text string formats. 
        {
        // Find the requested operator from the operator array.  
        // If array is empty, return an error. 
        if(!aDetectedNetworks)
            {
            TRACE_INFO(_L("CATCOPSCmd::ConvertOperatorToMccMnc() No detected networks!"));
            TRACE_FUNC_EXIT
            return KErrNotFound; 
            }

        RMobilePhone::TMobilePhoneNetworkInfoV2 nwInfo; 
        for(TInt i=0; i < iDetectedNetworks->Enumerate(); ++i)
            {
            TRAPD(err, nwInfo = iDetectedNetworks->GetEntryL(i))  
            if(err != KErrNone)
                {
                return KErrNotFound; 
                }

            if(aFormat == RMmCustomAPI::EOperatorNameNitzShort)
                {
                TRACE_INFO(_L("CATCOPSCmd::ConvertOperatorToMccMnc() Operator is in short format, comparing."));
                if(nwInfo.iShortName.Compare(aOperatorParameter) == 0)
                    {
                    TRACE_INFO(_L("Match found."));
                    aMcc = nwInfo.iCountryCode; 
                    aMnc = nwInfo.iNetworkId; 
                    TRACE_FUNC_EXIT
                    return KErrNone; 
                    } 
                }
            else if(aFormat == RMmCustomAPI::EOperatorNameNitzFull)
                {
                TRACE_INFO(_L("CATCOPSCmd::ConvertOperatorToMccMnc() Operator is in long format, comparing."));
                if(nwInfo.iLongName.Compare(aOperatorParameter) == 0)
                    {
                    TRACE_INFO(_L("Match found."));
                    aMcc = nwInfo.iCountryCode; 
                    aMnc = nwInfo.iNetworkId; 
                    TRACE_FUNC_EXIT
                    return KErrNone; 
                    } 
                }
            else        
                {
                TRACE_INFO(_L("CATCOPSCmd::ConvertOperatorToMccMnc() Unknown operator format!"));
                TRACE_FUNC_EXIT
                return KErrArgument; 
                }
            }
        TRACE_INFO(_L("CATCOPSCmd::ConvertOperatorToMccMnc() Operator was not found in list!"));
        TRACE_FUNC_EXIT
        return KErrNotFound; 
        }

    TRACE_FUNC_EXIT
    return KErrNone; 
    }


// ---------------------------------------------------------------------------
// Initiates an automatic network registration.  
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::AutomaticNetworkRegistration() 
    {
    TRACE_FUNC_ENTRY
    RMobilePhone::TMobilePhoneNetworkManualSelection nwInfo; 
	iCurrentOperation = EAutomaticallyRegisterToNetwork; 
	nwInfo.iCountry = KNullDesC;
	nwInfo.iNetwork = KNullDesC;
	iPhone.SelectNetwork(iStatus, EFalse, nwInfo); 
	SetActive();  // Response will be sent in RunL 
	TRACE_INFO((_L("CATCOPSCmd::HandleCommand() starting operation (%d)"), iCurrentOperation));
	TRACE_FUNC_EXIT
	}

// ---------------------------------------------------------------------------
// Initiates a manual network registration.  
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::ManualNetworkRegistration(const RMobilePhone::TMobilePhoneNetworkCountryCode& aMcc, 
                                           const RMobilePhone::TMobilePhoneNetworkIdentity& aMnc) 
    {
	TRACE_FUNC_ENTRY
	RMobilePhone::TMobilePhoneNetworkManualSelection nwInfo; 
	iCurrentOperation = EManuallyRegisterToNetwork; 
	nwInfo.iCountry.Append(aMcc);
	nwInfo.iNetwork.Append(aMnc);
	iPhone.SelectNetwork(iStatus, ETrue, nwInfo); 
	SetActive(); 	// Response will be sent in RunL 
	TRACE_INFO((_L("CATCOPSCmd::HandleCommand() starting operation (%d)"), iCurrentOperation));
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Initiates a manual network registration and access technology selection.  
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::ManualNetworkRegistration(const RMobilePhone::TMobilePhoneNetworkCountryCode& aMcc, 
                                           const RMobilePhone::TMobilePhoneNetworkIdentity& aMnc, 
                                           const TAccessTechnology aAccTech) 
    {
    TRACE_FUNC_ENTRY
	// Store access technology for later reference: 
    iAccTech = aAccTech; 
	// Call another overload to start the first phase of the operation: 
	ManualNetworkRegistration(aMcc, aMnc); 
	// Set the state again so the RunL knows to launch the next phase: 
	iCurrentOperation = EManuallyRegisterToNetworkAndChooseAccTech; 
  	TRACE_INFO((_L("CATCOPSCmd::HandleCommand() starting operation (%d)"), iCurrentOperation));
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CATCOPSCmd::RunL 
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::RunL()
    {
    TRACE_FUNC_ENTRY
	TInt err = KErrNone; 
	if(iStatus != KErrNone)   
	    {
        HandleError(); 
	    TRACE_FUNC_EXIT
        return; 
	    }
    // Proceed to next step or return a response if all is done.
    switch(iCurrentOperation)
        {
        case EListAvailableNetworkOperators: 
            TRACE_INFO((_L("CATCOPSCmd::HandleCommand() completing operation (%d)"), iCurrentOperation));
            if(iDetectedNetworks)
                {
                delete iDetectedNetworks;
                iDetectedNetworks = NULL; 
                }
            iDetectedNetworks = iRetrieveDetectedNetworks->RetrieveListV2L(); 
            // Then create a response. 
            TRAP(err, ConstructNetworkListResponseL()); 
            if(err != KErrNone)
                {
                // An error here means that no response has been sent. Reply with an error. 
                CreateReply(EFalse); 
                }
            break; 
        
        case EInspectModeAndProcessCommand: 
            // Check the mode and act accordingly 
            TRACE_INFO((_L("CATCOPSCmd::HandleCommand() completing operation (%d)"), iCurrentOperation));
            err = InspectModeAndProcessCommand(); 
            if(err != KErrNone)
                {
                CreateReply(EFalse); 
                }
            break; 

        case EGetNetworkInfoOperatorName: 
            if(ConstructNetworkInfoResponse() != KErrNone)
                {
                // An error means that no response has been sent. Reply with an error. 
                CreateReply(EFalse); 
                }
            break; 

        case EManuallyRegisterToNetworkAndChooseAccTech: 
            TRACE_INFO((_L("CATCOPSCmd::HandleCommand() completing operation (%d)"), iCurrentOperation));
            switch(iAccTech)
                {
                case EGSM: 
                    iCustomApi.SetSystemNetworkMode(iStatus, RMmCustomAPI::KCapsNetworkModeGsm);
                    iCurrentOperation = ESetSystemNetworkBand; 
                    SetActive(); 
                    break; 
                case EUDMA: 
                    iCustomApi.SetSystemNetworkMode(iStatus, RMmCustomAPI::KCapsNetworkModeUmts);
                    iCurrentOperation = ESetSystemNetworkBand; 
                    SetActive(); 
                    break; 
                default:
                    // No automatic registering requested, so send back an error response. 
                    TRACE_INFO( _L("CATCOPSCmd::RunL() incorrect acc.tech., reply an error."));
                    CreateReply(EFalse); 
                }
            TRACE_INFO((_L("CATCOPSCmd::HandleCommand() starting operation (%d)"), iCurrentOperation));
            break; 

        case EManuallyRegisterToNetwork: 
        case EAutomaticallyRegisterToNetwork: 
        case ESetSystemNetworkBand: 
            TRACE_INFO((_L("CATCOPSCmd::HandleCommand() completing operation (%d)"), iCurrentOperation));
            // Last step completed successfully, so create OK response. 
            CreateReply(ETrue); 
            break; 

        default: 
            TRACE_INFO(( _L("CATCOPSCmd::RunL() default operation (%d)!"), iCurrentOperation));
            break; 
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Handles an error in async call. 
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::HandleError()
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO(( _L("CATCOPSCmd::RunL() failure (%d) in operation (%d)!"), iStatus.Int(), iCurrentOperation));

    // In case of failure check the operation. In some cases failures are OK.
    switch(iCurrentOperation)
        {
        case EManuallyRegisterToNetwork: 
            if(iRegistrationMode == EModeManualAutomatic)
                {
                // Manual registration failed, try automatic next. 
                TRACE_INFO( _L("CATCOPSCmd::RunL() registration mode manual automatic, try automatic."));
                AutomaticNetworkRegistration(); 
                }
            else 
                {
                // No automatic registering requested, so send back an error response. 
                TRACE_INFO( _L("CATCOPSCmd::RunL() reply an error."));
                CreateReply(EFalse); 
                }
            break; 
        case ESetSystemNetworkBand: 
        case EManuallyRegisterToNetworkAndChooseAccTech: 
			if(iRegistrationMode == EModeManualAutomatic)
                {
                // Manual registration failed, try automatic next. 
                TRACE_INFO( _L("CATCOPSCmd::RunL() registration mode manual automatic, try automatic."));
                AutomaticNetworkRegistration(); 
				break;
                }
            else 
                {
				// Cannot set the access technology, so set it back to EAccTechNotSet. 
				// This prevents replying to queries with outdated or incorrect acc tech information. 
				TRACE_INFO( _L("CATCOPSCmd::RunL() couldn't set system network band, so reset access tech."));
				iAccTech = EAccTechNotSet; 
				// Fall through to default, because these require an error response. 
				}
        default: 
            // In all other cases send back an error response. 
            TRACE_INFO( _L("CATCOPSCmd::RunL() reply an error."));
            CreateReply(EFalse); 
            break; 
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Cancels a pending HandleCommand request.
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
    Cancel(); 
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CATCOPSCmd::DoCancel
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::DoCancel()
    {
    TRACE_FUNC_ENTRY
    switch(iCurrentOperation)
        {
        case EAutomaticallyRegisterToNetwork:
        case EManuallyRegisterToNetwork:
        case EManuallyRegisterToNetworkAndChooseAccTech:
            iPhone.CancelAsyncRequest(EMobilePhoneSelectNetworkCancel);  
            break; 
        case EGetNetworkInfoOperatorName:
            iPhone.CancelAsyncRequest(EMobilePhoneGetCurrentNetworkCancel);  
            break; 
        case ESetSystemNetworkBand:
            iCustomApi.CancelAsyncRequest(ECustomSetSystemNetworkModeIPC); 
            break; 
        case EListAvailableNetworkOperators:
            iRetrieveDetectedNetworks->Cancel();  
            break; 
        default: 
            break; 
        }

    iCurrentOperation = EIdle; 

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Helper method for RunL() 
// ---------------------------------------------------------------------------
//
TInt CATCOPSCmd::InspectModeAndProcessCommand()
    {
    TRACE_FUNC_ENTRY
    TBuf<KMaxOperatorNameLength> buf;
    TInt err; 

    switch (iRegistrationMode)
        {
        case EModeAutomatic: 
            AutomaticNetworkRegistration(); 
            break; 
        case EModeManual: 
        case EModeManualAutomatic: // see also RunL() 
            if(iParamArray.Count() < KMinimumParameterCountWhenOperatorPresent)
                {
                TRACE_FUNC_EXIT
                return KErrArgument; 
                }
            
            // At least the operator is present, so convert it into S60 format. 
            buf.Copy(iParamArray[KOperatorParameterIndex]->Des()); 
            err = ConvertOperatorToMccMnc(iDetectedNetworks, iFormat, buf, iMcc, iMnc); 
            if(err != KErrNone)
                {
                TRACE_INFO(_L("CATCOPSCmd::HandleCommand() -- operator conversion failed!"));
                TRACE_FUNC_EXIT
                return KErrArgument; 
                }

            if (iParamArray.Count() >= KMinimumParameterCountWhenAccTechPresent) 
                {
                // Also access tech. is present. Convert it to ETel compatible value.  
                TAccessTechnology accTech; 
                TInt err = GetAccTechFromParameter(iParamArray[KAccessTechnologyParameterIndex]->Des(), accTech); 
                if(err != KErrNone)
                    {
                    // Parameter problem, return an error. 
                    TRACE_FUNC_EXIT
                    return KErrArgument; 
                    }
                // Register both operator and access technology manually.
                ManualNetworkRegistration(iMcc, iMnc, accTech); 
                }
            else 
                {
                // No access technology parameter, so register just the operator. 
                ManualNetworkRegistration(iMcc, iMnc); 
                }
            break; 
        case EModeDeregister: // Deregister from network 
            // Not supported, return an error. 
			TRACE_FUNC_EXIT
            return KErrArgument; 
        case EModeSetFormatParameter: 
            // Storing format parameter was done already, so just reply OK. 
            CreateReply(ETrue); 
			TRACE_FUNC_EXIT
            return KErrNone; 
        default: 
            return KErrArgument; 
        }
    TRACE_FUNC_EXIT
    return KErrNone; 
    }

// ---------------------------------------------------------------------------
// Converts the ETel access technology into 3GPP TS 27.007 V8.4.1 compatible format. 
// ---------------------------------------------------------------------------
//
TInt CATCOPSCmd::SolveAccessTechnology(RMobilePhone::TMobilePhoneNetworkAccess &aAccessTech)
    {
    TRACE_FUNC_ENTRY

    TUint caps;
    if(iPacketService.GetStaticCaps(caps, RPacketContext::EPdpTypePPP) != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrGeneral;     
        }

    TRACE_INFO(( _L8("CATCOPSCmd::SolveAccessTechnology() static caps gotten (%b)"), caps));

    switch(aAccessTech)
        {
        case RMobilePhone::ENetworkAccessGsm: 
            if(caps & RPacketService::KCapsEGPRSSupported)
                {
                iAccTech = EGSMwithEGPRS; 
                }
            else 
                {
                iAccTech = EGSM; 
                }
            break; 
        case RMobilePhone::ENetworkAccessGsmCompact: 
            iAccTech = EGSMCompact; 
            break; 
        case RMobilePhone::ENetworkAccessUtran: 
            if(caps & RPacketService::KCapsHSDPASupported) 
                {
                if(caps & RPacketService::KCapsHSUPASupported)
                    {
                    iAccTech = EUDMAwithHSDPAandHSUPA;  
                    }
                else 
                    {
                    iAccTech = EHSDPA;  
                    }
                }
            else if(caps & RPacketService::KCapsHSUPASupported) 
                {
                iAccTech = EHSUPA;  
                }
            else 
                {
                iAccTech = EUDMA;  
                }
            break;
        default: 
            TRACE_INFO( _L("CATCOPSCmd::SolveAccessTechnology() unknown access tech!"));
            iAccTech = EAccTechNotSet; 
            return KErrArgument; 
		}
    TRACE_FUNC_EXIT
    return KErrNone;  
    }

// ---------------------------------------------------------------------------
// Contructs a response for the read command. 
// ---------------------------------------------------------------------------
//
TInt CATCOPSCmd::ConstructNetworkInfoResponse()
    {
    TRACE_FUNC_ENTRY
    RBuf8 reply;
    TInt size(KMaxNetworkTestResponseAdditionalSize + KLongOperatorNameFormatLength);       
    TChar carriageReturn;
    TChar lineFeed;
    TInt err;
    err = reply.Create(size);
	err |= iCallback->GetCharacterValue( ECharTypeCR, carriageReturn );
	err |= iCallback->GetCharacterValue( ECharTypeLF, lineFeed );		
	if(err != KErrNone) 
		{
		return err; 
		}

	// Some PC Software expects and extra CR+LF, hence those are added twice: 
	reply.Append( carriageReturn ); 
	reply.Append( lineFeed );
    reply.Append( carriageReturn );
    reply.Append( lineFeed );  
    reply.Append(_L("+COPS: "));  
    reply.AppendNum(iRegistrationMode);  
    reply.Append(_L(","));  
    switch(iFormat)
        {
        case RMmCustomAPI::EOperatorNameNitzFull:   
            reply.AppendNum(EFormatLong);  
            reply.Append(_L(",")); 
            reply.Append(_L("\""));   
            TRACE_INFO(( _L8("CATCOPSCmd::ConstructNetworkInfoResponse() appending (%S)"), 
                    &iNetworkInfo.iLongName));
            reply.Append(iNetworkInfo.iLongName);  
            break; 
        case RMmCustomAPI::EOperatorNameNitzShort:
            reply.AppendNum(EFormatShort); 
            reply.Append(_L(",")); 
            reply.Append(_L("\"")); 
            TRACE_INFO(( _L8("CATCOPSCmd::ConstructNetworkInfoResponse() appending (%S)"), 
                    &iNetworkInfo.iShortName));
            reply.Append(iNetworkInfo.iShortName); 
            break; 
        case RMmCustomAPI::EOperatorNameMccMnc: 
            reply.AppendNum(EFormatNumeric); 
            reply.Append(_L(",")); 
            reply.Append(_L("\"")); 
            TRACE_INFO(( _L8("CATCOPSCmd::ConstructNetworkInfoResponse() appending codes (%S) and (%S)"), 
                            &iNetworkInfo.iCountryCode, &iNetworkInfo.iNetworkId));
            reply.Append(iNetworkInfo.iCountryCode); 
            reply.Append(iNetworkInfo.iNetworkId); 
            break; 
        }
    reply.Append(_L("\"")); 

    if(SolveAccessTechnology(iNetworkInfo.iAccess) == KErrNone && iAccTech != EAccTechNotSet) 
        {
        TRACE_INFO((_L("CATCOPSCmd::ConstructNetworkInfoResponse() appending acc. tech. (%d)"), 
                            iAccTech));
        reply.Append(_L(",")); 
        reply.AppendNum(iAccTech); 
        }

    reply.Append( carriageReturn );
    reply.Append( lineFeed );

    CreateReply(ETrue, reply); 

    TRACE_FUNC_EXIT
    return KErrNone;  
    }


// ---------------------------------------------------------------------------
// Contructs a response for the test command. 
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::ConstructNetworkListResponseL()
    {
    TRACE_FUNC_ENTRY
    RBuf8 reply;
    TChar carriageReturn;
    TChar lineFeed;

    TInt maxItemSize(KMaxNetworkReadResponseAdditionalSize  
            + KShortOperatorNameFormatLength 
            + KLongOperatorNameFormatLength 
            + KNumericOperatorNameFormatLength
            + KSupportedModesStr().Length()
            + KSupportedFormatsStr().Length()); 

    CleanupClosePushL(reply); 

    User::LeaveIfNull(iDetectedNetworks); 
    User::LeaveIfError(reply.Create( maxItemSize * iDetectedNetworks->Enumerate())); 
    User::LeaveIfError(iCallback->GetCharacterValue( ECharTypeCR, carriageReturn ));
    User::LeaveIfError(iCallback->GetCharacterValue( ECharTypeLF, lineFeed ));		

    // Some PC Software expects and extra CR+LF, hence those are added twice: 
    reply.Append( carriageReturn );
    reply.Append( lineFeed );
    reply.Append( carriageReturn );
    reply.Append( lineFeed );
    reply.Append( _L("+COPS: ") ); 

    RMobilePhone::TMobilePhoneNetworkInfoV2 nwInfo; 
    for(TInt i = 0; i < iDetectedNetworks->Enumerate(); ++i)
        {
		if(i > 0) // Add CR+LF after the first cycle. 
			{
            reply.Append( carriageReturn );
            reply.Append( lineFeed );
			}
        nwInfo = iDetectedNetworks->GetEntryL(i);  

        reply.Append(_L("(")); 
        reply.AppendNum(nwInfo.iStatus); 
        reply.Append(_L(",")); 
		reply.Append(_L("\"")); 
        reply.Append(nwInfo.iLongName); 
		reply.Append(_L("\"")); 
        reply.Append(_L(",")); 
		reply.Append(_L("\"")); 
        reply.Append(nwInfo.iShortName); 
		reply.Append(_L("\"")); 
        reply.Append(_L(",")); 
		reply.Append(_L("\"")); 
        reply.Append(nwInfo.iCountryCode); 
        reply.Append(nwInfo.iNetworkId); 
		reply.Append(_L("\"")); 
        if(SolveAccessTechnology(nwInfo.iAccess) == KErrNone && iAccTech != EAccTechNotSet) 
            {
            TRACE_INFO((_L("CATCOPSCmd::ConstructNetworkListResponse() appending acc. tech. (%d)"), iAccTech));
            reply.Append(_L(",")); 
            reply.AppendNum(iAccTech); 
            }
        reply.Append(_L(")")); 
        reply.Append(_L(",")); 
		TRACE_INFO( _L("CATCOPSCmd::ConstructNetworkListResponse() -- entry added to reply."));
        }
    reply.Append(KSupportedModesStr); // Supported modes as defined in 3GPP TS 27.007 V8.4.1
    reply.Append(KSupportedFormatsStr);  // Supported formats as defined in 3GPP TS 27.007 V8.4.1

    reply.Append( carriageReturn );
    reply.Append( lineFeed );

		// Finally append the "OK". CreateOkOrErrorReply returns verbose or numeric version. 
    RBuf8 okReply;
    CleanupClosePushL(okReply); 
    iCallback->CreateOkOrErrorReply( okReply, ETrue );
    reply.Append( okReply);
    CreateReply(ETrue, reply); 
    CleanupStack::PopAndDestroy(&okReply);   
    CleanupStack::PopAndDestroy(&reply);   
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Finalises the response and sends it. 
// ---------------------------------------------------------------------------
//
void CATCOPSCmd::CreateReply(TBool aIsOK, const TDesC8 &aReply) 
    {
    if(aIsOK == EFalse)
        {
        iCallback->CreateReplyAndComplete( EReplyTypeError);
        }
    else 
        {
        if(aReply.Length() > 0)
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOther,
                                           aReply );
            }
        else 
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOk);
            }
        }
    iCurrentOperation = EIdle; 
    TRACE_FUNC_EXIT
    }
