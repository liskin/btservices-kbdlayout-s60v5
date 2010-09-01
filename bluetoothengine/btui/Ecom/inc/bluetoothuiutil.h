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
* Description:   This provides utility for loading a BT device name without
* localised to a string to be shown in GUI.
*
*/
#include <StringLoader.h>

class BluetoothUiUtil
    {
public:

/*
 * Gets the starting position of a key whose pattern matches
 * '%'{(0)*|<n>}<aKey> where 0 and <n> is the decimal value 
 * of the specified position and <aKey> is the symbol of the key, 
 * either 'U' or 'N'.
 * 
 * For example, if the specified position of a number key 
 * is 0, both "%N" and "%0N" match, 
 * but "%1N" doesn't. If the position n is greater than 0, 
 * only "%<n>N" matches.
 * @param aDes the descriptor where a key parameter will be searched.
 * @param aKey the specified key, 'U' or 'N'
 * @param aKeyPos the position of the key. position starts from 0.
 * @param aKeyLength the length of the key at return if the key exists.
 * @return the start position of the key in aDes. KErrNotFound if the key
 *          is not available.
 */
inline static TInt GetSubstringKeyPos( const TDes& aDes, const TDesC& aKey, 
        TInt aKeyPos, TInt& aKeyLength );

/*
 * Gets the starting position of a string key whose pattern matches
 * '%'{(0)*|<n>}'U' where 0 and <n> is the decimal value 
 * of the specified position. 
 * 
 * For example, if the specified position is 0, both "%U" and "%0U" match, 
 * but "%1U" doesn't. If the position n is greater than 0, 
 * only "%<n>U" matches.
 * @param aDes the descriptor in which a string key will be searched.
 * @param aKeyPos the position of the string key. position starts from 0.
 * @param aKeyLength the length of the string key at return if the key exists.
 * @return the start position of the string key in aDes. KErrNotFound if the key
 *          is not available.
 */
inline static TInt GetStringSubstringKeyPos( const TDes& aDes, TInt aKeyPos, TInt& aKeyLength );

/*
 * Add a substring to the specified RBuf descriptor.
 * ( Replace "%<n>U" at the specified position withe the given string.
 * @param aBuf the RBuf descriptor in which a "%<n>U" will be replaced.
 * @param aSub the substring that will replace a "%<n>U".
 * @param aSubPos the position of the "%U" to be replaced. Position starts from 0.
 */
inline static void AddSubstringL( RBuf& aBuf, const TDesC& aSub, TInt aSubPos);

/*
 * Load a string specified by the given resource ID, 
 * remove substrings that match "[" and ends with "]" and finally
 * replace the "%U" or "%<n>U" at the specified position with the
 * given substring.
 * 
 * This function is intended for adding a Bluetooth device name to a UI 
 * string but without localizing this name.
 * 
 * (Using StringLoader will get the device name localized in some variants. 
 * This is a workaround to tackle the unwanted localization.)
 * 
 * @param aBuf the RBuf descriptor in which a "%<n>U" will be replaced.
 * @param aSub the substring that will replace a "%<n>U".
 * @param aSubPos the position of the "%U" to be replaced. Position starts from 0.
 */
inline static void LoadResourceAndSubstringL(RBuf& aBuf,
        TInt aResourceId, const TDesC& aSub, TInt aSubPos );
    };

#include "bluetoothuiutil.inl"

// End of File

