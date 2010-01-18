/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   This is the implementation of main container. It contains 
*				  all necessary AVKON components and handles user inputs.
*
*/

#include <e32base.h>
#include <centralrepository.h> 
#include <barsread.h>   // Resource reader
#include <BtuiViewResources.rsg>     // Compiled resource ids
#include <btfeaturescfg.h>					// For EnterpriseEnablementL()
#include "BTUIMainContainer.h"
#include "debug.h"      // Global declarations
#include "btui.h"
#ifdef __SERIES60_HELP
#include <csxhelp/bt.hlp.hrh>   // Help ids
#endif

// ---------------------------------------------------------
// CBTUIMainContainer::ConstructL
// Symbian OS 2nd phase constructor can leave
// ---------------------------------------------------------
//
void CBTUIMainContainer::ConstructL(const TRect& aRect, CBTUIMainView* aMainView )
    {
    TRACE_FUNC_ENTRY

    CreateWindowL();

	iMainView = aMainView;
	
    // Check if listbox is empty
    __ASSERT_DEBUG(iBtMainListBox==0, PANIC( EBTMainListNotEmpty ));
    __ASSERT_DEBUG(iBtAllItemsArray==0, PANIC( EBTAllFormatStingsArrayNotEmpty ));
    __ASSERT_DEBUG(iBtListItemArray==0, PANIC( EBTFormatStingsArrayNotEmpty ));

    // Create main itemlist
    iBtMainListBox = new (ELeave) CAknSettingStyleListBox;
    iBtMainListBox->SetContainerWindowL( *this );
    iBtMainListBox->ConstructL( this, EAknListBoxMarkableList );


    // Update settings from original sources via model    
    iBtEngSettingsRef = iMainView->GetBtSettingsReference();    
        
    __ASSERT_DEBUG(iBtEngSettingsRef!=0, PANIC(EBTPanicIllegalValue));
        
    // Read all format strings from resources and create array for them
    iBtAllItemsArray = iCoeEnv->ReadDesCArrayResourceL( R_BT_LABEL_STRINGS );
    iBtListItemArray = new (ELeave) TMainListItemArray( 
      iBtAllItemsArray, aMainView, iBtEngSettingsRef );

    // Set up/down arrows at bottom of the screen (scrollbar)
    iBtMainListBox->CreateScrollBarFrameL( ETrue );
    iBtMainListBox->ScrollBarFrame()->SetScrollBarVisibilityL( 
      CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto );

    // Set text labels for list items again (overwriting resource definitions)
    iBtMainListBox->Model()->SetItemTextArray( iBtListItemArray );
    iBtMainListBox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );   // Does not delete items array

    // Set up the observer (events listener)
    iBtMainListBox->SetListBoxObserver( this );

    
    
    SetRect(aRect);
    ActivateL();    

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CBTUIMainContainer::~CBTUIMainContainer()
    {
	TRACE_FUNC_ENTRY		   
    delete iBtMainListBox;
    delete iBtAllItemsArray;
    delete iBtListItemArray;
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CBTUIMainContainer::SizeChanged()
// From CCoeControl
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CBTUIMainContainer::SizeChanged()
    {
	TRACE_FUNC_ENTRY    
    iBtMainListBox->SetRect(Rect());    // Mandatory, otherwise not drawn
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUIMainContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CBTUIMainContainer::CountComponentControls() const
    {
	TRACE_FUNC_ENTRY    
	if(iBtMainListBox )
    	return  1;
    else
    	return  0; // return nbr of controls inside this container
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUIMainContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CBTUIMainContainer::ComponentControl(TInt aIndex) const
    {
    TRACE_FUNC_ENTRY
    if( aIndex ==0 )
    	{
    	return iBtMainListBox;
    	}
    return NULL;
    }

// ---------------------------------------------------------
// From CCoeControl
// CBTUIMainContainer::Draw
// ---------------------------------------------------------
//
void CBTUIMainContainer::Draw(const TRect& /* aRect */) const
    {
    }

// ---------------------------------------------------------
// From CoeControl
// CBTUIMainContainer::HandleControlEventL
// ---------------------------------------------------------
//
void CBTUIMainContainer::HandleControlEventL( CCoeControl* /* aControl */, TCoeEvent /* aEventType */)
    {
    }

// ---------------------------------------------------------
// From CoeControl
// CBTUIMainContainer::OfferKeyEventL
// Redirect keypresses to the listbox
// ---------------------------------------------------------
//
TKeyResponse CBTUIMainContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
    {
	TRACE_FUNC_ENTRY    
    if(( iBtMainListBox ) && ( aKeyEvent.iCode != EKeyLeftArrow ) 
      && (aKeyEvent.iCode != EKeyRightArrow ))
        {
        TKeyResponse res = iBtMainListBox->OfferKeyEventL( aKeyEvent, aType );
        
        // Store current item index for options list usage later
        //
        iCurrentItemIndex = iBtMainListBox->CurrentItemIndex();
        return res;
        }
    return EKeyWasNotConsumed;
    }

// ---------------------------------------------------------
// From MEikListBoxObserver 
// CBTUIMainContainer::HandleListBoxEventL
// ---------------------------------------------------------
//
void CBTUIMainContainer::HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType)
    {
	TRACE_FUNC_ENTRY
	
    __ASSERT_DEBUG(iMainView!=0, PANIC(EBTPanicIllegalValue));

	// this is no longer called, unless MSK is disabled.
	// if msk is not disabled, then  
	// BTMainView::HandleCommand(EBTUICmdMSK) is called instead
    if( aEventType == EEventEnterKeyPressed
    || aEventType == EEventItemSingleClicked ) // Selection key pressed, or item clicked
        {
        TInt currentHighLightPosition = aListBox->CurrentItemIndex();
        switch(currentHighLightPosition)
            {
            case EBTMainListItemPowerMode:
                iMainView->HandleCommandL( EBTUICmdChangePowerStateSelect );
                break;
            case EBTMainListItemVisibilityMode:
                iMainView->HandleCommandL( EBTUICmdChangeVisibilitySelect );
                break;
            case EBTMainListItemNameSetting:
                iMainView->HandleCommandL( EBTUICmdChangeBTNameSelect );
                break;
                
			// For Bluetooth SIM Access Profile feature
			//
			case EBTMainListItemSapMode:
				iMainView->HandleCommandL( EBTUICmdChangeSapStateSelect );
				break;
            default:
                break;
            }
        aListBox->DrawItem(currentHighLightPosition);   // Redraw item only to the screen
        }
    }

// ----------------------------------------------------
// From CEikListBox 
// CBTUIMainContainer::CurrentItemIndex
// ----------------------------------------------------
//
TInt CBTUIMainContainer::CurrentItemIndex()
    {
	TRACE_FUNC_ENTRY

    return iBtMainListBox->CurrentItemIndex();
    }

// ----------------------------------------------------
// CBTUIMainContainer::SettingChangedL
// ----------------------------------------------------
//
void CBTUIMainContainer::SettingChanged( TBTMainListItemIndexes /* aListItem */ )
    {
	TRACE_FUNC_ENTRY

    iBtMainListBox->DrawDeferred();
    }    
    
#ifdef __SERIES60_HELP

// ---------------------------------------------------------
// From CCoeControl
// CBTUIMainContainer::GetHelpContext
// ---------------------------------------------------------
//
void CBTUIMainContainer::GetHelpContext(TCoeHelpContext& aContext) const
    {
 	TRACE_FUNC_ENTRY

    aContext.iMajor = KBtuiUid3;
    aContext.iContext = KBT_HLP_MAIN;
    }

#else

// ---------------------------------------------------------
// From CCoeControl
// CBTUIMainContainer::GetHelpContext
// ---------------------------------------------------------
//
void CBTUIMainContainer::GetHelpContext(TCoeHelpContext& /*aContext*/) const
    {
	TRACE_FUNC_ENTRY    
    }

#endif

// ---------------------------------------------------------
// From CCoeControl
// CBTUIMainContainer::HandleResourceChange
// ---------------------------------------------------------
//
void CBTUIMainContainer::HandleResourceChange( TInt aType )
    {
	TRACE_FUNC_ENTRY
	//Pass the event to base class, so whenever resource changing happened, 
	//this control will be considered to be re-drawed by base class, therefore
	//no specific DrawNow() function call is needed here.
    CCoeControl::HandleResourceChange(aType); 
    
    //Handle change in layout orientation
    //aType == KAknsMessageSkinChange is handled by base class.
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        //"TRect rect = iAvkonAppUi->ClientRect();"
        // The line above provides to big rectangle in the bottom causing cba's overdrawn by by blank area.
        // Correct way to do this is below.
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
		}      
    }
    
// ---------------------------------------------------------
// From CCoeControl
// CBTUIMainContainer::FocusChanged
// ---------------------------------------------------------
//
void CBTUIMainContainer::FocusChanged(TDrawNow /*aDrawNow*/)
    {
	TRACE_FUNC_ENTRY        
	  if( iBtMainListBox)
        {
        iBtMainListBox->SetFocus( IsFocused() );
        }
    }

    
// ******************************************* 
//    Nested class function implemenations:
// *******************************************

// ----------------------------------------------------
// CBTUIMainContainer::TMainListItemArray::TMainListItemArray
// Initialize friend class
// ----------------------------------------------------
CBTUIMainContainer::TMainListItemArray::TMainListItemArray( CDesCArray* aBtAllItemsArray, 
																CBTUIMainView* aMainView, CBTEngSettings* aBtEngSettings)
																
        : iArray( *aBtAllItemsArray ),  // Store references
        iMainView( aMainView ),        
        iBtEngSettingsRef( aBtEngSettings )
        
    {
	TRACE_FUNC_ENTRY        
    }

// ----------------------------------------------------
// CBTUIMainContainer::TMainListItemArray::MdcaCount()
// ----------------------------------------------------
//
TInt CBTUIMainContainer::TMainListItemArray::MdcaCount() const
    {
	TRACE_FUNC_ENTRY    
    return KNumberOfMainViewItems;
    }

// ----------------------------------------------------
// CBTUIMainContainer::TMainListItemArray::MdcaPointL
// Main view listbox item formatter:
// Returns the current format string depending value
// of the item concerned.
// ----------------------------------------------------
//
TPtrC CBTUIMainContainer::TMainListItemArray::MdcaPoint( TInt aIndex ) const
    {
	TRACE_FUNC_ENTRY    
    _LIT(KBTUIEmptyFormatString, " \t \t\t "); // Empty format string if illegal indexing
    TPtrC chosen;
    	
    switch ( aIndex )
        {
        case EBTMainListItemPowerMode:
            {
            TBTPowerStateValue powerState;
            iBtEngSettingsRef->GetPowerState(powerState);            
            if( powerState==EBTPowerOn )
                chosen.Set( iArray.MdcaPoint( EBTUISettModulePowerOn ));
            else
                chosen.Set( iArray.MdcaPoint( EBTUISettModulePowerOff ));
            break;
            }

        case EBTMainListItemVisibilityMode:
            {
        	TBTVisibilityMode mode;
        	iBtEngSettingsRef->GetVisibilityMode(mode);          	
            
            switch(mode)
                {                
                case EBTVisibilityModeGeneral:
                    chosen.Set( iArray.MdcaPoint( EBTUISettVisibilityModeGeneral ));
                    break;
                                 
                case EBTVisibilityModeHidden:
                    chosen.Set( iArray.MdcaPoint( EBTUISettVisibilityModeHidden ));
                    break;
                   
                case EBTVisibilityModeTemporary:
                    chosen.Set( iArray.MdcaPoint( EBTUISettVisibilityModeTemp ));
                    break;                    
                   
                default:
                    break;
                }
            break;
            }

        case EBTMainListItemNameSetting:
            {
            TDes &buf = MUTABLE_CAST(TBuf<KBTUIMaxFormattedNameLength>&,iItemText);
            buf.Zero();
            buf.Append( iArray.MdcaPoint( EBTUISettBTNameHeader ) );
            buf.Append( *iMainView->GetBTLocalName() );
            chosen.Set( iItemText );
            break;
            }

		// For Bluetooth SIM Access Profile feature
		case EBTMainListItemSapMode:
            {
			BluetoothFeatures::TEnterpriseEnablementMode mode = BluetoothFeatures::EEnabled;
			TRAPD(err, mode = BluetoothFeatures::EnterpriseEnablementL());
			if ( err == KErrNone )
				{
				if ( mode == BluetoothFeatures::EDataProfilesDisabled )
					{
					chosen.Set( iArray.MdcaPoint( EBTUISettSapDisabled ));
					break;
					}
				}
			
			TInt sapStatus;    
			TRAP_IGNORE(GetSapStatusL(sapStatus););
			    
			if(sapStatus==EBTSapEnabled)
				{
				 chosen.Set( iArray.MdcaPoint( EBTUISettSapEnabled ));
				}
			else
				{
				 chosen.Set( iArray.MdcaPoint( EBTUISettSapDisabled ));
				}          
				   
			TInt sapMode = KErrNone;
			TRAP_IGNORE(sapMode = iMainView->GetSapStatusL());  
			
			
			if(sapMode==EBTSapEnabled)
				{
				if ( sapMode ) 
					{			
					chosen.Set( iArray.MdcaPoint( EBTUISettSapEnabled ));
					}
				else
					{
					chosen.Set( iArray.MdcaPoint( EBTUISettSapDisabled ));
					}
				}
			               
            break;
            }	
        default:
            chosen.Set( KBTUIEmptyFormatString );
            break;
        }

    return chosen;
    }

// ----------------------------------------------------------------------
// CBTUIMainContainer::TMainListItemArray::GetSapStatusL(TInt& aStatus)
// ----------------------------------------------------------------------
//
void CBTUIMainContainer::TMainListItemArray::GetSapStatusL( TInt& aStatus )
{    
    CRepository* repository = CRepository::NewL(KCRUidBTEngPrivateSettings);
    CleanupStack::PushL(repository);            	
    repository->Get(KBTSapEnabled, aStatus);
    CleanupStack::PopAndDestroy(repository);      
}
