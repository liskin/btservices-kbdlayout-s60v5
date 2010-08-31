/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef UPDATEMUSICCOLLECTION_H_
#define UPDATEMUSICCOLLECTION_H_


#include <e32std.h>
#include <e32base.h>
#include <mpxcollectionhelper.h>
#include <mpxcollectionhelperobserver.h>
#include <mpxplaybackutility.h>

#include <mpxplaybackobserver.h>


class MMPXCollectionHelperObserver;
class MMPXPlaybackObserver;

NONSHARABLE_CLASS( CUpdateMusicCollection ): public CBase ,
                                             public MMPXCollectionHelperObserver,
                                             public MMPXPlaybackObserver

    {
   
    public: //cunstructor and distructors
        
        static CUpdateMusicCollection* NewL();  // Constructor (public)
       
        void ConstructL(); 
        
        ~CUpdateMusicCollection();
        
    public: //callback functions from the observers
        
        // from MMPXCollectionHelperObserver
        // Called back after a call to
        // CCollectionHelper::AddL() to provide
        // status
        void HandleAddFileCompleteL( TInt aErr ); 
       
        // from MMPXPlaybackObserver
        // If aErr is not KErrNone, plugin might 
        // still call back with more info in the aMsg.
        void HandlePlaybackMessage( CMPXMessage* aMessage, TInt aError );
       
    public: //own function
        
        //add the music to misic collection
        void addToCollectionL(const TDesC& aFileName);
        
        //check if the type of the recieved file is supported by music player
        TInt isSupported (const TDesC16& aPtr); 
    
    protected:       
        
         CUpdateMusicCollection();
      
    private:
        
        MMPXCollectionHelper* iCollectionHelper;
        MMPXPlaybackUtility* iPlaybackUtility;
    };

#endif /*UPDATEMUSICCOLLECTION_H_*/
