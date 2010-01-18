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
* Description:
*
*/


#ifndef PaintCursorDOCUMENT_H
#define PaintCursorDOCUMENT_H

#include <AknDoc.h>

class CEikAppUi;

/**
 *  CPaintCursorDocument application class.
 */
class CPaintCursorDocument : public CAknDocument
    {
public:
    // Constructors and destructor
    /**
     * Two-phased constructor.
     */
    static CPaintCursorDocument* NewL(CEikApplication& aApp);

    /**
     * Destructor.
     */
    virtual ~CPaintCursorDocument();

private:

    /**
     * EPOC default constructor.
     */
    CPaintCursorDocument(CEikApplication& aApp);
    void ConstructL();

private:

    /**
     * From CEikDocument, create CPaintCursorAppUi "App UI" object.
     */
    CEikAppUi* CreateAppUiL();
    };

#endif

