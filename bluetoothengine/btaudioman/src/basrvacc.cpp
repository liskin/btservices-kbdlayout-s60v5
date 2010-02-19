/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of an accessory management.
*
*/


// INCLUDE FILES
#include "basrvacc.h"
#include "basrvaccman.h"
#include "basrvrvc.h"
#include "basrvaccstatequery.h"
#include "basrvpowercontrolagent.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

CBasrvAcc* CBasrvAcc::NewL(CBasrvAccMan& aAccMan)
    {
    CBasrvAcc* self = CBasrvAcc::NewLC(aAccMan);
    CleanupStack::Pop();
    return self;
    }

CBasrvAcc* CBasrvAcc::NewLC(CBasrvAccMan& aAccMan)
    {
    CBasrvAcc* self = new(ELeave) CBasrvAcc(aAccMan);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CBasrvAcc::~CBasrvAcc()
    {
    delete iState;
    delete iRvc;
    delete iPowerController;
    TRACE_FUNC
    }
    
const TBTDevAddr& CBasrvAcc::Remote()
    {
    return iAccInfo.iAddr;
    }

void CBasrvAcc::ConnectL(const TBTDevAddr& aAddr)
    { 
    TRACE_FUNC
    if (!iState)
        {
        iAccInfo.SetBDAddress(aAddr);
        iState = CBasrvAccStateQuery::NewL(*this, ETrue);
        iState->EnterL();
        }
    else
        {
        iState->ConnectL(aAddr);
        }
    }

void CBasrvAcc::CancelConnect()
    {
    TRACE_FUNC
    if (iState)
        {
        iState->CancelConnect();
        }
    }

void CBasrvAcc::DisconnectL()
    {
    TRACE_FUNC
    if (iState)
        {
        iState->DisconnectL();
        }
    }

void CBasrvAcc::AccessoryConnectedL(const TBTDevAddr& aAddr, TProfiles aProfile)
    {
    TRACE_FUNC
    if (!iState)
        {
        iAccInfo.SetBDAddress(aAddr);
        iAccInfo.iConnProfiles = aProfile;
        iAccInfo.iSuppProfiles = aProfile;
        iState = CBasrvAccStateQuery::NewL(*this, EFalse);
        iState->EnterL();
        }
    else if (aAddr == iAccInfo.iAddr)
        {
        iState->AccessoryConnected(aProfile);
        }
    }

void CBasrvAcc::AccOpenedAudio(TProfiles aProfile)
    {
    TRACE_FUNC
    __ASSERT_ALWAYS(iState, User::Panic(KPanicCategory, EInvalidNullState));
    iState->AccOpenedAudio(aProfile);
    }

void CBasrvAcc::AccClosedAudio(TProfiles aProfile)
    {
    TRACE_FUNC
    __ASSERT_ALWAYS(iState, User::Panic(KPanicCategory, EInvalidNullState));
    iState->AccClosedAudio(aProfile);
    }
    
void CBasrvAcc::AccessoryDisconnectedL(TProfiles aProfile)
    {
    TRACE_FUNC
    __ASSERT_ALWAYS(iState, User::Panic(KPanicCategory, EInvalidNullState));
    iState->AccessoryDisconnectedL(aProfile);
    }

void CBasrvAcc::OpenAudioL(TAccAudioType aType)
    {
    TRACE_FUNC
    __ASSERT_ALWAYS(iState, User::Panic(KPanicCategory, EInvalidNullState));
    iState->OpenAudioL(aType);
    }

void CBasrvAcc::CloseAudioL(TAccAudioType aType)
    {
    TRACE_FUNC
    __ASSERT_ALWAYS(iState, User::Panic(KPanicCategory, EInvalidNullState));
    iState->CloseAudioL(aType);
    }

const TAccInfo* CBasrvAcc::AccInfo(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    if (iAccInfo.iAddr == aAddr)
        {
        return &iAccInfo;
        }
    return NULL;
    }

void CBasrvAcc::ChangeStateL(CBasrvAccState* aNewState)
    {
    TRACE_FUNC_ENTRY
    delete iState;
    iState = aNewState;
    if (iState)
        {
        TInt err = KErrNone;
        TRAP(err, iState->EnterL());
        if (err)
            {
            ChangeStateL(iState->ErrorOnEntry(err));
            }
        }
    if (!iState)
        {
        iAccMan.AccObsoleted(this, iAccInfo.iAddr);
        }
    TRACE_FUNC_EXIT
    }
    
CBasrvAccMan& CBasrvAcc::AccMan()
    {
    return iAccMan;
    }


TAccInfo& CBasrvAcc::AccInfo()
    {
    return iAccInfo;
    }

TBTEngConnectionStatus CBasrvAcc::ConnectionStatus() const
    {
    if (iState)
        {
        return iState->ConnectionStatus();
        }
    
    return EBTEngNotConnected;
    }

void CBasrvAcc::RequestSniffMode()
    {
    if (!(AccInfo().iSuppProfiles & EHFP) && !(AccInfo().iSuppProfiles & EStereo))
        {
        return; // No power mode handling with HSP
        }
    
    if (!iPowerController)
        {
        TRAP_IGNORE(iPowerController = CBasrvPowerControlAgent::NewL(iAccInfo.iAddr));
        }
    
    if (iPowerController)
        {
        iPowerController->RequestSniffMode();
        }
    }

void CBasrvAcc::RequestActiveMode()
    {
    if (!(AccInfo().iSuppProfiles & EHFP) && !(AccInfo().iSuppProfiles & EStereo))
        {
        return; // No power mode handling with HSP
        }
    if (!iPowerController)
        {
        TRAP_IGNORE(iPowerController = CBasrvPowerControlAgent::NewL(iAccInfo.iAddr));
        }
    if (iPowerController)
        {
        iPowerController->RequestActiveMode();
        }
    }

void CBasrvAcc::CancelPowerModeControl()
    {
    if (iPowerController)
        {
        iPowerController->CancelPowerModeControl();
        }
    }

void CBasrvAcc::NotifyLinkChange2Rvc()
    {
    if (!iRvc)
        iRvc = CBasrvRvc::New(*this);
    if (iRvc)
        {
        iRvc->Update(iAccInfo.iConnProfiles, iAccInfo.iAudioOpenedProfiles);
        }
    }

void CBasrvAcc::PreventLowPowerMode()
    {
    TRACE_FUNC
    if (!iPowerController)
        {
        TRAP_IGNORE(iPowerController = CBasrvPowerControlAgent::NewL(iAccInfo.iAddr));
        }
    if (iPowerController)
        {
        iPowerController->PreventLowPowerMode();
        }
    }
    
void CBasrvAcc::AllowLowPowerMode()
    {
    TRACE_FUNC
    if (iPowerController)
        {
        iPowerController->AllowLowPowerMode();
        }
    }


TInt CBasrvAcc::SupportedFeature( TProfiles aProfile )
    {
    return iAccInfo.SupportedFeature( aProfile );
    }

CBasrvAcc::CBasrvAcc(CBasrvAccMan& aAccMan)
    : iAccMan(aAccMan)
    {
    TRACE_FUNC
    iAccInfo.SetBDAddress(TBTDevAddr());
    }

void CBasrvAcc::ConstructL()
    {
    }

//  End of File  
