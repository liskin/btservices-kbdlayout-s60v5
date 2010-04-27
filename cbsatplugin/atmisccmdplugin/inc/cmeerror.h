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
 * Description : define all +CME Error code.
 * Reference:
 *    3GPP TS 27.007 version 6.9.0 Release 6 118 ETSI TS 127 007 V6.9.0 (2007-06)
 *
 */

#ifndef CMEERROR_H_
#define CMEERROR_H_

enum TATCMEError
    {
    EATCmeNoError       = -1,   // No errors, only for internal use
    EATCmePhoneFailure  =0,     // 0 phone failure
    EATCmeNoConnection,         // 1 no connection to phone
    EATCmeAdaptorLinkReserved,  // 2 phone-adaptor link reserved
    EATCmeNotAllowed,           // 3 operation not allowed
    EATCmeNotSupported,         // 4 operation not supported
    EATCmePhSimPinRequired,     // 5 PH-SIM PIN required
    EATCmePhFsimPinRequired,    // 6 PH-FSIM PIN required
    EATCmePhFsimPukRequired,    // 7 PH-FSIM PUK required
    EATCmeSimNotInserted,       // 10 SIM not inserted
    EATCmeSimPinRequired,       // 11 SIM PIN required
    EATCmeSimPukRequired,       // 12 SIM PUK required
    EATCmeSimFailure,           // 13 SIM failure
    EATCmeSimBusy,              // 14 SIM busy
    EATCmeSimWrong,             // 15 SIM wrong
    EATCmeIncorrectPassword,    // 16 incorrect password
    EATCmeSimPin2Required,      // 17 SIM PIN2 required
    EATCmeSimPuk2Required,      // 18 SIM PUK2 required
    EATCmeMemoryFull,           // 20 memory full
    EATCmeInvalidIndex,         // 21 invalid index
    EATCmeNotFound,             // 22 not found
    EATCmeMemoryFailure,        // 23 memory failure
    EATCmeTextTooLong,          // 24 text string too long
    EATCmeInvalidChar,          // 25 invalid characters in text string
    EATCmeDialStringTooLong,    // 26 dial string too long
    EATCmeInvalidDialString,    // 27 invalid characters in dial string
    EATCmeNoNetworkService,     // 30 no network service
    EATCmeNetworkTimeout,       // 31 network timeout
    EATCmeNetworkNotAllowed,    // 32 network not allowed - emergency calls only
    EATCmeNetPerPinRequired,    // 40 network personalization PIN required
    EATCmeNetPerPukRequired,    // 41 network personalization PUK required
    EATCmeNetSubPerPinRequired, // 42 network subset personalization PIN required
    EATCmeNetSubPerPukRequired, // 43 network subset personalization PUK required
    EATCmeSPPerPinRequired,     // 44 service provider personalization PIN required
    EATCmeSPPerPukRequired,     // 45 service provider personalization PUK required
    EATCmeCorpPerPinRequired,   // 46 corporate personalization PIN required
    EATCmeCorpPerPukRequired,   // 47 corporate personalization PUK required
    EATCmeHiddenKeyRequired,    // 48 hidden key required (NOTE: This key is required when accessing hidden phonebook entries.)
    EATCmeEAPNotSupported,      // 49 EAP method not supported
    EATCmeIncorrectParams,      // 50 Incorrect parameters
    EATCmeUnknown = 100         // 100 unknown
    };

#endif /* CMEERROR_H_ */
