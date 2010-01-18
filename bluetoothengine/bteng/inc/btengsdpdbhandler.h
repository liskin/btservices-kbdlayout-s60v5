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
* Description:  Helper class for SDP database management.
*
*/


#ifndef BTENGSDPDBHANDLER_H
#define BTENGSDPDBHANDLER_H

#include <btsdp.h>

class TResourceReader;


/**
 *  ?one_line_short_description
 *
 *  ?more_complete_description
 *
 *  @lib btengdiscovery.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CBTEngSdpDbHandler ) : public CBase
    {

public:

    /**
     * Two-phase constructor
     *
     * @since S60 v3.2
     * @return Pointer to the constructed CBTEngSdpDbHandler object.
     */
    static CBTEngSdpDbHandler* NewL();

    /**
     * Destructor
     */
    virtual ~CBTEngSdpDbHandler();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aService ?description
     * @param aChannel RFCOMM channel to be filled in into the record.
     * @param aHandle ?description
     */
    void RegisterSdpRecordL( const TUUID& aService, TInt aChannel, 
                              TSdpServRecordHandle& aHandle );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aService ?description
     * @param aVendorId Vendor ID to be filled in into the DI record.
     * @param aProductId Product ID to be filled in into the DI record.
     * @param aHandle ?description
     */
    void RegisterSdpRecordL( const TUUID& aService, TInt aVendorId, 
                              TInt aProductId, TSdpServRecordHandle& aHandle );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aService ?description
     * @param aHandle ?description
     */
    void RegisterSdpRecordL( const TUUID& aService, 
                              TSdpServRecordHandle& aHandle );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aHandle Handle to the SDP record to be deleted. Note that 
     *        this has to be a valid (existing) SDP record.
     */
    void DeleteSdpRecordL( const TSdpServRecordHandle aHandle );

private:

    /**
     * C++ default constructor
     *
     * @since S60 v3.2
     */
    CBTEngSdpDbHandler();

    /**
     * Symbian 2nd-phase constructor
     *
     * @since S60 v3.2
     */
    void ConstructL();

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aService ?description
     * @param aChannel ?description
     * @return ?description
     */
    void BuildAttributeLC( CSdpAttrValue*& aAttrVal, TResourceReader& aReader, 
                            TInt aAttrId );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aService ?description
     * @param aChannel ?description
     * @return ?description
     */
    void BuildAttrValueLC( CSdpAttrValue*& aAttrVal, TResourceReader& aReader, 
                            TUint aAttrType, TInt aAttrId );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aService ?description
     * @param aChannel ?description
     * @return ?description
     */
    void BuildAttrDesLC( CSdpAttrValue*& aAttrVal, TResourceReader& aReader, 
                          TInt aAttrId );

    /**
     * ?description
     *
     * @since S60 v3.2
     * @param aService ?description
     * @param aChannel ?description
     * @return ?description
     */
    void ReadRecordResourceL( const TDesC8& aService, TResourceReader& aReader, 
                               HBufC8*& aRecordBuf );

private: // data

    /**
     * RFComm channel number.
     * (to be replaced with a more flexible structure 
     * so that e.g. DI profile values can be set too).
     */
    TUint iChannel;

    /**
     * DI profile vendor ID.
     */
    TUint iVendorId;

    /**
     * DI profile product ID.
     */
    TUint iProductId;

    /**
     * Session to SDP database.
     */
    RSdp iSdp;

    /**
     * Subsession to SDP database for managing SDP records.
     */
    RSdpDatabase iDb;

    };

#endif // BTENGSDPDBHANDLER
