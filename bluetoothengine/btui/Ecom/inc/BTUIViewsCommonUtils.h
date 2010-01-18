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

#ifndef BTUIVIEWSCOMMONUTILS_H
#define BTUIVIEWSCOMMONUTILS_H

#include <e32base.h>

class CBTEngSettings;
/**
* Utils class which offer common functions that were used by all BTUI views
*/
class TBTUIViewsCommonUtils 
    {
    public:

        /**
        * Displays a general error note.
        *
        * @param None.
        * @return None.
        */
        static void ShowGeneralErrorNoteL();                     // Display general error note
        
        /** Check if there are any bluetooth connections. 
         *@param None
         *@return ETrue if there is one or more connections.
         */
		static TBool IsAnyDeviceConnectedL();
		
		/** Asks user to turn on bluetooth is it it not on.		
		* @param aBtEngSettings settings object used to turn on the bt.
		* @param aCoverDisplayEnabled. True if cover display is enabled.
		* @return KErrNone, if bluetooth is on, or has turned on. KErrCancel if user said no. System wide errorcode otherwise.
		*/
		static TInt TurnBTOnIfNeededL(CBTEngSettings* aBtEngSettings,TBool aCoverDisplayEnabled );		
 	};
 	
#endif