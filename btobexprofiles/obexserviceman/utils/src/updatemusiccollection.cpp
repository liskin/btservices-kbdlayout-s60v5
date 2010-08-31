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
* Description:  update the music library after the musics are recieved using direct file saving
*
*/



//#include <mpxcollectionhelper.h>
#include <mpxcollectionhelperfactory.h>

#include "updatemusiccollection.h"
#include "obexutilsdebug.h"

//#include <mpxplaybackutility.h>
#include <mpxmessagegeneraldefs.h>

// -----------------------------------------------------------------------------
// CUpdateMusicCollection::CUpdateMusicCollection
// Default constructor
// -----------------------------------------------------------------------------
CUpdateMusicCollection::CUpdateMusicCollection() 
    {
    }

// -----------------------------------------------------------------------------
// CUpdateMusicCollection::NewL
// -----------------------------------------------------------------------------

CUpdateMusicCollection* CUpdateMusicCollection::NewL()
    {
    CUpdateMusicCollection* self = new( ELeave ) CUpdateMusicCollection();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
// -----------------------------------------------------------------------------
// CUpdateMusicCollection::ConstructL
// Symbian OS default constructor can leave.
// -----------------------------------------------------------------------------
 
void CUpdateMusicCollection::ConstructL()
    {
    iCollectionHelper = 
        CMPXCollectionHelperFactory::NewCollectionHelperL();
    iPlaybackUtility = MMPXPlaybackUtility::NewL( KPbModeDefault, this );

    }
// -----------------------------------------------------------------------------
// CUpdateMusicCollection::addToCollectionL
// add the music to music library
// -----------------------------------------------------------------------------
 
void CUpdateMusicCollection::addToCollectionL(const TDesC& aFileName)
    {
    iCollectionHelper->AddL(aFileName,this);
    }

// -----------------------------------------------------------------------------
// CUpdateMusicCollection::isSupported
// checks if the file is supported by music library
// -----------------------------------------------------------------------------
 TInt CUpdateMusicCollection::isSupported (const TDesC16& aPtr)
    {
    CDesCArray* SupportedMimeTypes =
            iPlaybackUtility->SupportedMimeTypes();
    TInt pos = 0;
    return  SupportedMimeTypes->Find(aPtr,pos);
    }
 
 // -----------------------------------------------------------------------------
 // CUpdateMusicCollection::HandleAddFileCompleteL
 // callbackCalled back after a call to CCollectionHelper::AddL() to provide status
 // -----------------------------------------------------------------------------
  void CUpdateMusicCollection::HandleAddFileCompleteL( TInt aErr )
    {    
    if(aErr == KErrNone)
        {
        FLOG(_L("[CUpdateMusicCollection]\t HandleAddFileCompleteL successful"));    
        }
    else
        {
         FLOG(_L("[CUpdateMusicCollection]\t HandleAddFileCompleteL unsuccessful"));    
        }
  
    }
 // -----------------------------------------------------------------------------
 // CUpdateMusicCollection::HandlePlaybackMessage
 // -----------------------------------------------------------------------------
   
void CUpdateMusicCollection::HandlePlaybackMessage( CMPXMessage* aMessage,TInt aError )
   {
     if ( aError == KErrNone && aMessage )
         {
         FLOG(_L("[CUpdateMusicCollection]\t HandleAddFileCompleteL unsuccessful"));
        }
     }

// -----------------------------------------------------------------------------
// CUpdateMusicCollection::~CUpdateMusicCollection
// default destructor
// -----------------------------------------------------------------------------
   
CUpdateMusicCollection::~CUpdateMusicCollection()
    {
    if ( iCollectionHelper )
        {
            iCollectionHelper->Close();
        }    
    if (iPlaybackUtility)
        {
        iPlaybackUtility->Close();
        }
    }
