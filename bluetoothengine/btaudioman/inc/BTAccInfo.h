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
* Description:  A class that stores audio accessory related information. 
*
*/


#ifndef BTACC_INFO_H
#define BTACC_INFO_H

// INCLUDES
#include <bttypes.h>  // TBTDevAddr

// DATA TYPES

// CLASS DECLARATION
class TAccInfo
    {

public: 

    enum TSupportedFeaturesIndex
        {
        ERemConTGIndex    = 3,
        EStereoIndex      = 2,
        EHSPIndex         = 1,
        EHFPIndex         = 0
        };

    enum TAvrcpVersion
        {
        EAvrcpVersionNo14 = 0x0103,
        EAvrcpVersion14   = 0x0104
        };
    
public:
    
    /** 
    * Accessor for the 
    * @return 
    */
    void SetBDAddress(const TBTDevAddr& aAddr);
    
    /** 
    * Accessor for the 
    * @return 
    */
    const TBTDevAddr& GetBDAddress(); 
    
    /** 
    * Accessor for the 
    * @return 
    */
    void SetCapturingSupport(TBool aCapturingSupport); 
    
    /** 
    * Accessor for the 
    * @return 
    */
    TBool GetCapturingSupport(); 
    
    /** 
    * Accessor for the 
    * @return 
    */
    void SetSuppportedProfiles(TUint8 aSuppProfiles);
    
    /** 
    * Accessor for the 
    * @return 
    */
    TUint8 SupportedProfiles() const;
    
    /** 
    * Accessor for the 
    * @return 
    */
    void SetConnectedProfiles(TUint8 aConnProfiles);
    
    /** 
    * Accessor for the 
    * @return 
    */
    TUint8 ConnectedProfiles() const;
    
    /** 
    * Accessor for the 
    * @return 
    */
    void SetDeviceType(TUint8 aDeviceType); 
    
    /** 
    * Accessor for the 
    * @return 
    */
    TUint8 DeviceType() const;
    
    /** 
    * Accessor for the 
    * @return 
    */
    void SetModel(TUint16 aModel);
    
    /** 
    * Accessor for the 
    * @return 
    */
    TUint16 Model() const;
    
    /** 
    * Accessor for the 
    * @return 
    */
    void SetSupportedFeature(TUint16 aSupportedFeature, TInt aPosition);
    
    /** 
    * Accessor for the 
    * @return 
    */
    TUint16 SupportedFeatureL(TInt aPosition) const;
     
public:
    TBTDevAddr iAddr;
    TBool iCapturingSupport; // If accessory is capable of capturing or recording (DRM feature)
    TUint8 iSuppProfiles;    // Supported profile in the remote device
    TUint8 iConnProfiles;    // Connected Profiles from the remote device point of view
    TUint8 iAudioOpenedProfiles;    // The profiles that currently have audio link.
    TUint8 iDeviceType;  // 0x1 headset device, 0x2 carkit (@see AccPolGenericid.h)
    TUint16 iModel;
    TUint16 iAvrcpVersion;  // 0x00 AVRCP1.3, 0x01 AVRCP1.4
    TFixedArray<TUint16,8> iSupportedFeatures;  // position of objects matched TSupportedFeaturesIndex
    };

#endif      // BTACC_INFO_H

// End of File
