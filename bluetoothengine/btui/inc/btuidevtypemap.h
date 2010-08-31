/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

#ifndef BTUIDEVTYPEMAP_H
#define BTUIDEVTYPEMAP_H

#include <qglobal.h>
#include <btdevice.h>

class BtuiDevProperty
{
public: // type definitions:
    /*
     * Major device property values.
     */
    enum DevMajorProperty {
        NullProperty   = 0x00000000, // device without any specific filter.
        Bonded         = 0x00000001, // device is in registry and bonded with phone
        Blocked        = 0x00000002, // device is in registry and blocked by user
        RecentlyUsed   = 0x00000004, // device is in registry and was used in last 30 days.
        Trusted        = 0x00000008, // device is in registry and authorized by user.
        InRegistry     = 0x00000010, // device exists in registry.
        
        Connected      = 0x00000020, // device is currently connected to one or more 
                                     // services managed by Bluetooth Engine.
        Connectable    = 0x00000040, // device is connectable to one or more 
                                     // services managed by Bluetooth Engine.
        InRange        = 0x00000100, // device is in range

        // bits re-defined according to Class of Device:
        Computer         = 0x00010000, // a computer
        Phone            = 0x00020000, // a phone
        LANAccessDev     = 0x00040000, // a LAN access point
        AVDev            = 0x00080000, // an A/V device
        Peripheral       = 0x00100000, // a peripheral (input device)
        ImagingDev       = 0x00200000, // an imaging device
        WearableDev      = 0x00400000, // a wearable device
        Toy              = 0x00800000, // a toy
        HealthDev        = 0x01000000, // a health device
        UncategorizedDev = 0x02000000, // a generic device that is uncategorized
        
        // all properties derived from BT registry
        RegistryProperties = Bonded |
            Blocked | RecentlyUsed | Trusted | InRegistry,
        
        // all properties derived from CoD
        CodProperties = Computer | Phone | LANAccessDev |
            AVDev | Peripheral | ImagingDev | WearableDev | 
            Toy | HealthDev  | UncategorizedDev,
    };
    
    /*
     * Minor device filters for major property \code AVDev \endcode
     */
    enum AVDevMinorProperty {
        Carkit   = 0x00000001,
        Headset  = 0x00000002,
    };
    
    /*
     * Minor device filters for major property \code Peripheral \endcode
     */
    enum PeripheralMinorProperty {
        Mouse    = 0x00000001,
        Keyboard = 0x00000002,
    };

public:
    static void mapDeiveType(int &majorProperty, int &minorProperty, int cod );

};


class DevTypeMapping 
{
public:    
    int majorDevClass; // major device class value from CoD
    int minorDevClass; // minor device class value from CoD
    int majorProperty;   // one of major properties defined in BtDeviceModel
    int minorProperty;   // one of minor properties defined in BtDeviceModel
};

// mapping table from major and minor Device Classes to device types
// which are specifically defined in Bluetooth ui namespace.
// (Note audio device mapping is not in this table due to its complex logic)
static const DevTypeMapping DeviceTypeMappingTable[] =
{
{EMajorDeviceComputer,       0, BtuiDevProperty::Computer, 0 },
{EMajorDevicePhone,          0, BtuiDevProperty::Phone,    0 },
{EMajorDeviceLanAccessPoint, 0, BtuiDevProperty::LANAccessDev, 0},
{EMajorDevicePeripheral,     EMinorDevicePeripheralKeyboard, 
                             BtuiDevProperty::Peripheral, 
                             BtuiDevProperty::Keyboard},
{EMajorDevicePeripheral,     EMinorDevicePeripheralPointer, 
                             BtuiDevProperty::Peripheral, 
                             BtuiDevProperty::Mouse},
{EMajorDeviceImaging,        0, BtuiDevProperty::ImagingDev, 0},
{EMajorDeviceWearable,       0, BtuiDevProperty::WearableDev, 0},
{EMajorDeviceToy,            0, BtuiDevProperty::Toy, 0}
};

static const int DeviceTypeMappingTableSize = 
        sizeof( DeviceTypeMappingTable ) / sizeof( DevTypeMapping );

inline void BtuiDevProperty::mapDeiveType(
        int &majorProperty, int &minorProperty, int cod  )
{
    // remove garbage value.
    majorProperty = 0;
    minorProperty = 0;
    TBTDeviceClass codClass = TBTDeviceClass( cod );
    
    // device type must be mapped according to CoD:
    int majorServiceCls = codClass.MajorServiceClass();
    int majorDevCls = codClass.MajorDeviceClass();
    int minorDevCls = codClass.MinorDeviceClass();
    
    int i;
    for (i = 0; i < DeviceTypeMappingTableSize; ++i ) {
        if ( DeviceTypeMappingTable[i].majorDevClass == majorDevCls &&
             ( DeviceTypeMappingTable[i].minorDevClass == 0 || 
               DeviceTypeMappingTable[i].minorDevClass == minorDevCls ) ) {
             // device classes match a item in table, get the mapping:
            majorProperty |= DeviceTypeMappingTable[i].majorProperty;
            minorProperty |= DeviceTypeMappingTable[i].minorProperty;
            break;
         }
    }
    
    // AV device mapping are not defined in the table, 
    // do mapping here if no device type has been mapped so far.
    // This part is not reliably working with all AV devices.
    if ( i == DeviceTypeMappingTableSize) {
        // audio device, carkit, headset or speaker:
        if( ( majorDevCls == EMajorDeviceAV) 
            || (majorServiceCls == EMajorServiceRendering 
            && majorDevCls != EMajorDeviceImaging) ) {
            majorProperty |= BtuiDevProperty::AVDev;
            if( minorDevCls == EMinorDeviceAVCarAudio ) {
                // carkit:
                minorProperty |= BtuiDevProperty::Carkit; 
            }
            else {
                // headset:
                minorProperty |= BtuiDevProperty::Headset;
            }
        }
    }
}
#endif // BTUIDEVTYPEMAP_H
