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

_LIT8(KAtCLCK, "\r\n+CLCK: ");
_LIT8(KAtCFUN, "+CFUN: ");
_LIT8(KAtCBC, "+CBC: ");
_LIT8(KAtCUSD, "+CUSD: ");
_LIT8(KAtCMGW, "+CMGW: ");
_LIT8(KSCPBR, "\r\n^SCPBR: ");
_LIT8(KCLCKSupportedCmdsList, "\r\n+CLCK: (\"PS\",\"SC\",\"AO\",\"OI\",\"OX\",\"AI\",\"IR\",\"AB\",\"AG\",\"AC\")\r\n\r\nOK\r\n");
_LIT8(KCFUNSupportedCmdsList, "\r\n+CFUN: (0,1,4),(0,1)\r\n\r\nOK\r\n");
_LIT8(KCBCSupportedCmdsList, "\r\n+CBC: (0,1,2,3),(1...100)\r\n\r\nOK\r\n");
_LIT8(KCUSDSupportedCmdsList, "+CUSD: (0,1)\r\n\r\nOK\r\n");
_LIT8(KCMGDSupportedCmdsList, "\r\n+CMGD: (%S), (0,1,2,3,4)\r\n");
_LIT8(KSCPBRSupportedEntriesIndexList,"\r\n^SCPBR:(1-%d),%d,%d,%d \r\n"); 
_LIT8(KSCPBRReplyOneEntry,"%d,\"%S\",,\"%S\",,\"%S\",,\"%S\",,\"%S\",,\"%S\"");
_LIT8(KSCPBWSupportedCmdsList, "\r\n^SCPBW: (1-%d),%d,(),%d,%d\r\n");

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

_LIT8(KHVERModelString, "RM-");

_LIT8(KCMSErr, "+CMS ERROR: ");

// Max buffer length for an MD5 digest - originally defined in SCPServerInterface.h
const TInt KSCPMaxHashLength( 32 );

const TInt KSCPBRDefaultSizeOfOneEntry( 320 );
const TInt KSCPBRMaxNameLength( 64 );
const TInt KSCPBRMaxNumberLength( 32 );
const TInt KSCPBRMaxEmailLength( 64 );
const TInt KSCPBRMaxNumberCount( 4 );
const TInt KSCPBRMaxEntryCount( 1000 );

const TInt KBufBlockSize (1024);
#endif // ATMISCCMDPLUGINCONSTS_H
