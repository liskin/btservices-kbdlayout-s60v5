/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     Message type module UI data part.
*
*
*/


#ifndef CIRMTMUIDATA_H
#define CIRMTMUIDATA_H

//  INCLUDES
#include <mtudcbas.h>

// FORWARD DECLARATIONS


// CLASS DECLARATION
/**
*  CIrMtmUiData
*  
*/
class CIrMtmUiData : public CBaseMtmUiData
	{
    public: // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CIrMtmUiData* NewL(CRegisteredMtmDll& aRegisteredDll);
	
        /**
        * Destructor.
        */
        virtual ~CIrMtmUiData();

        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();

    public: // Functions from base classes

        /**
        * From CBaseMtmUiData function query.
        * @param aOperationId Id for function.
        * @param aContext messaging store entry.
        * @return TInt error code.
        */
	    virtual TInt OperationSupportedL(
            TInt aOperationId, 
            const TMsvEntry& aContext) const;

        /**
        * From CBaseMtmUiData MTM capability check.
        * @param aCapability Uid for message type.
        * @param aResponse response for capability check.
        * @return TInt error code.
        */
	    virtual TInt QueryCapability(
            TUid aCapability, 
            TInt& aResponse) const;

        /**
        * From CBaseMtmUiData MTM related context icons.
        * @param aContext Messaging entry refence.
        * @param aStateFlags Flags for icon.
        * @return TInt error code.
        */
	    virtual const CBitmapArray& ContextIcon(
            const TMsvEntry& aContext, 
            TInt aStateFlags) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aParent Messaging entry refence.
        * @param aNewEntry Messaging entry refence(created entry).
        * @param aReasonResourceId ResourceID.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanCreateEntryL(
            const TMsvEntry& aParent, 
            TMsvEntry& aNewEntry, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanReplyToEntryL(
            const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanForwardEntryL(
            const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanEditEntryL(
            const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanViewEntryL(const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanOpenEntryL(
            const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanCloseEntryL(
            const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanDeleteFromEntryL(
            const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanDeleteServiceL(
            const TMsvEntry& aService, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanCopyMoveToEntryL(
            const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
	    virtual TBool CanCopyMoveFromEntryL(
            const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @param aReasonResourceId Resource id.
        * @return TBool inform that mtm can provice function or not.
        */
        virtual TBool CanCancelL(const TMsvEntry& aContext, 
            TInt& aReasonResourceId) const;

        /**
        * From CBaseMtmUiData Provide context-specific function information.
        * @param aContext Messaging entry refence.
        * @return HBufC* Statustext for sending.
        */
        virtual HBufC* StatusTextL(const TMsvEntry& aContext) const;

    protected:  // Functions from base classes

        /**
        * From CBaseMtmUiData Populate icon array
        */
	    virtual void PopulateArraysL();

        /**
        * From CBaseMtmUiData gets resource filename.
        */
	    virtual void GetResourceFileName(TFileName& aFileName) const;

    protected: 

        /**
        * C++ default constructor.
        */
        CIrMtmUiData(CRegisteredMtmDll& aRegisteredDll);

    protected: // New functions

        /**
        * Checks is the entry valid.
        * @param aContext reference to the entry.
        * @return TBool 
        */
	    TBool CheckEntry(const TMsvEntry& aContext) const;

	};

#endif // CIRMTMUIDATA_H
// End of File
