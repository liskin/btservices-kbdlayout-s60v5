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


#ifndef __HIDDESCRIPTORLIST_H__
#define __HIDDESCRIPTORLIST_H__

#include <e32base.h>
#include <s32strm.h>

class CHidDescriptor;

/*!
 This class contains a set of BT HID Descriptors
 */
class CHidDescriptorList : public CBase
    {
public:

    /*!
     Constructs this object
     */
    CHidDescriptorList();

    /*!
     Destroy the object and release all memory objects
     */
    ~CHidDescriptorList();

    /*!
     Get a count of the number of HID descriptors
     @result The number of descriptors.
     */
    TInt DescriptorCount() const;

    /*!
     Get access to an individual HID descriptor by index.
     @param aIndex the index of the HID descriptor
     @result A HID descriptor object
     */
    const CHidDescriptor& operator[](TInt aIndex) const;

    /*!
     Add a new descriptor to the list
     @param aDesc a HID descriptor object
     */
    void AddDescriptorL(const CHidDescriptor* aDesc);

    /*!
     Determine the number of bytes this object will take up when
     externalised to disk
     @return The size required to externalise.
     */
    TInt DiskSize() const;

    /*!
     Externalize data to a stream
     @param aStream stream object 
     */
    void ExternalizeL(RWriteStream& aStream) const;

    /*!
     Internalize data from a stream
     @param aStream stream object 
     */
    void InternalizeL(RReadStream& aStream);

private:
    // Member variables
    /*! A list of descriptors */
    CArrayPtrFlat<const CHidDescriptor>* iDescriptorList;
    };

#endif // __HIDDESCRIPTORLIST_H__
