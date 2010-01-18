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
* Description:  Helper class for parsing SDP attribute query results.
*
*/



//#include <?include_file>

#include "btengsdpattrparser.h"

//const ?type ?constant_var = ?constant;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSdpAttrParser::CBTEngSdpAttrParser( RSdpResultArray* aResultArray )
    : iResultArray( aResultArray )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSdpAttrParser::ConstructL()
    {
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSdpAttrParser* CBTEngSdpAttrParser::NewL( RSdpResultArray* aResultArray )
    {
    CBTEngSdpAttrParser* self = new( ELeave ) CBTEngSdpAttrParser( aResultArray );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSdpAttrParser::~CBTEngSdpAttrParser()
    {
    }


// ---------------------------------------------------------------------------
// Set the current attribute ID, to add to the result array.
// ---------------------------------------------------------------------------
//
void CBTEngSdpAttrParser::SetAttributeID( TSdpAttributeID aAttrId )
    {
    iAttrId = aAttrId;
    }


// ---------------------------------------------------------------------------
// From class MSdpAttributeValueVisitor.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpAttrParser::VisitAttributeValueL( CSdpAttrValue& aValue, 
    TSdpElementType aType )
    {
    TBTEngSdpAttrValue attrValue;
    attrValue.iAttrId = iAttrId;
    attrValue.iAttrType = aType;
    switch( aType )
        {
        case ETypeNil:
            {
            attrValue.iAttrValue.iValNumeric = 0;   // To be checked
            }
            break;
        case ETypeUint:
            {
            attrValue.iAttrValue.iValNumeric = aValue.Uint();
            }
            break;
        case ETypeInt:
            {
            attrValue.iAttrValue.iValNumeric = aValue.Int();
            }
            break;
        case ETypeUUID:
            {
            attrValue.iAttrValue.iValString.Set( aValue.UUID().Des() );
            }
            break;
        case ETypeString:
            {
            attrValue.iAttrValue.iValString.Set( aValue.Des() );
            }
            break;
        case ETypeBoolean:
            {
            attrValue.iAttrValue.iValNumeric = aValue.Bool();
            }
            break;
        case ETypeURL:
            {
            attrValue.iAttrValue.iValString.Set( aValue.Des() );
            }
            break;
        case ETypeDES:
        case ETypeDEA:
        case ETypeEncoded:
        default:
                // Don't add any value
            break;
        }
    iResultArray->Append( attrValue );
    }


// ---------------------------------------------------------------------------
// From class MSdpAttributeValueVisitor.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpAttrParser::StartListL( CSdpAttrValueList& aList )
    {
        // Increase counter to check the nested lists
    iNestingLevel++;
    (void) aList;
    }


// ---------------------------------------------------------------------------
// From class MSdpAttributeValueVisitor.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSdpAttrParser::EndListL()
    {
        // Decrease counter to check the nested lists and signal completion.
    iNestingLevel--;
    if( iNestingLevel == 0 )
        {
        ;   // Parsing completed
        }
    }
