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
* Description:  Plugin loader
*
*/



#ifndef BTUIPLUGINMAN_H
#define BTUIPLUGINMAN_H

#include <e32base.h>
#include <btdevice.h>
#include "BtuiPluginInterface.h"
#include "btdevmodel.h"

typedef RPointerArray<CImplementationInformation> RImplInfoPtrArray;
typedef RPointerArray<CBtuiPluginInterface> RPluginPtrArray;

/**
 *  Class CBTUIPluginMan
 *
 *  Load plugins of CBtuiPluginInterface
 *
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CBTUIPluginMan ) 
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 v5.0
     */
    static CBTUIPluginMan* NewL( CAknViewAppUi* aAppUi );

    /**
     * Destructor
     */
    virtual ~CBTUIPluginMan();
     
    /**
     * Get the setting view.
     * @param TBTDevice Get this device's view
     * @return CAknView the view of the specified plugin
     */
    CAknView* GetSettingViewL(TBTDevice& aDevice);
    
    /**
     * Check if the plug-in for specified HID device exists.
     * @param TBTDeviceClass Use ClassOfDevice as the property to search 
     * @return ETrue if exist, otherwise EFalse.
     */
     TBool IsPluginAvaiable(TBTDeviceClass aDeviceClassInfo);

private:

    /**
     * C++ default constructor
     *
     * @since S60 v5.0
     */
    CBTUIPluginMan( CAknViewAppUi* aAppUi );

    /**
     * Symbian 2nd-phase constructor
     *
     * @since S60 v5.0
     */
    void ConstructL();
    
    /**
     * Load plugins which are for HID devices.
     * @param None
     * @return None
     */
    void LoadPluginsL();
    
private: // data

	/**
     * Array of pointers to ECom plug-in information objects.
     */
    RImplInfoPtrArray iPluginInfoArray;

    /**
     * Array of pointers to the BtuiPluginInterface ECom plug-ins loaded by PairedView.
     */
    RPluginPtrArray iPluginArray;
    
    /**
     * Current Application process
     */
    CAknViewAppUi* iAppUi;
    };


#endif // BTUIPLUGINMAN_H
