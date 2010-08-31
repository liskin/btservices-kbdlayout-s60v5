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

#include "btdelegatedevname.h"
#include <btsettingmodel.h>
#include <bluetoothuitrace.h>
#include <QRegExp>

BtDelegateDevName::BtDelegateDevName(QObject *parent ) :
    BtAbstractDelegate( NULL, NULL, parent )
{
    TRAP_IGNORE( mBtEngSettings = CBTEngSettings::NewL() );
    Q_CHECK_PTR( mBtEngSettings );

}
    
BtDelegateDevName::~BtDelegateDevName()
{
    delete mBtEngSettings;
}

/*!
    Validate the bluetooth device name given by the user:
    Extra spaces (' ', '\n', '\t' and '\r') from the beginning, 
    middle and the end of the name are always removed;
    the maximum lengthof a name is 30.
 */
bool BtDelegateDevName::validateName(QString &name )
{
    // remove spaces at the beginning and end:
    name = name.trimmed();
    // regular expression of one or more consecutive spaces:
    QRegExp rx("[ \n\t\r]+");
    name.replace( rx, " ");
    if (name.length() > 30 ) {
        name.resize( 30 );
    }
    return name.length() > 0;
}

void BtDelegateDevName::exec( const QVariant &params )
{
    int error = KErrNone;
    QString btDevName = params.toString();
    
    validateName(btDevName);
    
    TPtrC ptrName(reinterpret_cast<const TText*>(btDevName.constData()));
    
    RBuf16 btName;
    error = btName.Create(ptrName.Length());
    
    if(error == KErrNone) {
        btName.Copy(ptrName);
        error = mBtEngSettings->SetLocalName(btName);
        btName.Close();
    }
    
    emit commandCompleted(error, btDevName);
}



