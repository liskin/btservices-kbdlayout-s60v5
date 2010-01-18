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


#ifndef __HIDDESCRIPTOR_H__
#define __HIDDESCRIPTOR_H__

#include <e32base.h>
#include <s32strm.h>

/*!
 This class represents an individual BT HID Descriptor
 */
class CHidDescriptor : public CBase
    {
public:
    /*! The type of this HID descriptor */
    enum TDescType
        {
        EReportDescriptor = 0x22, /*!< Report Descriptor */
        EPhysicalDescriptor = 0x23,
        /*!< Physical Descriptor */
        };
public:

    /*!
     Create a CHidDescriptor object
     @result a pointer to the created instance of CHidDescriptor
     */
    static CHidDescriptor* NewL();

    /*!
     Create a CHidDescriptor object
     (object is left on the cleanup stack)
     @result a pointer to the created instance of CHidDescriptor
     */
    static CHidDescriptor* NewLC();

    /*!
     Create a CHidDescriptor object from a stream.
     @param aStream a stream to create the object from
     @result a pointer to the created instance of CHidDescriptor
     */
    static CHidDescriptor* NewL(RReadStream& aStream);

    /*!
     Create a CHidDescriptor object from a stream.
     (object is left on the cleanup stack)
     @param aStream a stream to create the object from
     @result a pointer to the created instance of CHidDescriptor
     */
    static CHidDescriptor* NewLC(RReadStream& aStream);

    /*!
     Destroy the object and release all memory objects
     */
    ~CHidDescriptor();

    /*!
     Sets the type of this HID descriptor
     @param aType The type of this descriptor.
     */
    inline void SetDescriptorType(TDescType aType);

    /*!
     Sets the contents of the member buffer
     @param aBuf new buffer to copy.
     */
    inline void SetRawDataL(const TDesC8& aBuf);

    /*!
     Gets the type of this HID descriptor
     @return The type of this descriptor.
     */
    inline TDescType DescriptorType() const;

    /*!
     Gets the contents of the member buffer
     @return A TDesC8 reference to the member data buffer
     */
    inline const TDesC8& RawData() const;

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

private:

    /*!
     Constructs this object
     */
    CHidDescriptor();

    /*!
     Performs second phase construction of this object
     */
    void ConstructL();

    /*!
     Performs second phase construction of this object from a filestore
     @param aStream a stream to create the object from
     */
    void ConstructL(RReadStream& aStream);

private:
    // Member variables
    /*! The type of this descriptor */
    TDescType iType;

    /*! The raw descriptor */
    HBufC8* iRawData;
    };

inline void CHidDescriptor::SetDescriptorType(TDescType aType)
    {
    iType = aType;
    }

inline void CHidDescriptor::SetRawDataL(const TDesC8& aText)
    {
    // Copy the buffer
    delete iRawData;
    iRawData = 0;
    iRawData = aText.AllocL();
    }

inline CHidDescriptor::TDescType CHidDescriptor::DescriptorType() const
    {
    return iType;
    }

inline const TDesC8& CHidDescriptor::RawData() const
    {
    if (!iRawData)
        {
        return KNullDesC8;
        }
    else
        {
        return *iRawData;
        }
    }

#endif // __HIDDESCRIPTOR_H__
