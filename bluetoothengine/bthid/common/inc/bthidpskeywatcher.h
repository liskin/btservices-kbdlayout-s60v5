/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*
*
*/


#ifndef BTHIDPSKEYWATCHER_H_
#define BTHIDPSKEYWATCHER_H_
//  INCLUDES
#include <e32base.h>
#include <e32property.h>

class MMouseCursorStatusObserver;

/*
* An abstarct interface that defines a callback method
* for the hid mouse cursor observer. CursorStatusChangedL
* is called by CBTMouseCursorStatusObserver when the mouse device 
* connection state has changed. aStatus represents the new status 
* value.
*/
class MMouseCursorStatusObserver
    {
    public:
    virtual void MouseCursorStatusChangedL(TInt aStatus) = 0;
    };
    
/*
* An observer that monitors KPSUidBluetoothSapConnectionState
* P&S key. The clients can call SubscribeCursorStatusL in order to 
* get a chenged new SAP connection status returned via 
* MSapStatusObserver::SapStatusChangedL.
*/ 
class CBTMouseCursorStatusObserver : public CActive
    {
public:
    /**
    * Two-phased constructor.
    */
    static CBTMouseCursorStatusObserver* NewL();
    /**
    * Destructor.
    */
    virtual ~CBTMouseCursorStatusObserver();
    
    /**
    * Starts listening KPSUidBluetoothSapConnectionState P&S key.
    * When the key value is changed, the new state is indicated
    * through the observer interface (MSapStatusObserver).
    */
    void SubscribeMouseCursorStatusL(MMouseCursorStatusObserver* aObserver);
      
private:
    /**
    * Two-Phase constructor
    */
    void ConstructL();
    
    /**
    * Default constructor
    */
    CBTMouseCursorStatusObserver();
    
    // From CActive
    virtual void DoCancel();
    virtual void RunL();
      
private:
    MMouseCursorStatusObserver* iObserver;
    RProperty iProperty;
    };


#endif /* BTHIDPSKEYWATCHER_H_ */
