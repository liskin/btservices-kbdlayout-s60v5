/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Interface class for mobile and voip lines.
*
*/

#ifndef BTMCLINE_H_
#define BTMCLINE_H_

#include <e32base.h>
#include <mcall.h>

class CBtmcCallActive;

class MBtmcLine
{
    public:
      virtual TInt CallStatusL() const = 0;            
      virtual const RPointerArray<CBtmcCallActive>& ActiveCalls() const = 0;
      virtual RMobilePhone& Phone() = 0;
      virtual ~MBtmcLine() { /**/ };
      virtual const MCall& CallInformationL() = 0;
      virtual TBool IsVoip() = 0;
};


#endif /* BTMCLINE_H_ */
