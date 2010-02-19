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

inline TInt BluetoothUiUtil::GetSubstringKeyPos( 
        const TDes& aDes, const TDesC& aKey, 
        TInt aKeyPos, TInt& aKeyLength )
    {
    ASSERT( aKeyPos > KErrNotFound );
    _LIT(KPercentSymbol, "%" );
    // 32 would be to fit any key substring in practice:
    TBuf<32> key;
    TInt pos( KErrNotFound );
    // if the specified position is 0, check if "%<aKey>" (%U or %N) exists.
    if ( aKeyPos == 0 )
        {
        key.Copy( KPercentSymbol );
        key.Append(aKey);
        pos = aDes.Find( key );
        }
    // Either the key is not found, or aKeyPos is not 0.
    if ( pos == KErrNotFound )
        {
        key.Copy( KPercentSymbol );
        key.AppendNum( aKeyPos );
        key.Append( aKey );
        pos = aDes.Find( key );
        }
    if ( pos > KErrNotFound )
        {
        aKeyLength = key.Length();
        }
    return pos;
    }

inline TInt BluetoothUiUtil::GetStringSubstringKeyPos( 
        const TDes& aDes, TInt aKeyPos, TInt& aKeyLength )
    {
    _LIT(KStringKeyword,"U");
    TPtrC ptr( KStringKeyword );
    return GetSubstringKeyPos(aDes, ptr, aKeyPos, aKeyLength );
    }

inline void BluetoothUiUtil::AddSubstringL( 
        RBuf& aBuf, const TDesC& aSub, TInt aSubPos)
    {
    // Find the position of the specified "%U" or %[0-9]U in aBuf.
    TInt keyLength;
    TInt pos = GetStringSubstringKeyPos( aBuf, aSubPos, keyLength );
    if ( pos == KErrNotFound )
        {
        User::Leave( pos );
        }
    // Enlarge the max length of aBuf if needed:
    TInt reqedLen = aBuf.Length() - keyLength + aSub.Length();
    if ( aBuf.MaxLength() <  reqedLen )
        {
        aBuf.ReAllocL( reqedLen );
        }
    aBuf.Replace( pos, keyLength, aSub );
    }

inline void BluetoothUiUtil::LoadResourceAndSubstringL(RBuf& aBuf,
        TInt aResourceId, const TDesC& aSub, TInt aSubPos )
    {
    HBufC* string = StringLoader::LoadL( aResourceId );
    aBuf.Assign( string );
    // Logical loc strings may contain redundant "[<chars>]" for some reason.
    // This sub-string should be removed in our ad hoc parameter loading method.
    _LIT( KLeftSquareBracket,"[" );
    _LIT( KRightSquareBracket,"]" );
    while ( 1 )
        {
        TInt lpos = aBuf.Find( KLeftSquareBracket );
        TInt rpos = aBuf.Find( KRightSquareBracket );
        if ( lpos > KErrNotFound && rpos > KErrNotFound && ( rpos > lpos ) )
            {
            aBuf.Delete(lpos, rpos - lpos + 1 );
            continue;
            }
        // No sub-strings that match "[" and ends with "]". Done
        break;
        }
    // Add the substring to this string loaded from resource:
    AddSubstringL( aBuf,  aSub, aSubPos );
    }

// End of File

