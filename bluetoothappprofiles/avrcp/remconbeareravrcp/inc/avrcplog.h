// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Declares logging and instrumentation for avrcp (when flogging active)
//



/**
 @file
 @internalComponent
*/

#ifndef AVRCPLOG_H
#define AVRCPLOG_H

#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_AVRCP_BEARER);
#endif // __FLOG_ACTIVE

#define AVRCPLOG(A) LOG(A)
#define AVRCPLOG2(A,B) LOG1(A,B)
#define AVRCPLOG3(A,B,C) LOG2(A,B,C)
#define AVRCPLOG4(A,B,C,D) LOG3(A,B,C,D)
#define AVRCPLOG5(A,B,C,D,E) LOG4(A,B,C,D,E)

#endif //AVRCPLOG_H

