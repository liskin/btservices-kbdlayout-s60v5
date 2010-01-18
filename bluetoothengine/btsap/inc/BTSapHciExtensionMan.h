/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
*     This class holds the HCI Extension Conduit singleton
*
*
*/

#ifndef BT_SERVER_HCIEXTENSIONMAN_H
#define BT_SERVER_HCIEXTENSIONMAN_H

//  INCLUDES
#include <hciproxy.h>

// CLASS DECLARATION
/**
* This class holds the HCI Extension Conduit instance and is the single interface to 
* access HCI enxtension functionality.
*/
NONSHARABLE_CLASS(CBTHciExtensionMan) : public CBase, public MVendorSpecificHciConduit 
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTHciExtensionMan* NewL();

        /**
        * Destructor.
        */
	    ~CBTHciExtensionMan();
	    
    public: // New functions

        /**
        * Hanldes a request throught Hci extension
        * @param aStatus: for signalling the completion of the operation
        * @param aBTDevAddr: the BT device address
        */
        void GetEncryptionKeyLengthL(const TBTDevAddr& aBTDevAddr, TRequestStatus& aStatus);
        
        /**
        * Cancel the outstanding request if there is any
        * @param aMessage the requested service 
        * @return TInt the error code of this operation
        */
        void CancelRequest();
        
        /**
        * After the GetEncryptionKeyLengthL operation is completed
        * the result can be read with the GetResult method
        */
        TInt GetResultL(TUint8& aKeyLength);
        
        
    private: //From MVendorSpecificHciConduit

        /**
        * CommandCompleted is called when a vendor specific command issued through the conduit
        * receives a completion from the hardware.
        * @param aError	:	The error code with which the command completed.
        **/	
        void CommandCompleted(TInt aError);
    	
        /**
        * ReceiveEvent is called when a vendor specific command issued through the conduit
        * receives a completion from the hardware.
        *
        * @param aEvent 	Returns raw vendor specific debug event data generated from the 
        *                   hardware (with no HCTL framing)
        * @param aError 	An error indicating that the conduit did not successfully receive 
        *                   the event.
        * @return           If ETrue, the conduit will continue to listen for vendor specific 
        *                   debug events, 
        *                   If EFalse, the conduit stops listening for these events.
        **/	
        TBool ReceiveEvent(TDesC8& aEvent, TInt aError);

    private:

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * C++ default constructor.
        */
        CBTHciExtensionMan();

    private:  // DATA
        CHciExtensionConduit* iConduit;
        TRequestStatus* iStatus;
        TUint8  iRequestOpcode;
        TUint8 iKeyLength;
    };

// Helper class 
/**
* This class represents a HCI extension command
*/
NONSHARABLE_CLASS(CBTHciExtensionCmd) : CBase
    {
    public: // Constrtuctor and destructor
    
        /**
        * Two-phased constructor.
        */
        static CBTHciExtensionCmd* NewL(TUint8 aOpcode);

        /**
        * Two-phased constructor. Leave it in CleanupStack
        */
        static CBTHciExtensionCmd* NewLC(TUint8 aOpcode);

        /**
        * Destructor.
        */
        ~CBTHciExtensionCmd();
    
    public: // New Functions
        
        /**
        * Gets the opcode of this command
        */
        TUint8 Opcode() const;
        
        /**
        * Gets the command descritpor
        * @return the reference to the command descriptor
        */
        TPtrC8 DesC() const;
        
        /**
        * Sets the parameter in this command
        */
        void SetParamL(const TDesC8& aParam);

    private:
    
        /**
        * C++ default constructor.
        */
        CBTHciExtensionCmd(TUint8 aOpcode);
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
        /**
        * Initialize command descritpor
        */
        void InitializeCmdDesL();
        
    protected: // DATA
        TUint8 iChannelID;
        TUint8 iOpcode;
        HBufC8* iCmdDes;
        TInt iParamTotalLength;
        TInt iCmdLength;
    };

/**
* This class represents the response to a hci extension functionality command
*/
NONSHARABLE_CLASS(CBTHciExtensionCmdEvent) : CBase
    {
    public: // Constrtuctor and destructor
    
        /**
        * Two-phased constructor.
        */
        static CBTHciExtensionCmdEvent* NewL(const TDesC8& aEvent);

        /**
        * Two-phased constructor. Leave it in CleanupStack
        */
        static CBTHciExtensionCmdEvent* NewLC(const TDesC8& aEvent);

        /**
        * Destructor.
        */
        ~CBTHciExtensionCmdEvent();
    
    public: // New Functions
        
        /**
        * Gets the event opcode
        */
        TUint8 Opcode() const;
        
        /**
        * Gets the event descritpor
        * @return the reference to the event descriptor
        */
        TPtrC8 DesC() const;

    private:
    
        /**
        * C++ default constructor.
        */
        CBTHciExtensionCmdEvent();
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(const TDesC8& aEvent);
        
    protected: // DATA
        HBufC8* iEventDes;
    };
#endif // BT_SERVER_HCIEXTENSIONMAN_H
