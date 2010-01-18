// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @file
 @publishedAll
 @released
*/

#include <remconstatusapicontrollerobserver.h>

/**
This virtual function allows the M- class to be extended in future in a binary
compatible way by providing a method that clients can override in future to
allow extra callbacks to be made via aObject.
*/
EXPORT_C void MRemConStatusApiControllerObserver::Mrcsaco_ExtensionInterfaceL(TUid /*aInterface*/, void*& aObject)
	{
	aObject = NULL;
	}
