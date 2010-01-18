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
* Description:  BT Remote Control Controller class declaration.
*
*/


#ifndef BTRCCPLUGIN_H
#define BTRCCPLUGIN_H

//  INCLUDES
#include <es_sock.h>
#include <bttypes.h>
#include <e32std.h>


// Implement Interface provided by 'BT Accessory Server' component. E-com framework
// uses this interface to load this plugin
#include <btaccPlugin.h>

// FORWARD DECLARATIONS
class CBTRCCLinker;

// CLASS DECLARATION

/**
*  The main controller for BT audio. Maintains e.g. the internal state of the
*  Symbian side BT audio system and controls the serving of requests received
*  both from the Symbian side and the DOS side of the system.
*/
NONSHARABLE_CLASS(CBTRCCPlugin) 
    : public CBTAccPlugin
  	{
  	
  	public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTRCCPlugin* NewL(TPluginParams& aParams);
        
        /**
        * Destructor.
        */
        ~CBTRCCPlugin();
        
      private: // Functions from CBTAccPlugin

        void ConnectToAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

        void CancelConnectToAccessory(const TBTDevAddr& aAddr);

    	/** 
    	* Called by BT Acc Sever to disconnect from mono or stereo accessory
    	* @param aAddr Bluetooth Device address of the remote device
    	*/
    	virtual void DisconnectAccessory(const TBTDevAddr& aAddr,TRequestStatus& aStatus);

       	/** 
    	* BT Acc Server checks the type of this plugin
    	@return TProfiles Type of this plugin
    	*/
    	virtual TProfiles PluginType();
    	
    	/** 
		* Called by BT Acc Sever to inform plugin that accessory is in use. 
		*/
		virtual void AccInUse();
        
        void ActivateRemoteVolumeControl();

        void DeActivateRemoteVolumeControl();
        
    private:

        /**
        * C++ default constructor.
        * @param aParms T-class used for call-back to BT Acc Server
        */
        CBTRCCPlugin(TPluginParams& aParams);

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

    private:    // Data            
             
        CBTRCCLinker* iLinker; // owned; Pointer to the linker class
    };


#endif      // BTRCCPLUGIN_H
            
// End of File
