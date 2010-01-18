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
* Description:  Retrieves accessory information from Bluetooth Accessory Server
*
*/


// INCLUDE FILES
#include <btaccTypes.h>
#include "BTAccInfoRequester.h"
#include "debug.h"

// ============================= LOCAL FUNCTIONS ===============================

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTAccInfoRequester::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTAccInfoRequester* CBTAccInfoRequester::NewL(MBTAccInfoNotifier& aNotifier)
    {
    CBTAccInfoRequester* self = new (ELeave) CBTAccInfoRequester(aNotifier);
    return self;
    }
    
// Destructor
CBTAccInfoRequester::~CBTAccInfoRequester()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    iBTAccClient.Close();
    TRACE_FUNC_EXIT
    }

// -----------------------------------------------------------------------------
// CBTAccInfoRequester::GetBTAccInfo
// -----------------------------------------------------------------------------
//
void CBTAccInfoRequester::GetBTAccInfoL(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC;
    if (IsActive())
        {
        LEAVE(KErrInUse);
        }

    TInt err = iBTAccClient.Connect();
    if (err != KErrNone && err != KErrAlreadyExists)
        {
        LEAVE(err);
        }
        
    iAccInfo.iAddr = aAddr; 
    iAccInfo.iSuppProfiles = 0x00;
    iAccInfo.iConnProfiles = 0x00; 
    iAccInfo.iModel = 0x00;
    iAccInfo.iCapturingSupport = EFalse;
    for (TInt x=0; x<iAccInfo.iSupportedFeatures.Count(); x++)
        {
        iAccInfo.iSupportedFeatures.At(x) = 0x00;    
        }
    iBTAccClient.GetInfoOfConnectedAcc(iAccInfoPckg, iStatus);
    SetActive();
    }

// -----------------------------------------------------------------------------
// CBTAccInfoRequester::RunL
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CBTAccInfoRequester::RunL()
    {
    TRACE_FUNC;
    TASYBTAccInfo asyInfo;
    if (iStatus == KErrNone)
        {
        asyInfo.iDeviceInfo = iAccInfo.iModel;
        asyInfo.iDeviceType = iAccInfo.iDeviceType;
        TUint32 supportedProfiles = iAccInfo.iSuppProfiles;
        asyInfo.iProfiles = 0;
        asyInfo.iReserved = 0;
        
        if (supportedProfiles & (TUint32)EHSP)
            {
            asyInfo.iProfiles |= KAccInfoHSP;
            }
        if (supportedProfiles & (TUint32)EHFP)
            {
            asyInfo.iProfiles |= KAccInfoHFP;
            asyInfo.iReserved |= KAccInfoLatency;
            }
        if (supportedProfiles & (TUint32)EStereo)
            {
            asyInfo.iProfiles |= KAccInfoA2DP;
            }
        if (supportedProfiles & (TUint32)ERemConTG)
            {
            asyInfo.iProfiles |= KAccInfoAVRCP;
            }        

        asyInfo.iSupportedFeatures[EAccInfoHSPIndex] = iAccInfo.iSupportedFeatures[TAccInfo::EHSPIndex];
        asyInfo.iSupportedFeatures[EAccInfoHFPIndex] = iAccInfo.iSupportedFeatures[TAccInfo::EHFPIndex];
        asyInfo.iSupportedFeatures[EAccInfoAVRCPIndex] = iAccInfo.iSupportedFeatures[TAccInfo::ERemConTGIndex];
        asyInfo.iSupportedFeatures[EAccInfoA2DPIndex] = iAccInfo.iSupportedFeatures[TAccInfo::EStereoIndex];
        

        if (iAccInfo.iCapturingSupport)
            {
            asyInfo.iReserved |= KAccInfoDRM;
            }
        
        TRACE_INFO_SEG(
            Trace(_L("ORG Model %d, Type %d, DRM %d, Profiles 0x%08X, SF: HS %d, HF %d, AVRCP %d, A2DP %d"),
                iAccInfo.iModel, 
                iAccInfo.iDeviceType,
                iAccInfo.iCapturingSupport,
                iAccInfo.iSuppProfiles,
                iAccInfo.iSupportedFeatures[TAccInfo::EHSPIndex],
                iAccInfo.iSupportedFeatures[TAccInfo::EHFPIndex],
                iAccInfo.iSupportedFeatures[TAccInfo::ERemConTGIndex],
                iAccInfo.iSupportedFeatures[TAccInfo::EStereoIndex]);
            Trace(_L("ASY Model %d, Type %d, DRM %d, Profiles 0x%08X, SF: HS %d, HF %d, AVRCP %d, A2DP %d"),
                asyInfo.iDeviceInfo, 
                asyInfo.iDeviceType, 
                asyInfo.iReserved,
                asyInfo.iProfiles,
                asyInfo.iSupportedFeatures[EAccInfoHSPIndex],
                asyInfo.iSupportedFeatures[EAccInfoHFPIndex],
                asyInfo.iSupportedFeatures[EAccInfoAVRCPIndex],
                asyInfo.iSupportedFeatures[EAccInfoA2DPIndex]);
           );
        }
    iNotifier.GetBTAccInfoCompletedL(iStatus.Int(), asyInfo);
    }
    
// -----------------------------------------------------------------------------
// CBTAccInfoRequester::DoCancel
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CBTAccInfoRequester::DoCancel()
    {
    TRACE_FUNC;
    }
    
// -----------------------------------------------------------------------------
// CBTAccInfoRequester::CBTAccInfoRequester
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTAccInfoRequester::CBTAccInfoRequester(MBTAccInfoNotifier& aNotifier) 
    : CActive(EPriorityStandard), iNotifier(aNotifier), iAccInfoPckg(iAccInfo)
    {
    CActiveScheduler::Add(this);
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================
    
//  End of File
