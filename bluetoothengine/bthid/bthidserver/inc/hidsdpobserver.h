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
* Description:  Declares main application class.
 *
*/


#ifndef __HIDSDPOBSERVER_H__
#define __HIDSDPOBSERVER_H__

/*!
 This class specifies the hid parser observer interface
 Used in conjunction with CHidSdpClient class
 */

class MHidSdpObserver
    {
public:

    /*!
     Reports that the hid device sdp query has finished.
     @param aResult System error code.
     */
    virtual void HidSdpSearchComplete(TInt aResult) = 0;

    };

#endif // __HIDSDPOBSERVER_H__
