/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef BTUIPLUGININTERFACE_H
#define BTUIPLUGININTERFACE_H

#include <aknview.h>
#include <gsbaseview.h>
#include <btdevice.h>

const TUid KCBtuiPluginInterfaceUid    = { 0x1020745F }; 


enum TPluginViewId 
{
    EMainViewId =1,
    EPairedDevicesViewId=2,
    EBlockedDevicesViewId=3
};



/**
* Callback interface for plugins view activation observer
*/
class MBtuiPluginViewActivationObserver
{
public: 
    /**
     *  Called when view is activated
     *
     *  @param aView Activated view id
     */
    virtual void PluginViewActivated(TPluginViewId aViewId)=0;    
};


/** This class implements the CGSPluginInterface which is used 
* to implement the main view as general settings plugin.
*
*@lib BTUIPlugin.dll
*@since S60 v3.1
*/
class CBtuiPluginInterface	: public CGSBaseView
    {    
    public: // Constructors & destructors

        /**
        * Creates new BTUI View plugin having the given UID.
        * Uses Leave code KErrNotFound if implementation is not found.
        *
        * @param aImplementationUid Implementation UID of the plugin to be 
        *        created.
        * @param aModel Pointer to the BTUI Model of BTUI Application.
        */                                                 
        static CBtuiPluginInterface* NewL(
            const TUid aImplementationUid, MBtuiPluginViewActivationObserver* aObserver = NULL);

        /**
        * From CGSPluginInterface
        */            
		void GetCaptionL( TDes& aCaption ) const;

        /**
        * Destructor
        */
        inline ~CBtuiPluginInterface();
        
        /**
         * One ECom plugin of a type of HID device has its unique DeviceClass.
         * The HID plugin has to implement this function to report the plug-in loader of its COD.
         * Otherwise, it won't be added to enable setting view.
         * @return TBTDeviceClass Class of Device defined in btdevice.h
         */        
        inline void SetCOD(TBTDeviceClass& aCOD);
        inline TBTDeviceClass GetCOD() const;
            
    protected: 
    	/** Non leaving constructor
    	*/
        inline CBtuiPluginInterface();

		inline void NewContainerL();

		inline void HandleListBoxSelectionL();

			
    private: // Data

        /**
        * ECOM plugin instance UID.
        */
        TUid iDtor_ID_Key;
        TBTDeviceClass iCOD;
    };

#include "BtuiPluginInterface.inl"

#endif //BTUIPLUGININTERFACE_H
