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
 * Description: Bluetooth Indicator class
 *
 */

#ifndef BTINDICATOR_H
#define BTINDICATOR_H


#include <hbindicatorinterface.h>
//#include <xqappmgr.h>
/**
 * Bluetooth indicator class. 
 * Handles client request and showing the indications. 
 */
class BTIndicator : public HbIndicatorInterface
{
public:
    /**
     * Constructor
     */
    BTIndicator(const QString &indicatorType);
    
    /**
     * Destructor
     */
    ~BTIndicator();
    
    /**
     * @see HbIndicatorInterface
     */
    bool handleInteraction(InteractionType type);
    
    /**
     * @see HbIndicatorInterface
     */
    QVariant indicatorData(int role) const;
    
protected:
    /**
     * @see HbIndicatorInterface
     */
    bool handleClientRequest(RequestType type, const QVariant &parameter);
    
private:
//    void launchBTCpSettingView(); 
    
/*private slots:
    void handleReturnValue(const QVariant &returnValue);
    void handleError(int errorCode,const QString &errorMessage);*/
    
private: 

    QString mSecDisplayName;
//    XQApplicationManager mAppMgr;
    int mIndicatorStatus;
    
private:

};

#endif /* BTINDICATOR_H */




