/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

#ifndef BTMSGVIEWERUTILS_H_
#define BTMSGVIEWERUTILS_H_

#include <msvapi.h>


class CBtMsgViewerUtils: public CBase, public MMsvSessionObserver
    {    
public:
    static CBtMsgViewerUtils* NewL();
    ~CBtMsgViewerUtils();
    HBufC* GetMessagePath(TInt aMessageId, TInt aError);
    HBufC8* GetMimeType();

private: // From MMsvSessionObserver
    void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3);
    
private:
    void GetMessagePathL(TPtr aMsgPath, const TInt aMessageId);
    void StoreMessageMimeTypeL(TPtr aMsgPath);
       
private:
    CBtMsgViewerUtils();
    void ConstructL();
    
private:
    CMsvSession* iMsvSession;
    HBufC8*      iMimeType;
    };

#endif // BTMSGVIEWERUTILS_H_
