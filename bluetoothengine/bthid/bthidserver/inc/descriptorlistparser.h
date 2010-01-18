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


#ifndef __CDESCRIPTORLISTPARSER_H__
#define __CDESCRIPTORLISTPARSER_H__

#include <e32base.h>
#include <btsdp.h>

class CHidDescriptor;
class CHidDescriptorList;

/*!
 This class parses a HID Descriptor List DES, extracting its HID Descriptors. 
 BT HID Spec. 1.0 Sec. 7.11.6
 */
class CDescriptorListParser : public CBase, public MSdpAttributeValueVisitor
    {
public:

    /*!
     Constructs this object
     @param aDescriptorList the descriptor list to populate
     */
    CDescriptorListParser(CHidDescriptorList& aDescriptorList);

    /*!
     Destroy the object and release all memory objects
     */
    ~CDescriptorListParser();

public:
    // from MSdpAttributeValueVisitor
    /*!
     Handle attribute value.
     @param aValue Attribute value
     @param aType Attribute type
     */
    void VisitAttributeValueL(CSdpAttrValue& aValue, TSdpElementType aType);

    /*!
     Indicates that a new Data Element Sequence (DES) has started.
     @param aList Attribute value list
     */
    void StartListL(CSdpAttrValueList& aList);

    /*!
     Indicates that a Data Element Sequence (DES) has ended.
     */
    void EndListL();

private:
    // Member variables

    /*! The current indentation when parsing Data Element Sequences */
    TInt iIndentationLevel;

    /*! The descriptor list to populate */
    CHidDescriptorList& iDescriptorList;

    /*! The descriptor currently being created */
    CHidDescriptor* iCurrentDescriptor;
    };

#endif // __CDESCRIPTORLISTPARSER_H__
