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


#include "hiddescriptorlist.h"
#include "hiddescriptor.h"
#include "bthidserver.pan"

const TInt KArrayGranularity = 2;

CHidDescriptorList::CHidDescriptorList()
    {
    }

CHidDescriptorList::~CHidDescriptorList()
    {
    if (iDescriptorList)
        {
        // Clean up.
        iDescriptorList->ResetAndDestroy();
        delete iDescriptorList;
        }
    }

TInt CHidDescriptorList::DescriptorCount() const
    {
    if (iDescriptorList)
        {
        return iDescriptorList->Count();
        }
    else
        {
        return 0;
        }
    }

const CHidDescriptor& CHidDescriptorList::operator[](TInt aIndex) const
    {
    __ASSERT_ALWAYS(iDescriptorList &&
            0 <= aIndex && aIndex < iDescriptorList->Count(),
            User::Panic(KBTHIDServer, EBadRequest));

    return (*iDescriptorList->At(aIndex));
    }

void CHidDescriptorList::AddDescriptorL(const CHidDescriptor* aDesc)
    {
    if (iDescriptorList)
        {
        iDescriptorList->AppendL(aDesc);
        }
    else
        {
        iDescriptorList = new (ELeave) CArrayPtrFlat<const CHidDescriptor> (
                KArrayGranularity);
        iDescriptorList->AppendL(aDesc);
        }
    }

TInt CHidDescriptorList::DiskSize() const
    {
    // We always write out the number of descriptors which is an int.
    TInt size = 4;

    // Add on the size of all descriptors
    if (iDescriptorList)
        {
        TInt count = iDescriptorList->Count();

        for (TInt i = 0; i < count; i++)
            {
            const CHidDescriptor* tmp = ((*iDescriptorList)[i]);
            size += tmp->DiskSize();
            }
        }

    return size;
    }

void CHidDescriptorList::ExternalizeL(RWriteStream& aStream) const
    {
    // NOTE!! When changing this function, also check DiskSize

    if (iDescriptorList)
        {
        TInt count = iDescriptorList->Count();

        aStream.WriteInt32L(count);
        for (TInt i = 0; i < count; i++)
            {
            const CHidDescriptor* tmp = ((*iDescriptorList)[i]);
            aStream << *tmp;
            }
        }
    else
        {
        aStream.WriteInt32L(0);
        }
    }

void CHidDescriptorList::InternalizeL(RReadStream& aStream)
    {
    // Read in the number of descriptors
    TInt count = aStream.ReadInt32L();

    if (count > 0)
        {
        // Create the array to hold the CHidDescriptor.
        iDescriptorList = new (ELeave) CArrayPtrFlat<const CHidDescriptor> (
                KArrayGranularity);

        // Load each descriptor in and add it to the list
        for (TInt i = 0; i < count; i++)
            {
            CHidDescriptor *temp = CHidDescriptor::NewLC(aStream);
            iDescriptorList->AppendL(temp);
            CleanupStack::Pop(); // temp
            }
        }
    }

