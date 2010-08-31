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
 * Description: Bluetooth Indicator Plugin Class 
 *
 */

#ifndef BTINDICATORPLUGIN_H
#define BTINDICATORPLUGIN_H


#include <hbindicatorplugininterface.h>
#include <hbindicatorinterface.h>

class HbTranslator;

/**
 * Bluetooth indicator plugin class. 
 * Main class of bluetooth indicator plugin. 
 */
class BTIndicatorPlugin : public QObject, public HbIndicatorPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(HbIndicatorPluginInterface)

public:
    /**
     * Constructor
     */
    BTIndicatorPlugin();
     
     /**
      * Destructor
      */
    ~BTIndicatorPlugin();
    
    /**
     * @see HbIndicatorPluginInterface
     */
    QStringList indicatorTypes() const;
    
    /**
     * @see HbIndicatorPluginInterface
     */
    bool accessAllowed(const QString &indicatorType,
            const QVariantMap &securityInfo) const;   
    /**
     * @see HbIndicatorPluginInterface
     */
    HbIndicatorInterface* createIndicator(const QString &indicatorType);
    
    /**
     * @see HbIndicatorPluginInterface
     */
    int error() const;
    
private:
    Q_DISABLE_COPY(BTIndicatorPlugin)
    
    /**
     * Error value
     */
    int mError;
    
    /**
     * Indicator types 
     * supported indicator types. 
     */
    QStringList mIndicatorTypes;
    
    HbTranslator* mIndicatorTranslator;
};


#endif /* BTINDICATORPLUGIN_H */
