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


#ifndef __LAYOUTUTILS_H__
#define __LAYOUTUTILS_H__

#include <e32base.h>
#include "hidlayoutids.h"

/*!
 Utility class providing static function for keyboard layout 
 code manipulation.
 */
class CLayoutUtils : public CBase
    {
public:
    /*!
     Return a position in the list of standard layout codes for a given
     layout.
     @param aLayoutId A Standard layout code.
     @result The index of the layout.
     */
    static TInt IndexFromStandardLayout(THidKeyboardLayoutId aLayoutId);

    /*!
     Return a position in the list of nokia layout codes for a given
     layout.
     @param aLayoutId A Nokia layout code.
     @result The index of the layout.
     */
    static TInt IndexFromNokiaLayout(THidKeyboardLayoutId aLayoutId);

    /*!
     Return a layout code for a given index in the list of standard layout
     codes.
     @param aIndex The index of the layout. 
     @result A Standard layout code.
     */
    static THidKeyboardLayoutId StandardLayoutFromIndex(TInt aIndex);

    /*!
     Return a layout code for a given index in the list of nokia layout
     codes.
     @param aIndex The index of the layout. 
     @result A Standard layout code.
     */
    static THidKeyboardLayoutId NokiaLayoutFromIndex(TInt aIndex);

    /*!
     Ensures a given layout is a valid standard layout. If it is not
     then a default standard value is given.
     @param aLayoutId A Standard layout code.
     */
    static void ValidateStandardLayout(THidKeyboardLayoutId& aLayoutId);

    /*!
     Ensures a given layout is a valid Nokia Su8 layout. If it is not
     then a default Nokia Su8 value is given.
     @param aLayoutId A Nokia Su8 layout code.
     */
    static void ValidateNokiaSU8Layout(THidKeyboardLayoutId& aLayoutId);

    /*!
     Identify the layout selected resource string for the given layout.
     @param aLayoutCode The layout code of the string to return.
     */
    static TInt LayoutConfirmationStringL(THidKeyboardLayoutId aLayoutCode);

    /*!
     Identify the initialed layout ID belongs to same group than the layout ID in CenRep
     @param aInitialLayoutCode the initialed layout ID.
     @param aLayoutCode the layout ID in CenRep.
     @return ETrue when IDs are in the same group ETrue, otherwise EFalse
     */
    static TBool SameCategory(THidKeyboardLayoutId aInitialLayoutCode,
            THidKeyboardLayoutId aLayoutCode);
    };

#endif // __LAYOUTUTILS_H__
