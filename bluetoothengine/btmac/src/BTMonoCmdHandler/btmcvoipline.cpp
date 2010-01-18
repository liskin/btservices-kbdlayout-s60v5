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
* Description:  VoIP call status handling. 
*
*/

#include "btmcvoipline.h"
#include "btmc_defs.h"
#include "debug.h"
#include "btmccallstatus.h"
#include <ccallinformation.h>
#include <mcall.h>
#include <ccallinfoiter.h>
#include <etelmm.h>

// ==========================================================
// CBtmcVoIPLine::NewL
// 
// ==========================================================
//
CBtmcVoIPLine* CBtmcVoIPLine::NewL(CBtmcCallStatus& aParent, RMobilePhone& aPhone) 
    {
    CBtmcVoIPLine* self = new (ELeave) CBtmcVoIPLine(aParent, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();   
    return self;
    }

// ==========================================================
// CBtmcVoIPLine::ConstructL
// 
// ==========================================================
//
void CBtmcVoIPLine::ConstructL()
    {
    iInfo = CCallInformation::NewL();
    iInfo->NotifyCallInformationChanges(*this);
    }

// ==========================================================
// CBtmcVoIPLine::Phone
// 
// ==========================================================
//
RMobilePhone& CBtmcVoIPLine::Phone()
    {    
    iPhone.Close();
    return iPhone;
    }

// ==========================================================
// CBtmcVoIPLine::CallInformationL
// 
// ==========================================================
//
const MCall& CBtmcVoIPLine::CallInformationL()
    {
    TRACE_FUNC_ENTRY
    CCallInfoIter& iter = iInfo->GetCallsL();
    MCall *returnValue = NULL;
    for( iter.First(); !iter.IsDone(); iter.Next() )
       {       
       const MCall& call( iter.Current() );
       if(call.CallType() == CCPCall::ECallTypePS)
           {           
           returnValue = const_cast<MCall*> (&call); 
           break;                     
           }            
       }      
    TRACE_FUNC_EXIT
    return *returnValue;
    }

// ==========================================================
// CBtmcVoIPLine::CallStatus
// 
// ==========================================================
//
TInt CBtmcVoIPLine::CallStatusL() const
    {
    TRACE_FUNC_ENTRY
    CCallInfoIter& iter = iInfo->GetCallsL();    
    TInt mask = 0;    
    for( iter.First(); !iter.IsDone(); iter.Next() )
       {
       const MCall& call( iter.Current() );       
       if(call.CallType() == CCPCall::ECallTypePS)
           {                      
           TInt tempState =  call.CallState();                     
           switch(call.CallState())
               {
               /** Indicates initial call state after call creation. Basic state. */
               case CCPCall::EStateIdle:
                   break;
                /** Indicates that the MT call is ringing but not answered yet by the local user. Basic state.*/
               case CCPCall::EStateRinging:
                   {
                   mask |= KCallRingingBit;
                   break;
                   }       
                /** MO Call: the network notifies to the MS that the remote party is now ringing. Basic state.*/
               case CCPCall::EStateConnecting:
                   {
                   mask |= KCallConnectingBit;
                   break;
                   }
                /** Indicates that call is connected and active. Basic state.*/
               case CCPCall::EStateConnected:
                   {
                   mask |= KCallConnectedBit;
                   break;
                   }
                /** Indicates that call is disconnecting. Basic state.*/
               case CCPCall::EStateDisconnecting:
                   break;

                /** Indicates that the call is connected but on hold. Basic state.*/
               case CCPCall::EStateHold:
                   {
                   mask |= KCallHoldBit;
                   break;
                   }
                /** Call is transfering. Optional state.*/
               case CCPCall::EStateTransferring:
                /** MO call is being forwarded at receiver end. Optional state. */
               case CCPCall::EStateForwarding:
                /** Call is queued locally. Optional state. */
               case CCPCall::EStateQueued:
                   break;
                /** Indicates that the local user has answered the MT call but the network has not
                    acknowledged the call connection yet.  Must be sent after MCCPCall::Answer method has been completed. */
               case CCPCall::EStateAnswering:
                   {
                   mask |= KCallAnsweringBit;
                   break;          
                   }
               
                /** Indicates that the call is dialing. Must be sent after MCCPCall::Dial method has been completed. */
               case CCPCall::EStateDialling:            
                   {           
                   mask |= KCallDiallingBit;
                   break;
                   }                      
               }           
           }
       }
    TRACE_INFO((_L("Voip line, CALL STATUS 0x%08x"), mask))    
    TRACE_FUNC_EXIT
    return mask;
    }

// ==========================================================
// CBtmcVoIPLine::~CBtmcVoIPLine
// Destructor
// ==========================================================
//
CBtmcVoIPLine::~CBtmcVoIPLine()
    {
    if (iInfo != NULL)
        {
        iInfo->CancelNotification();
        delete iInfo;
        }    
    }

// ==========================================================
// CBtmcVoIPLine::ActiveCalls
// Destructor
// ==========================================================
//
const RPointerArray<CBtmcCallActive>& CBtmcVoIPLine::ActiveCalls() const
    {
    TRACE_FUNC
    return iCallActives;
    }

// ==========================================================
// CBtmcVoIPLine::CallInformationChanged
// Callback
// ==========================================================
//
void CBtmcVoIPLine::CallInformationChanged()
    {
    TRACE_FUNC
    TRAP_IGNORE( CallInformationChangedL() );
    }

// ==========================================================
// CBtmcVoIPLine::CBtmcVoIPLine
// Constructor
// ==========================================================
//
CBtmcVoIPLine::CBtmcVoIPLine(CBtmcCallStatus& aParent, RMobilePhone& aPhone) : iParent(aParent), iPhone(aPhone)
    {    
    }

// ==========================================================
// CBtmcVoIPLine::IsVoIP
// Returns always ETrue  
// ==========================================================
//
TBool CBtmcVoIPLine::IsVoip()
    {
    return ETrue; 
    }

// ==========================================================
// CBtmcVoIPLine::CallInformationChangedL
// Pass the call status change to the owner of this if necessary
// ==========================================================
//
void CBtmcVoIPLine::CallInformationChangedL()
    {
    TRACE_FUNC_ENTRY
    _LIT(KVoip, "VoIP");
    _LIT(KVoipCall, "VoIP Call");        
    TPtrC voipPtr(KVoip);    

    CCallInfoIter& iter = iInfo->GetCallsL();    
    for( iter.First(); !iter.IsDone(); iter.Next() )
        {
        const MCall& call( iter.Current() );
        if(call.CallType() == CCPCall::ECallTypePS)
            {            
            switch(call.CallState())
                {
                /** Indicates initial call state after call creation. Basic state. */
                case CCPCall::EStateIdle:
                    iParent.HandleMobileCallEventL( voipPtr , KVoipCall, RMobileCall::EStatusIdle);
                    break;
                    /** Indicates that the MT call is ringing but not answered yet by the local user. Basic state.*/
                case CCPCall::EStateRinging:
                    iParent.HandleMobileCallEventL(voipPtr, KVoipCall, RMobileCall::EStatusRinging);
                    break;
                    /** MO Call: the network notifies to the MS that the remote party is now ringing. Basic state.*/
                case CCPCall::EStateConnecting:
                    iParent.HandleMobileCallEventL(voipPtr, KVoipCall, RMobileCall::EStatusConnecting);
                    break;
                    /** Indicates that call is connected and active. Basic state.*/
                case CCPCall::EStateConnected:                           
                    iParent.HandleMobileCallEventL(voipPtr, KVoipCall, RMobileCall::EStatusConnected);
                    break;
                    /** Indicates that call is disconnecting. Basic state.*/
                case CCPCall::EStateDisconnecting:
                    iParent.HandleMobileCallEventL(voipPtr, KVoipCall, RMobileCall::EStatusDisconnecting);
                    break;
                    /** Indicates that the call is connected but on hold. Basic state.*/
                case CCPCall::EStateHold:
                    iParent.HandleMobileCallEventL(voipPtr, KVoipCall, RMobileCall::EStatusHold);
                    break;
                    /** Call is transfering. Optional state.*/
                case CCPCall::EStateTransferring:
                    /** MO call is being forwarded at receiver end. Optional state. */
                case CCPCall::EStateForwarding:
                    /** Call is queued locally. Optional state. */
                case CCPCall::EStateQueued:
                    break;
                    /** Indicates that the local user has answered the MT call but the network has not
            acknowledged the call connection yet.  Must be sent after MCCPCall::Answer method has been completed. */
                case CCPCall::EStateAnswering:
                    iParent.HandleMobileCallEventL(voipPtr, KVoipCall, RMobileCall::EStatusAnswering);
                    break;
                    /** Indicates that the call is dialing. Must be sent after MCCPCall::Dial method has been completed. */
                case CCPCall::EStateDialling:            
                    iParent.HandleMobileCallEventL(voipPtr, KVoipCall, RMobileCall::EStatusDialling);
                    break;
                }                
            }
        }
    TRACE_FUNC_EXIT
    }
