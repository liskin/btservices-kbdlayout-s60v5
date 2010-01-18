/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Baseband power mode control agent for plugins
*
*/


#ifndef C_BASRVPOWERCONTROLAGENT_H
#define C_BASRVPOWERCONTROLAGENT_H

#include <e32base.h>
#include <bt_sock.h>  // RBTPhysicalLinkAdapter
#include "basrvactive.h"

NONSHARABLE_CLASS(CBasrvPowerControlAgent) : public CBase, public MBasrvActiveObserver
	{
public:
    static CBasrvPowerControlAgent* NewL(const TBTDevAddr& aAddr);
    
    ~CBasrvPowerControlAgent();
    
    void RequestSniffMode();

    void RequestActiveMode();

    void CancelPowerModeControl();
    
    void PreventLowPowerMode();
    
    void AllowLowPowerMode();
    
private:
    // From MBasrvActiveObserver
    
    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void RequestCompletedL(CBasrvActive& aActive);
    
    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void CancelRequest(CBasrvActive& aActive);
    
    
private:
    CBasrvPowerControlAgent(const TBTDevAddr& aAddr);
    
    void ConstructL();
    
    void DoPowerControl();

    void DosniffRequest();

private:    
    enum TPowerControlState
        {
        EPowerModeDefault,
        ESniffModeTiming,
        ESniffModeRequested,
        EActiveModeRequested,
        };
private:
    RSocketServ iSockServ;
    TBTDevAddr iAddr;
    TPowerControlState iState;
    RBTPhysicalLinkAdapter iPlAda;
    TBTBasebandEvent iBbEvent;
    RTimer iTimer;
    CBasrvActive* iSniffDelayTimer;
    CBasrvActive* iBbEventer;
    };
	
#endif	// C_BASRVPOWERCONTROLAGENT_H