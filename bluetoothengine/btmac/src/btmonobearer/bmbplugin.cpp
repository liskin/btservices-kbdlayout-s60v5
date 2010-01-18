/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
*       This class implements RemCon bearer pulgin interface.
*
*/


// INCLUDE FILES
#include <remcon/remconbearerobserver.h>
#include <remcon/remconconverterplugin.h>
#include <remconaddress.h>
#include <RemConExtensionApi.h>
#include "atcodec.h"
#include "bmbserviceuid.h"
#include "bmbplugin.h"
#include "debug.h"
#include "bmbcmdlistener.h"
#include <remconcoreapi.h>

class SRemConExtCallHandlingOpToCmd
    {
    public:
        TRemConExtCallHandlingApiOperationId ifId;
        TATId brId;
        TATType brType;
    };

const SRemConExtCallHandlingOpToCmd KRemConExtCallHandlingOpToCmd[] =
    {
    {ERemConExtAnswerCall,       EATA,    EATActionCmd},
    {ERemConExtEndCall,          EATCHUP, EATActionCmd},
    {ERemConExtDialCall,         EATD1,   EATActionCmd},
    {ERemConExtSpeedDial,        EATD2,   EATActionCmd},
    {ERemConExtVoiceDial,        EATBVRA, EATWriteCmd},
    {ERemConExtLastNumberRedial, EATBLDN, EATActionCmd},
    {ERemConExt3WaysCalling,     EATCHLD, EATWriteCmd},
    {ERemConExtGenerateDTMF,     EATVTS,  EATWriteCmd},
    {ERemConExtAnswerEnd,        EATCKPD, EATWriteCmd}
    };

const TInt KRemConExtCallHandlingOpNum = (sizeof(KRemConExtCallHandlingOpToCmd) / sizeof(SRemConExtCallHandlingOpToCmd));


