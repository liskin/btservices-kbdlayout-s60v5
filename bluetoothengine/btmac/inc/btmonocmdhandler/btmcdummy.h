/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: Dummy implementation of MCall  
*
*/

#ifndef BTMCDUMMY_H_
#define BTMCDUMMY_H_

#include <mcall.h>

class MBtmcDummy : public MCall
{
public:
    
    MBtmcDummy() {};    
    ~MBtmcDummy() {};
    
    TInt CallIndex() const { return 0; };     

    /**
     * Call type getter
     *
     * @since S60 v5.1
     * @return the call type
     */    
    CCPCall::TCallType CallType() const {return CCPCall::ECallTypeCSVoice; }; 


    /**
     * Call state getter
     *
     * @since S60 v5.1     
     * @return the call state
     */    
    CCPCall::TCallState CallState() const { return CCPCall::EStateIdle; };
    /**
     * Getter for the service ID of the call.
     *
     * @since S60 v5.1    
     * @return the service ID of the call
     */
    TUint32 ServiceId() const { return 0; };

    /**
     * Call direction getter
     *
     * @since S60 v5.1     
     * @return the call direction
     */
    CCPCall::TCallDirection CallDirection() const  { return CCPCall::EDirectionUnknown; };

    /**
     * Returns information whether call is an emergency call.
     * @since S60 v5.2
     * @param None.
     * @return TBool ETrue call is emergency call else EFalse.
     */
    TBool IsEmergency() const { return EFalse; };

};


#endif /* BTMCDUMMY_H_ */
