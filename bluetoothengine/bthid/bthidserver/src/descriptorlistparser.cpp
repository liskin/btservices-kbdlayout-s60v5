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


#include "descriptorlistparser.h"
#include "hiddescriptorlist.h"
#include "hiddescriptor.h"

CDescriptorListParser::CDescriptorListParser(
        CHidDescriptorList& aDescriptorList) :
    iDescriptorList(aDescriptorList)
    {
    }

CDescriptorListParser::~CDescriptorListParser()
    {
    delete iCurrentDescriptor;
    }

void CDescriptorListParser::VisitAttributeValueL(CSdpAttrValue& aValue,
        TSdpElementType aType)
    {
    // We are only interested in the descriptor type and data
    if (iCurrentDescriptor)
        {
        switch (aType)
            {
            case ETypeUint:
                iCurrentDescriptor->SetDescriptorType(
                        static_cast<CHidDescriptor::TDescType> (aValue.Uint()));
                break;

            case ETypeString:
                iCurrentDescriptor->SetRawDataL(aValue.Des());
                break;

            default:
                break;
            }
        }
    }

void CDescriptorListParser::StartListL(CSdpAttrValueList& /*aList*/)
    {
    ++iIndentationLevel;
    // 1 Indentation is the HID Descriptor List DES
    // 2 Indentation is a HID Descriptor DES
    if (iIndentationLevel == 2)
        {
        // Create a HID Descriptor to fill
        if (!iCurrentDescriptor)
            {
            iCurrentDescriptor = CHidDescriptor::NewL();
            }
        }
    }

void CDescriptorListParser::EndListL()
    {
    --iIndentationLevel;
    // 1 Indentation is the HID Descriptor List DES
    if (iIndentationLevel == 1 && iCurrentDescriptor)
        {
        // We have moved back from a HID Descriptor DES
        // Store the populated HID Descriptor in the list
        iDescriptorList.AddDescriptorL(iCurrentDescriptor);
        iCurrentDescriptor = 0;
        }
    }
