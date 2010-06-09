/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Interface class for AT Commands
*
*/

#ifndef ATCMDBASE_H_
#define ATCMDBASE_H_

/**
 *  Class for accessing AT command common functionality
 */
NONSHARABLE_CLASS( MATCmdBase )
    {
public:
    /**
     * @see CATExtPluginBase::HandleCommand
     */
    virtual void HandleCommand( const TDesC8& aCmd,
                                RBuf8& aReply,
                                TBool aReplyNeeded ) = 0;

    /**
     * @see CATExtPluginBase::HandleCommandCancel
     */
    virtual void HandleCommandCancel() = 0;
    };

#endif /* ATCMDBASE_H_ */