// ================= MEMBER FUNCTIONS =======================
// ---------------------------------------------------------
// CBTAspAccConnection::NewL()
// ---------------------------------------------------------
//
CBmbPlugin* CBmbPlugin::NewL(TBearerParams& aParams)
	{
	CBmbPlugin* self = new (ELeave) CBmbPlugin(aParams);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

// Destructor
CBmbPlugin::~CBmbPlugin()
	{
	TRACE_FUNC_ENTRY
	delete iListener;
	delete iPlayerControlListener;
	if( iConnIndicated )
    	{
        Observer().DisconnectIndicate(iRemConAddress);	
      	}
	TRACE_FUNC_EXIT
	}

TAny* CBmbPlugin::GetInterface(TUid aUid)
	{
    TRACE_FUNC_ENTRY
	TRACE_INFO((_L("aUid = [0x%08X]"), this, aUid))
	TAny* ret = NULL;
	if (aUid == TUid::Uid(KRemConBearerInterface1))
		{
		ret = reinterpret_cast<TAny*>(
			static_cast<MRemConBearerInterface*>(this)
			);
		}
	TRACE_INFO((_L("ret = [0x%08X]"), ret))
	TRACE_FUNC_EXIT
	return ret;
	}

TInt CBmbPlugin::GetResponse(
    TUid& /*aInterfaceUid*/, 
	TUint& /*aTransactionId*/,
	TUint& /*aOperationId*/, 
	RBuf8& /*aData*/,
	TRemConAddress& /*aAddr*/)
	{
	TRACE_FUNC
	return KErrNotSupported;
	}

TInt CBmbPlugin::SendCommand(
    TUid /*aInterfaceUid*/,
	TUint /*aOperationId*/,
	TUint /*aTransactionId*/,
	RBuf8& /*aData*/,
	const TRemConAddress& /*aAddr*/)
	{
    TRACE_FUNC
    return KErrNotSupported;
	}

void CBmbPlugin::DataFromRemote(const TDesC8& aATCmd)
    {
    TRACE_FUNC_ENTRY
    if (!iConnIndicated)
        {
        iConnIndicated = ETrue;
        Observer().ConnectIndicate(iRemConAddress);
        }
    
    CATCommand *tmpCmd = NULL;
    TRAPD(err,  tmpCmd = CATCommand::NewL(aATCmd));
    if(!err)
        {
        TRACE_INFO((_L8("DataFromRemote: %S, Id: %d, type %d"), &aATCmd, tmpCmd->Id(), tmpCmd->Type()));
        for (TInt i = 0; i < KRemConExtCallHandlingOpNum; i++)
            {
            if (KRemConExtCallHandlingOpToCmd[i].brId == tmpCmd->Id() && 
                tmpCmd->Type() == KRemConExtCallHandlingOpToCmd[i].brType)
                {
                iOperationId = (TUint)KRemConExtCallHandlingOpToCmd[i].ifId;
                break;
                }
            }
    	
    	iInterfaceUid = TUid::Uid(KRemConExtCallHandlingApiUid);
    	iTransactionId = Observer().NewTransactionId();

        if (tmpCmd->ParamNum())
            {
            TATParam param;
            tmpCmd->Parameter(0, param);
            iInData.Copy(param.Des());
            }
        else
            {
            iInData = KNullDesC8;
            }

        TRACE_INFO((_L8("  iInterfaceUid 0x%08X, iOperationId %d, iInData '%S'"), 
            iInterfaceUid, iOperationId, &iInData))
            
    	err = Observer().NewCommand(iRemConAddress);
    	TRACE_INFO((_L("NewCommand return %d"), err))
        delete tmpCmd;      
        }
    if (err)
        {
        TPckgBuf<TInt> pckg(err);
        iListener->HandlingDataCompleted( pckg );
        }
    TRACE_FUNC_EXIT
    }

TInt CBmbPlugin::GetCommand(
    TUid& aInterfaceUid,
	TUint& aTransactionId,
	TUint& aOperationId,
	RBuf8& aData,
	TRemConAddress& aAddr)
	{
	TRACE_FUNC
	aInterfaceUid = iInterfaceUid;
	aTransactionId = iTransactionId;
    aOperationId = iOperationId;
    TInt err = aData.Create(iInData);
    if (err)
        {
        TRACE_ERROR((_L("RBuf8::Create err %d"), err))
        }
    aAddr = iRemConAddress;
	return err;
	}

TInt CBmbPlugin::SendResponse(
    TUid aInterfaceUid, 
	TUint aOperationId,
	TUint /*aTransactionId*/,
	RBuf8& aData,
	const TRemConAddress& aAddr)
	{
	TRACE_FUNC
	TRACE_INFO((_L8("interface [0x%08X], op %d"), 
	    aInterfaceUid, aOperationId))
	if (aInterfaceUid.iUid == KRemConCoreApiUid)
	    {
	    aData.Close();
	    return KErrNone;
	    }
	if (aInterfaceUid.iUid == KRemConExtCallHandlingApiUid && 
	    iRemConAddress == aAddr && 
	    iOperationId == aOperationId)
	    {
        iListener->HandlingDataCompleted( aData );
        aData.Close();
        return KErrNone;
	    }
	return KErrNotFound;
	}

void CBmbPlugin::ConnectRequest(const TRemConAddress& aAddr)
	{
	TRACE_FUNC_ENTRY
	TRACE_INFO((_L8("Request to connect to %S"), &(aAddr.Addr())))
    Observer().ConnectConfirm(aAddr, KErrNotSupported);
    TRACE_FUNC_EXIT
	}

void CBmbPlugin::DisconnectRequest(const TRemConAddress& aAddr)
	{
	TRACE_FUNC_ENTRY
	TRACE_INFO((_L8("Request to disconnect %S"), &(aAddr.Addr())))
   	Observer().DisconnectConfirm(aAddr, KErrNotSupported);
	TRACE_FUNC_EXIT
	}

void CBmbPlugin::ClientStatus(TBool /*aControllerPresent*/, TBool /*aTargetPresent*/)
	{
	TRACE_FUNC
	}

TSecurityPolicy CBmbPlugin::SecurityPolicy() const
	{
	return TSecurityPolicy(ECapabilityLocalServices);
	}
void CBmbPlugin::Pause()
    {
    TRACE_FUNC_ENTRY
    if (!iConnIndicated)
        {
        iConnIndicated = ETrue;
        Observer().ConnectIndicate(iRemConAddress);
        }
    
    iOperationId = (TUint) ERemConCoreApiPause;
    iInterfaceUid = TUid::Uid(KRemConCoreApiUid);
    iTransactionId = Observer().NewTransactionId();

    iInData = _L8("a");
    iInData[0] = ERemConCoreApiButtonClick;

    TRACE_INFO((_L8("  iInterfaceUid 0x%08X, iOperationId %d"), 
        iInterfaceUid, iOperationId))
	iRemConAddress.Addr() = KBTAudioServiceName;
	TInt err = Observer().NewCommand(iRemConAddress);
    TRACE_INFO((_L("NewCommand return %d"), err))
	TRACE_FUNC_EXIT
    }

void CBmbPlugin::Play()
    {
    TRACE_FUNC_ENTRY
    if (!iConnIndicated)
        {
        iConnIndicated = ETrue;
        Observer().ConnectIndicate(iRemConAddress);
        }
    
    iOperationId = (TUint) ERemConCoreApiPlay;
    iInterfaceUid = TUid::Uid(KRemConCoreApiUid);
    iTransactionId = Observer().NewTransactionId();

    iInData = _L8("a");
    iInData[0] = ERemConCoreApiButtonClick;

    TRACE_INFO((_L8("  iInterfaceUid 0x%08X, iOperationId %d"), 
        iInterfaceUid, iOperationId))
	iRemConAddress.Addr() = KBTAudioServiceName;
	TInt err = Observer().NewCommand(iRemConAddress);
    TRACE_INFO((_L("NewCommand return %d"), err))
	TRACE_FUNC_EXIT
    }

CBmbPlugin::CBmbPlugin(TBearerParams& aParams)
:	CRemConBearerPlugin(aParams)
	{
	}

void CBmbPlugin::ConstructL()
	{
	TRACE_FUNC_ENTRY
    iRemConAddress.BearerUid() = Uid();
    iListener = CBmbCmdListener::NewL(*this);
    iPlayerControlListener = CBTAudioPlayerControlListener::NewL(*this);
    iRemConAddress.Addr() = KBTAudioServiceName;
    TRACE_FUNC_EXIT
	}

// End of file
