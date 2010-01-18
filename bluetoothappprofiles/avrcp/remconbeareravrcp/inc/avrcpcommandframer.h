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

#ifndef AVRCPCOMMANDFRAMER_H
#define AVRCPCOMMANDFRAMER_H

/**
@file
@internalComponent
@released
*/

#include <avcpanel.h>
#include <remcon/avrcpspec.h>

class CAVCFrame;
NONSHARABLE_CLASS(AvrcpCommandFramer)
	{
public:
	static CAVCFrame* PassthroughL(AVCPanel::TOperationId aOperationId, 
		AVCPanel::TButtonAction aButtonAction);
	
	static CAVCFrame* UnitInfoResponseL();
	static CAVCFrame* SubunitInfoResponseL();
	static CAVCFrame* SetAbsoluteVolumeCommandL(TUint8 aAbsVol);
	static CAVCFrame* NotifyVolumeChangeCommandL();
	};

#endif // AVRCPCOMMANDFRAMER_H
