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


#ifndef BTUIAPP_H
#define BTUIAPP_H

#include <aknapp.h>

/**
* CBTUIApp application class
*
* Provides factory to create concrete document object.
*
* @lib btui.exe
* @sice S60v3.0
*/
class CBTUIApp : public CAknApplication
    {

    private:

        /**
        * From CAknApplication Creates CBTUIDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();

        /**
        * From CAknApplication  Returns application's UID (KUidBTUI).
        * @return The value of KUidBTUI.
        */
        TUid AppDllUid() const;
    };

#endif



