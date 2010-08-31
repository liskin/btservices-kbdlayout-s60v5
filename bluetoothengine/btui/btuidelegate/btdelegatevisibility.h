/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
*/

#ifndef BTDELEGATEVISIBILITY_H
#define BTDELEGATEVISIBILITY_H

#include <e32base.h>
#include <btengsettings.h>
#include "btabstractdelegate.h"

class BtuiModel;

/*!
    \class BtDelegateVisibility
    \brief the base class for handling Bluetooth visibility management.

    \\sa btuidelegate
 */
class BtDelegateVisibility : public BtAbstractDelegate, public MBTEngSettingsObserver
{
    Q_OBJECT

public:
    explicit BtDelegateVisibility( QObject *parent = 0 );
    
    virtual ~BtDelegateVisibility();

    virtual void exec( const QVariant &params );
    

    
public slots:

private:

    virtual void PowerStateChanged( TBTPowerStateValue aState );
    virtual void VisibilityModeChanged( TBTVisibilityMode aState );

private:
    CBTEngSettings* mBtengSettings;
    bool mActiveHandling;
    TBTVisibilityMode mOperation;

private:

    Q_DISABLE_COPY(BtDelegateVisibility)

};

#endif // BTDELEGATEVISIBILITY_H
