/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 * Description :
 *
 */

#ifndef ATMISCCMDPLUGINCONSTS_H
#define ATMISCCMDPLUGINCONSTS_H

#include <e32def.h>

_LIT8(KCRLF, "\r\n");
_LIT8(KOKCRLF, "\r\n\r\nOK\r\n");

_LIT8(KAtCLCK, "+CLCK: ");
_LIT8(KAtCFUN, "+CFUN: ");
_LIT8(KAtCBC, "+CBC: ");
_LIT8(KAtCUSD, "+CUSD: ");

_LIT8(KCLCKSupportedCmdsList, "+CLCK: (\"PS\",\"SC\",\"AO\",\"OI\",\"OX\",\"AI\",\"IR\",\"AB\",\"AG\",\"AC\")\r\n\r\nOK\r\n");
_LIT8(KCFUNSupportedCmdsList, "+CFUN: (0,1,4),(0,1)\r\n\r\nOK\r\n");
_LIT8(KCBCSupportedCmdsList, "+CBC: (0,1,2,3),(1...100)\r\n\r\nOK\r\n");
_LIT8(KCUSDSupportedCmdsList, "+CUSD: (0,1)\r\n\r\nOK\r\n");


_LIT8(KATCLCKPS, "PS");
_LIT8(KATCLCKSC, "SC");
_LIT8(KATCLCKAO, "AO");
_LIT8(KATCLCKOI, "OI");
_LIT8(KATCLCKOX, "OX");
_LIT8(KATCLCKAI, "AI");
_LIT8(KATCLCKIR, "IR");
_LIT8(KATCLCKAB, "AB");
_LIT8(KATCLCKAG, "AG");
_LIT8(KATCLCKAC, "AC");

#endif // ATMISCCMDPLUGINCONSTS_H
