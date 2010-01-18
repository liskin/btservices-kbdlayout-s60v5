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
* Description: 
*                 Struct for initialisation parameters for plugins. The derived 
* (concrete) plugin will receive an instance of this in its NewL. It should pass 
* it to the CBTAccPlugin base class constructor.
*
*/


#ifndef BTACC_PARAMS_H
#define BTACC_PARAMS_H

// FORWARD DECLARATION
class MBTAccObserver;


class TPluginParams
    {
public:
    /**
    Constructor.
    @param aImplementationUid The implementation UID of the plugin.
    @param aObserver Observer.
    */
    TPluginParams(const TUid aImplementationUid, MBTAccObserver& aObserver);

    /** Destructor. */
    ~TPluginParams();

public:
    /** 
    Accessor for the implementation UID.
    @return Implementation UID.
    */
    TUid ImplementationUid() const;

    /**
    Accessor for the observer.
    @return Observer.
    */
    MBTAccObserver& Observer() const;

private:
    const TUid iImplementationUid;
    MBTAccObserver& iObserver;
    };
    
#include "btaccParams.inl"

#endif // BEARERPARAMS_H
