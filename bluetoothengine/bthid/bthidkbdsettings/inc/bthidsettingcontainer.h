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
* Description:  Declares setting container class.
 *
*/

#ifndef BTHIDSETTINGCONTAINER_H
#define BTHIDSETTINGCONTAINER_H

/**
 *  CBTHidSettingContainer  
 *  container control class.
 */
class CBTHidSettingContainer : public CCoeControl, public MCoeControlObserver
    {
public:
    // Constructors and destructor

    /**
     * EPOC default constructor.
     * @param aRect Frame rectangle for container.
     */
    void ConstructL(const TRect& aRect);

    /**
     * Constructs common parts of the container.
     * @param aRect control's rect.
     */
    void BaseConstructL(const TRect& aRect);

    /**
     * Destructor.
     */
    ~CBTHidSettingContainer();

private:
    // Functions from base classes

    /**
     * From CCoeControl,Draw.
     */
    void Draw(const TRect& aRect) const;

    /**
     * From CCoeControl
     */
    void HandleResourceChange(TInt aType);

    /**
     * From CoeControl,SizeChanged.
     */
    void SizeChanged();

    /**
     * From CoeControl,CountComponentControls.
     */
    TInt CountComponentControls() const;

    /**
     * From CCoeControl,ComponentControl.
     */
    CCoeControl* ComponentControl(TInt aIndex) const;

    /**
     * From MCoeControlObserver
     */
    void HandleControlEventL(CCoeControl* aControl, TCoeEvent aEventType);

    /**
     * Pass skin information if needed.
     */
    TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

    /**
     * From CCoeControl
     */
    void GetHelpContext(TCoeHelpContext& aContext) const;

private:
    //data
    CAknsBasicBackgroundControlContext* iBackGround; //for skins support


    };

#endif // BTHIDSETTINGCONTAINER_H
// End of File
