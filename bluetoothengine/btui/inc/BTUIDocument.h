/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


#ifndef BTUIDOCUMENT_H
#define BTUIDOCUMENT_H

// INCLUDES

#include <AknDoc.h>
#include <aknapp.h>

// FORWARD DECLARATIONS

class CEikAppUi;
//class CBTUIModel;

// CLASS DECLARATION

/**
*  This class is a base class mandatory for all Symbian OS UI applications.
*/
class CBTUIDocument : public CAknDocument
    {
    public: // Constructors and destructor

		/**
        * Two-phased constructor.
        */
        static CBTUIDocument* NewL(CAknApplication& aApp);

		/**
        * Destructor.
        */
        virtual ~CBTUIDocument();

	private: // Functions from base classes

		/**
        * From CAknDocument Creates AppUi class.        
        * @param None.
        * @return A reference to created class.
        */
		CEikAppUi*  CreateAppUiL();

    private:

		/**
        * C++ default constructor.
        */
        CBTUIDocument(CEikApplication& aApp) :CAknDocument(aApp) { }

		/**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
    };

#endif

