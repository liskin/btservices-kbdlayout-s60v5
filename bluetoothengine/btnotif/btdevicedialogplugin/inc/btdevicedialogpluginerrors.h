/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Plugin error constants
*
*/

#ifndef BTDEVICEDIALOGPLUGINERRORS_H
#define BTDEVICEDIALOGPLUGINERRORS_H

#include <hbdevicedialog.h>

// No error
const int NoError = 0;
// Illegal parameter error
const int ParameterError = HbDeviceDialog::PluginErrors + 1;
// Unknown device dialog error
const int UnknownDeviceDialogError = HbDeviceDialog::PluginErrors + 2;

#endif // BTDEVICEDIALOGPLUGINERRORS_H
