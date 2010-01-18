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



#ifndef BTENGSDPATTRPARSER_H
#define BTENGSDPATTRPARSER_H


#include <btsdp.h>

#include "btengdiscovery.h"


/**
 *  Class CBTEngSdpAttrParser
 *
 *  ?more_complete_description
 *
 *  @lib ?library
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngSdpAttrParser ) : public CBase, 
                                           public MSdpAttributeValueVisitor
    {

public:

    /**
     * Two-phase constructor
     */
    static CBTEngSdpAttrParser* NewL( RSdpResultArray* aResultArray );

    /**
     * Destructor
     */
    virtual ~CBTEngSdpAttrParser();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     * @param ?arg2 ?description
     * @return ?description
     */
    void SetAttributeID( TSdpAttributeID aAttrId );

// from base class MSdpAttributeValueVisitor

    /**
     * From MSdpAttributeValueVisitor.
     * Called to pass an attribute value.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void VisitAttributeValueL( CSdpAttrValue &aValue, TSdpElementType aType );

    /**
     * From MSdpAttributeValueVisitor.
     * Called to indicate the start of a list of attribute values. 
     * This call is followed by a call to VisitAttributeValueL() 
     * for each attribute value in the list, and concluded by 
     * a call to EndList().
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void StartListL( CSdpAttrValueList &aList );

    /**
     * From MSdpAttributeValueVisitor.
     * Called to indicate the end of a list of attribute values.
     *
     * @since S60 v3.2
     * @param ?arg1 ?description
     */
    virtual void EndListL();

private:

    /**
     * C++ default constructor
     */
    CBTEngSdpAttrParser( RSdpResultArray* aResultArray );

    /**
     * Symbian 2nd-phase constructor
     */
    void ConstructL();

private: // data

    /**
     * ?description_of_member
     */
    TInt iNestingLevel;

    /**
     * ?description_of_member
     */
    TSdpAttributeID iAttrId;

    /**
     * Reference to the array for storing the parsed result.
     * Not own.
     */
    RSdpResultArray* iResultArray;

    };


#endif // BTENGSDPATTRPARSER_H
