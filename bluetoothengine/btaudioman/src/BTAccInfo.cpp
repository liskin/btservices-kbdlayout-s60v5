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
*
*/


// INCLUDE FILES
#include "BTAccInfo.h"

/////////////////////////////// TAccInfo implementation /////////////////////////
// -----------------------------------------------------------------------------
// TAccInfo::TAccInfo
// -----------------------------------------------------------------------------
//
void TAccInfo::SetBDAddress(const TBTDevAddr& aAddr)
    {
    iAddr = aAddr; 
    iSuppProfiles = 0x00;
    iConnProfiles = 0x00;
    iAudioOpenedProfiles = 0x00;
    iModel = 0x00;
    iCapturingSupport = EFalse;
    for (TInt x=0; x<iSupportedFeatures.Count(); x++)
        {
        iSupportedFeatures.At(x) = 0x00;    
        }    
    }
        
// -----------------------------------------------------------------------------
// TAccInfo::GetBDAddress
// -----------------------------------------------------------------------------
//
const TBTDevAddr&  TAccInfo::GetBDAddress()
    {
    return iAddr;     
    }
        

// -----------------------------------------------------------------------------
// TAccInfo::SetCapturingSupport
// -----------------------------------------------------------------------------
//
void TAccInfo::SetCapturingSupport(TBool aCapturingSupport)
    {
    iCapturingSupport = aCapturingSupport;
    }


// -----------------------------------------------------------------------------
// TAccInfo::GetCapturingSupport
// -----------------------------------------------------------------------------
//
TBool TAccInfo::GetCapturingSupport()
    {
    return iCapturingSupport;     
    }



// -----------------------------------------------------------------------------
// TAccInfo::SetSuppportedProfiles
// -----------------------------------------------------------------------------
//
void  TAccInfo::SetSuppportedProfiles(TUint8 aSuppProfiles)
    {
    iSuppProfiles = aSuppProfiles;     
    }
        
// -----------------------------------------------------------------------------
// TAccInfo::SupportedProfiles
// -----------------------------------------------------------------------------
//
TUint8 TAccInfo::SupportedProfiles() const
    {
    return iSuppProfiles;
    }
        
// -----------------------------------------------------------------------------
// TAccInfo::SetConnectedProfiles
// -----------------------------------------------------------------------------
//
void TAccInfo::SetConnectedProfiles(TUint8 aConnProfiles)  
    {
    iConnProfiles = aConnProfiles;     
    }
    
// -----------------------------------------------------------------------------
// TAccInfo::ConnectedProfiles
// -----------------------------------------------------------------------------
//
TUint8 TAccInfo::ConnectedProfiles()  const  
    {
    return iConnProfiles;
    }

// -----------------------------------------------------------------------------
// TAccInfo::SetDeviceType
// -----------------------------------------------------------------------------
//
void TAccInfo::SetDeviceType(TUint8 aDeviceType)
    {
    iDeviceType = aDeviceType;
    }

    
// -----------------------------------------------------------------------------
// TAccInfo::DeviceType
// -----------------------------------------------------------------------------
//
TUint8 TAccInfo::DeviceType() const
    {
    return iDeviceType;        
    }    


// -----------------------------------------------------------------------------
// TAccInfo::SetModel
// -----------------------------------------------------------------------------
//
void TAccInfo::SetModel(TUint16 aModel)
    {
    iModel = aModel;
    }


// -----------------------------------------------------------------------------
// TAccInfo::Model
// -----------------------------------------------------------------------------
//
TUint16 TAccInfo::Model() const
    {
    return iModel;
    }

// -----------------------------------------------------------------------------
// TAccInfo::SetSupportedFeature
// -----------------------------------------------------------------------------
//
void TAccInfo::SetSupportedFeature(TUint16 aSupportedFeature, TInt aPosition)
    {
    iSupportedFeatures.At(aPosition) = aSupportedFeature; 
    }
    
    
// -----------------------------------------------------------------------------
// TAccInfo::SupportedFeatureL
// -----------------------------------------------------------------------------
//    
TUint16 TAccInfo::SupportedFeatureL(TInt aPosition)    const
    {
    if ( aPosition < iSupportedFeatures.Count() )
        {
        return iSupportedFeatures.At(aPosition);            
        }
    else
        {
        User::Leave(KErrOverflow);
        }
    return 0x00;
    }
