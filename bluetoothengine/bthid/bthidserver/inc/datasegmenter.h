/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares main application class.
 *
*/


#ifndef __CMDSEGMENTER_H__
#define __CMDSEGMENTER_H__

#include <e32base.h>

/*!
 This utility class segments large commands into MTU size packets
 */
class CDataSegmenter : public CBase
    {
public:

    /*!
     Create a CDataSegmenter object
     @result a pointer to the created instance of CDataSegmenter
     */
    static CDataSegmenter* NewL();

    /*!
     Create a CDataSegmenter object
     @result a pointer to the created instance of CDataSegmenter
     */
    static CDataSegmenter* NewLC();

    /*!
     Destroy the object and release all memory objects
     */
    ~CDataSegmenter();

    /*!
     Prepares buffers for sending a large command, possibly in separate packets.
     @param aInitialHeader the header to use in the first packet.
     @param aPayload the payload section of the packet.
     @param aAdditionalHeader the header byte to use on continuation packets.
     @param aMTU the current MTU for a packet.
     */
    void SegmentDataL(const TDesC8& aInitialHeader, const TDesC8& aPayload,
            TChar aAdditionalHeaderByte, TInt aMTU);

    /*!
     Retrieves the first packet in a sequence.
     Function will leave if there is no data packet to return.
     @result a pointer to the data packet
     */
    const HBufC8* FirstPacketL() const;

    /*!
     Retrieves the next packet in a sequence.
     @result a pointer to the data packet
     @result 0 if there are no more packets
     */
    const HBufC8* NextPacket();

    /*!
     Resets the state of the object
     */
    void Reset();

private:

    /*!
     Constructs this object
     */
    CDataSegmenter();

    /*!
     Performs second phase construction of this object
     */
    void ConstructL();

private:
    /*! Dynamic buffer to hold a command packet */
    HBufC8* iPacket;

    /*! Dynamic buffer used to hold payload not sent in first packet */
    HBufC8* iPayloadBuf;

    /*! Pointer to the current remaining payload */
    TPtrC8 iPayloadBufPtr;

    /*! The current maximum size of a packet */
    TInt iMTU;

    /*! The header byte to use on continuation packets */
    TChar iAdditionalHeaderByte;
    };

#endif // __CMDSEGMENTER_H__
