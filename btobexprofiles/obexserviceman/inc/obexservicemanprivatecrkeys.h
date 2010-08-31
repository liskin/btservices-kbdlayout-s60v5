/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Obex Service Manager private central repository key definitions.
*
*/


#ifndef OBEXSERVICEMAN_PRIVATE_CR_KEYS_H
#define OBEXSERVICEMAN_PRIVATE_CR_KEYS_H


/**  Obex Service Manager configuration CenRep UID */
const TUid KCRUidObexServiceMan = { 0x20016BC5 };


/**
 * CenRep key for storing Obex USB DMA usage settings.
 *
 * Possible integer values:
 * 0 USB DMA is not in use
 * 1 USB DMA is in use
 *
 * Default value: 1
 *
 */
const TUint32 KObexUsbDmaUsage =	0x00000001;

/**  Enumeration for obex usb dma usage*/
enum TObexUsbDmaUsage
    {
    EObexUsbDmaNotInUse = 0,
    EObexUsbDmaInUse = 1
    };

#endif // OBEXSERVICEMAN_PRIVATE_CR_KEYS_H
