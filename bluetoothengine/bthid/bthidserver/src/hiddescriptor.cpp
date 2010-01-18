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
* Description:  This is the implementation of application class
 *
*/


#include "hiddescriptor.h"

CHidDescriptor* CHidDescriptor::NewL()
    {
    CHidDescriptor* self = NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CHidDescriptor* CHidDescriptor::NewLC()
    {
    CHidDescriptor* self = new (ELeave) CHidDescriptor();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CHidDescriptor* CHidDescriptor::NewL(RReadStream& aStream)
    {
    CHidDescriptor* self = NewLC(aStream);
    CleanupStack::Pop(self);
    return self;
    }

CHidDescriptor* CHidDescriptor::NewLC(RReadStream& aStream)
    {
    CHidDescriptor* self = new (ELeave) CHidDescriptor();
    CleanupStack::PushL(self);
    self->ConstructL(aStream);
    return self;
    }

CHidDescriptor::CHidDescriptor()
    {
    }

CHidDescriptor::~CHidDescriptor()
    {
    delete iRawData;
    }

void CHidDescriptor::ConstructL()
    {
    }

void CHidDescriptor::ConstructL(RReadStream& aStream)
    {
    // First read in the type
    iType = static_cast<CHidDescriptor::TDescType> (aStream.ReadInt32L());

    // The TInt size was written out using WriteInt32L
    // Read in the buffer size
    TInt size = aStream.ReadInt32L();

    if (size > 0)
        {
        // Read in any data
        iRawData = HBufC8::NewL(aStream, size);
        }
    }

TInt CHidDescriptor::DiskSize() const
    {
    // Always using 10 bytes as an estimate for Series 60 Descriptor headers

    // We write out the type which is an int
    TInt size = 4;

    // If there is raw data we will write out the descriptor
    if (iRawData && iRawData->Length() > 0)
        {
        // An estimate of the header requirements.
        size += 10;
        size += iRawData->Length();
        }

    return size;
    }

void CHidDescriptor::ExternalizeL(RWriteStream& aStream) const
    {
    // NOTE!! When changing this function, also check DiskSize

    // First write out the type
    aStream.WriteInt32L(iType);

    if (iRawData && iRawData->Length() > 0)
        {
        // Write out the buffer length
        aStream.WriteInt32L(iRawData->Length());
        // Write out the buffer
        aStream << *iRawData;
        }
    else
        {
        // No buffer so just put 0
        aStream.WriteInt32L(0);
        }
    }
