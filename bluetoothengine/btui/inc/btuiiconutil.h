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

#ifndef BTUIICONUTIL_H
#define BTUIICONUTIL_H

#include <qglobal.h>
#include <bt_sock.h>
#include <HbIcon>
//#include "btdevicemodel.h"
#include "btuidevtypemap.h"

enum BtuiIconCorner {
    BtuiNoCorners   = 0x00,   // No badge needed
    BtuiBottomLeft  = 0x01,   // Badge for : Paired, Paired & Authorized, Blocked
    BtuiBottomRight = 0x02,   // Badge for : Connected
    BtuiTopLeft     = 0x04,   // Badge for : Signal strength
    BtuiTopRight    = 0x08    // Badge for : Battery low
};

/*!
  Creates a HbIcon according to the type of the CBTDevice dev,
  Creates badges in the icon's 4 corners according to the parameters 
  passed to the function
  bottom left corner: Paired, Paired & Authorized, Blocked
  bottom right corner: Connected
  top right corner: Battery low
  top left corner: signal strength
*/

inline void setBadging(HbIcon &deviceIcon, int majorProperty, int corners)
{
    if (corners & BtuiBottomLeft) {
         if (majorProperty & BtuiDevProperty::Trusted ) {
             HbIcon badge("qtg_small_authorised");
             badge.setIconName("qtg_small_authorised");
             deviceIcon.addBadge(Qt::AlignLeft|Qt::AlignBottom, badge);
         }
         else if (majorProperty & BtuiDevProperty::Bonded) {
             HbIcon badge("qtg_small_pair");
             badge.setIconName("qtg_small_pair");
             deviceIcon.addBadge(Qt::AlignLeft|Qt::AlignBottom, badge);
         }
         else if (majorProperty & BtuiDevProperty::Blocked) {
             HbIcon badge("qtg_small_blocked");
             badge.setIconName("qtg_small_blocked");
             deviceIcon.addBadge(Qt::AlignLeft|Qt::AlignBottom, badge);
         }
     }
     if (corners & BtuiBottomRight) {
         if (majorProperty & BtuiDevProperty::Connected) {
             HbIcon badge("qtg_small_connection");
             badge.setIconName("qtg_small_connection");
             deviceIcon.addBadge(Qt::AlignRight|Qt::AlignBottom, badge);
         }
     }
     if (corners & BtuiTopRight) {
     //ToDo: when power level added to model
     }
     if (corners & BtuiTopLeft) {
     //ToDo: add signal strength.    
     }
}


/*!
 * builds device icon based on device type, including badging with connection info, etc.
 * 
 * \param cod the integer value of the CoD.
 * \param majorProperty the major property consisting of BtuiDevProperty::DevMajorProperty bits
 * \param corners specifies which corners of primary icon 
 *          should be badged with connectivity, pairing, trust, etc information
 * \return an HbIcon
 *       
 */
inline HbIcon getBadgedDeviceTypeIcon(int cod, int majorProperty = 0, int corners = BtuiNoCorners)
{   
    int uiMajorDevice;
    int uiMinorDevice;
    // device type is mapped according to CoD:
    BtuiDevProperty::mapDeiveType(uiMajorDevice, uiMinorDevice, cod);
    
    // Set icon according to the device category
    QString name;
    if (uiMajorDevice & BtuiDevProperty::Phone) {
        name = QString("qtg_large_mobile");
    }
    else if (uiMajorDevice & BtuiDevProperty::Computer) {
        name = QString("qtg_large_computer");
    }
    else if (uiMajorDevice & BtuiDevProperty::AVDev) {
        name = QString("qtg_large_generic_audio");
    }
    else if (uiMajorDevice & BtuiDevProperty::Peripheral) {
        // todo: map to keyboard & mouse from uiMinorDevice
        name = QString("qtg_large_input_device");
    }
    else {
        name = QString("qtg_large_generic_bluetooth");
    }
    HbIcon icon(name);
    if (corners > BtuiNoCorners) {
        setBadging(icon, majorProperty, corners);
    }
    return icon;
}

/*
 * returns device type as a translated string based on  
 * Class of Device (CoD) parameter
 */
inline QString getDeviceTypeString( int cod )
{   
    int uiMajorDevice;
    int uiMinorDevice;
    // device type is mapped according to CoD:
    BtuiDevProperty::mapDeiveType(uiMajorDevice, uiMinorDevice, cod);
    
    // Set icon according to the device category
    if (uiMajorDevice & BtuiDevProperty::Phone) {
        return hbTrId("txt_bt_list_phone");
    }
    else if (uiMajorDevice & BtuiDevProperty::Computer) {
        return hbTrId("txt_bt_list_computer");
    }
    else if (uiMajorDevice & BtuiDevProperty::AVDev) {  
        return hbTrId("txt_bt_list_audio_device");
    }
    else if (uiMajorDevice & BtuiDevProperty::Peripheral) {
        // todo: map to keyboard & mouse
        return hbTrId("txt_bt_list_input_device");
    }
    else {
        return hbTrId("txt_bt_list_other");
    }
}

#endif // BTUIMODELUTIL_H
