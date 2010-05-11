// Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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
// TPlayerFeatureBitmask.cpp
//

#include <remconcoreapi.h>
#include "avrcplog.h"
#include "avrcputils.h"
#include "mediabrowse.h"
#include "nowplaying.h"
#include "playerbitmasks.h"
#include "remcongroupnavigation.h"


TPlayerFeatureBitmask::TPlayerFeatureBitmask()
	{
	iFeatureBitmask.FillZ(KFeatureBitmaskLength);
	}

TPlayerFeatureBitmask::TPlayerFeatureBitmask(const TPlayerFeatureBitmask& aFeatureBitmask)
	{
	iFeatureBitmask.Copy(aFeatureBitmask.FeatureBitmask());
	}

void TPlayerFeatureBitmask::SetCoreApiFeatures(const RArray<TUint>& aOperation)
	{
	if(aOperation.Count() > 0)
		{
		for(TInt j = 0; j< aOperation.Count() ; j++)
			{
			switch(aOperation[j])
				{
			case ERemConCoreApiSelect:
				iFeatureBitmask[ESelectByteOffset] |= (1<<ESelectBitOffset);
				break;
			case ERemConCoreApiUp:
				iFeatureBitmask[EUpByteOffset]|= (1<<EUpBitOffset);
				break;
			case ERemConCoreApiDown:
				iFeatureBitmask[EDownByteOffset]|= (1<<EDownBitOffset);
				break;
			case ERemConCoreApiLeft:
				iFeatureBitmask[ELeftByteOffset]|= (1<<ELeftBitOffset);
				break;
			case ERemConCoreApiRight:
				iFeatureBitmask[ERightByteOffset]|= (1<<ERightBitOffset);
				break;
			case ERemConCoreApiRightUp:
				iFeatureBitmask[ERightUpByteOffset]|= (1<<ERightUpBitOffset);
				break;
			case ERemConCoreApiRightDown:
				iFeatureBitmask[ERightDownByteOffset]|= (1<<ERightDownBitOffset);
				break;
			case ERemConCoreApiLeftUp:
				iFeatureBitmask[ELeftUpByteOffset]|= (1<<ELeftUpBitOffset);
				break;
			case ERemConCoreApiLeftDown:
				iFeatureBitmask[ELeftDownByteOffset]|= (1<<ELeftDownBitOffset);
				break;
			case ERemConCoreApiRootMenu:
				iFeatureBitmask[ERootMenuByteOffset]|= (1<<ERootMenuBitOffset);
				break;
			case ERemConCoreApiSetupMenu:
				iFeatureBitmask[ESetupMenuByteOffset]|= (1<<ESetupMenuBitOffset);
				break;
			case ERemConCoreApiContentsMenu:
				iFeatureBitmask[EContentsMenuByteOffset]|= (1<<EContentsMenuBitOffset);
				break;
			case ERemConCoreApiFavoriteMenu:
				iFeatureBitmask[EFavoriteMenuByteOffset]|= (1<<EFavoriteMenuBitOffset);
				break;
			case ERemConCoreApiExit:
				iFeatureBitmask[EExitByteOffset]|= (1<<EExitBitOffset);
				break;
			case ERemConCoreApi0:
				iFeatureBitmask[E0ByteOffset]|= (1<<E0BitOffset);
				break;
			case ERemConCoreApi1:
				iFeatureBitmask[E1ByteOffset]|= (1<<E1BitOffset);
				break;
			case ERemConCoreApi2:
				iFeatureBitmask[E2ByteOffset]|= (1<<E2BitOffset);
				break;
			case ERemConCoreApi3:
				iFeatureBitmask[E3ByteOffset]|= (1<<E3BitOffset);
				break;
			case ERemConCoreApi4:
				iFeatureBitmask[E4ByteOffset]|= (1<<E4BitOffset);
				break;
			case ERemConCoreApi5:
				iFeatureBitmask[E5ByteOffset]|= (1<<E5BitOffset);
				break;
			case ERemConCoreApi6:
				iFeatureBitmask[E6ByteOffset]|= (1<<E6BitOffset);
				break;
			case ERemConCoreApi7:
				iFeatureBitmask[E7ByteOffset]|= (1<<E7BitOffset);
				break;
			case ERemConCoreApi8:
				iFeatureBitmask[E8ByteOffset]|= (1<<E8BitOffset);
				break;	
			case ERemConCoreApi9:
				iFeatureBitmask[E9ByteOffset]|= (1<<E9BitOffset);
				break;
			case ERemConCoreApiDot:
				iFeatureBitmask[EDotByteOffset]|= (1<<EDotBitOffset);
				break;
			case ERemConCoreApiEnter:
				iFeatureBitmask[EEnterByteOffset]|= (1<<EEnterBitOffset);
				break;
			case ERemConCoreApiClear:
				iFeatureBitmask[EClearByteOffset]|= (1<<EClearBitOffset);
				break;
			case ERemConCoreApiChannelUp:
				iFeatureBitmask[EChannelUpByteOffset]|= (1<<EChannelUpBitOffset);
				break;
			case ERemConCoreApiChannelDown:
				iFeatureBitmask[EChannelDownByteOffset]|= (1<<EChannelDownBitOffset);
				break;
			case ERemConCoreApiPreviousChannel:
				iFeatureBitmask[EPreviousChannelByteOffset]|= (1<<EPreviousChannelBitOffset);
				break;
			case ERemConCoreApiSoundSelect:
				iFeatureBitmask[ESoundSelectByteOffset]|= (1<<ESoundSelectBitOffset);
				break;
			case ERemConCoreApiInputSelect:
				iFeatureBitmask[EInputSelectByteOffset]|= (1<<EInputSelectBitOffset);
				break;
			case ERemConCoreApiDisplayInformation:
				iFeatureBitmask[EDisplayInformationByteOffset]|= (1<<EDisplayInformationBitOffset);
				break;
			case ERemConCoreApiHelp:
				iFeatureBitmask[EHelpByteOffset]|= (1<<EHelpBitOffset);
				break;
			case ERemConCoreApiPageUp:
				iFeatureBitmask[EPageUpByteOffset]|= (1<<EPageUpBitOffset);
				break;
			case ERemConCoreApiPageDown:
				iFeatureBitmask[EPageDownByteOffset]|= (1<<EPageDownBitOffset);
				break;
			case ERemConCoreApiPower:
				iFeatureBitmask[EPowerByteOffset]|= (1<<EPowerBitOffset);
				break;
			case ERemConCoreApiVolumeUp:
				iFeatureBitmask[EVolumeUpByteOffset]|= (1<<EVolumeUpBitOffset);
				break;
			case ERemConCoreApiVolumeDown:
				iFeatureBitmask[EVolumeDownByteOffset]|= (1<<EVolumeDownBitOffset);
				break;
			case ERemConCoreApiMute:
				iFeatureBitmask[EMuteByteOffset]|= (1<<EMuteBitOffset);
				break;
			case ERemConCoreApiPlay:
				iFeatureBitmask[EPlayByteOffset]|= (1<<EPlayBitOffset);
				break;
			case ERemConCoreApiStop:
				iFeatureBitmask[EStopByteOffset]|= (1<<EStopBitOffset);
				break;
			case ERemConCoreApiPause:
				iFeatureBitmask[EPauseByteOffset]|= (1<<EPauseBitOffset);
				break;
			case ERemConCoreApiRecord:
				iFeatureBitmask[ERecordByteOffset]|= (1<<ERecordBitOffset);
				break;
			case ERemConCoreApiRewind:
				iFeatureBitmask[ERewindByteOffset]|= (1<<ERewindBitOffset);
				break;
			case ERemConCoreApiFastForward:
				iFeatureBitmask[EFastForwardByteOffset]|= (1<<EFastForwardBitOffset);
				break;
			case ERemConCoreApiEject:
				iFeatureBitmask[EEjectByteOffset]|= (1<<EEjectBitOffset);
				break;
			case ERemConCoreApiForward:
				iFeatureBitmask[EForwardByteOffset]|= (1<<EForwardBitOffset);
				break;
			case ERemConCoreApiBackward:
				iFeatureBitmask[EBackwardByteOffset]|= (1<<EBackwardBitOffset);
				break;
			case ERemConCoreApiAngle:
				iFeatureBitmask[EAngleByteOffset]|= (1<<EAngleBitOffset);
				break;
			case ERemConCoreApiSubpicture:
				iFeatureBitmask[ESubpictureByteOffset]|= (1<<ESubpictureBitOffset);
				break;
			case ERemConCoreApiF1:
				iFeatureBitmask[EF1ByteOffset]|= (1<<EF1BitOffset);
				break;
			case ERemConCoreApiF2:
				iFeatureBitmask[EF2ByteOffset]|= (1<<EF2BitOffset);
				break;
			case ERemConCoreApiF3:
				iFeatureBitmask[EF3ByteOffset]|= (1<<EF3BitOffset);
				break;
			case ERemConCoreApiF4:
				iFeatureBitmask[EF4ByteOffset]|= (1<<EF4BitOffset);
				break;
			case ERemConCoreApiF5:
				iFeatureBitmask[EF5ByteOffset]|= (1<<EF5BitOffset);
				break;
			default:
				// Ignore operations that aren't recorded in feature bitmask
				break;
				}
			}
		}
	else
		{
		// Don't know specific info, set all operations
		SetCoreApiFeatures();
		}
	}

void TPlayerFeatureBitmask::SetCoreApiFeatures()
	{
	iFeatureBitmask[ESelectByteOffset] |= (1<<ESelectBitOffset);
	iFeatureBitmask[EUpByteOffset]|= (1<<EUpBitOffset);
	iFeatureBitmask[EDownByteOffset]|= (1<<EDownBitOffset);
	iFeatureBitmask[ELeftByteOffset]|= (1<<ELeftBitOffset);
	iFeatureBitmask[ERightByteOffset]|= (1<<ERightBitOffset);
	iFeatureBitmask[ERightUpByteOffset]|= (1<<ERightUpBitOffset);
	iFeatureBitmask[ERightDownByteOffset]|= (1<<ERightDownBitOffset);
	iFeatureBitmask[ELeftUpByteOffset]|= (1<<ELeftUpBitOffset);
	iFeatureBitmask[ELeftDownByteOffset]|= (1<<ELeftDownBitOffset);
	iFeatureBitmask[ERootMenuByteOffset]|= (1<<ERootMenuBitOffset);
	iFeatureBitmask[ESetupMenuByteOffset]|= (1<<ESetupMenuBitOffset);
	iFeatureBitmask[EContentsMenuByteOffset]|= (1<<EContentsMenuBitOffset);
	iFeatureBitmask[EFavoriteMenuByteOffset]|= (1<<EFavoriteMenuBitOffset);
	iFeatureBitmask[EExitByteOffset]|= (1<<EExitBitOffset);
	iFeatureBitmask[E0ByteOffset]|= (1<<E0BitOffset);
	iFeatureBitmask[E1ByteOffset]|= (1<<E1BitOffset);
	iFeatureBitmask[E2ByteOffset]|= (1<<E2BitOffset);
	iFeatureBitmask[E3ByteOffset]|= (1<<E3BitOffset);
	iFeatureBitmask[E4ByteOffset]|= (1<<E4BitOffset);
	iFeatureBitmask[E5ByteOffset]|= (1<<E5BitOffset);
	iFeatureBitmask[E6ByteOffset]|= (1<<E6BitOffset);
	iFeatureBitmask[E7ByteOffset]|= (1<<E7BitOffset);
	iFeatureBitmask[E8ByteOffset]|= (1<<E8BitOffset);
	iFeatureBitmask[E9ByteOffset]|= (1<<E9BitOffset);
	iFeatureBitmask[EDotByteOffset]|= (1<<EDotBitOffset);
	iFeatureBitmask[EEnterByteOffset]|= (1<<EEnterBitOffset);
	iFeatureBitmask[EClearByteOffset]|= (1<<EClearBitOffset);
	iFeatureBitmask[EChannelUpByteOffset]|= (1<<EChannelUpBitOffset);
	iFeatureBitmask[EChannelDownByteOffset]|= (1<<EChannelDownBitOffset);
	iFeatureBitmask[EPreviousChannelByteOffset]|= (1<<EPreviousChannelBitOffset);
	iFeatureBitmask[ESoundSelectByteOffset]|= (1<<ESoundSelectBitOffset);
	iFeatureBitmask[EInputSelectByteOffset]|= (1<<EInputSelectBitOffset);
	iFeatureBitmask[EDisplayInformationByteOffset]|= (1<<EDisplayInformationBitOffset);
	iFeatureBitmask[EHelpByteOffset]|= (1<<EHelpBitOffset);
	iFeatureBitmask[EPageUpByteOffset]|= (1<<EPageUpBitOffset);
	iFeatureBitmask[EPageDownByteOffset]|= (1<<EPageDownBitOffset);
	iFeatureBitmask[EPowerByteOffset]|= (1<<EPowerBitOffset);
	iFeatureBitmask[EVolumeUpByteOffset]|= (1<<EVolumeUpBitOffset);
	iFeatureBitmask[EVolumeDownByteOffset]|= (1<<EVolumeDownBitOffset);
	iFeatureBitmask[EMuteByteOffset]|= (1<<EMuteBitOffset);
	iFeatureBitmask[EPlayByteOffset]|= (1<<EPlayBitOffset);
	iFeatureBitmask[EStopByteOffset]|= (1<<EStopBitOffset);
	iFeatureBitmask[EPauseByteOffset]|= (1<<EPauseBitOffset);
	iFeatureBitmask[ERecordByteOffset]|= (1<<ERecordBitOffset);
	iFeatureBitmask[ERewindByteOffset]|= (1<<ERewindBitOffset);
	iFeatureBitmask[EFastForwardByteOffset]|= (1<<EFastForwardBitOffset);
	iFeatureBitmask[EEjectByteOffset]|= (1<<EEjectBitOffset);
	iFeatureBitmask[EForwardByteOffset]|= (1<<EForwardBitOffset);
	iFeatureBitmask[EBackwardByteOffset]|= (1<<EBackwardBitOffset);
	iFeatureBitmask[EAngleByteOffset]|= (1<<EAngleBitOffset);
	iFeatureBitmask[ESubpictureByteOffset]|= (1<<ESubpictureBitOffset);
	iFeatureBitmask[EF1ByteOffset]|= (1<<EF1BitOffset);
	iFeatureBitmask[EF2ByteOffset]|= (1<<EF2BitOffset);
	iFeatureBitmask[EF3ByteOffset]|= (1<<EF3BitOffset);
	iFeatureBitmask[EF4ByteOffset]|= (1<<EF4BitOffset);
	iFeatureBitmask[EF5ByteOffset]|= (1<<EF5BitOffset);
	}

void TPlayerFeatureBitmask::SetAbsoluteVolumeApiFeatures(const RArray<TUint>& aOperation)
	{
	if(aOperation.Count())
		{
		iFeatureBitmask[EAdvancedControlPlayerByteOffset] |= (1<<EAdvancedControlPlayerBitOffset);
		}
	}

void TPlayerFeatureBitmask::SetGroupNavigationApiFeatures(const RArray<TUint>& aOperation)
	{
	if(aOperation.Count())
		{
		iFeatureBitmask[EVendorUniqueByteOffset] |= (1<<EVendorUniqueBitOffset);
		iFeatureBitmask[EGroupNavigationByteOffset] |=  (1<<EGroupNavigationBitOffset);
		}
	}

void TPlayerFeatureBitmask::SetNowPlayingApiFeatures(const RArray<TUint>& aOperation)
	{
	if(aOperation.Count())
		{
		iFeatureBitmask[EAdvancedControlPlayerByteOffset] |= (1<<EAdvancedControlPlayerBitOffset);
		iFeatureBitmask[ENowPlayingByteOffset] |= (1<<ENowPlayingBitOffset);
		
		if(aOperation.Find(EAddToNowPlaying) >= 0)
			{
			iFeatureBitmask[EAddToNowPlayingByteOffset] |= (1<<EAddToNowPlayingBitOffset);
			}
		}
	}

void TPlayerFeatureBitmask::SetMediaBrowseApiFeatures(const RArray<TUint>& aOperation)
	{
	if(aOperation.Count())
		{
		iFeatureBitmask[EAdvancedControlPlayerByteOffset] |= (1<<EAdvancedControlPlayerBitOffset);
		iFeatureBitmask[ENowPlayingByteOffset] |= (1<<ENowPlayingBitOffset);
		iFeatureBitmask[EBrowsingByteOffset] |= (1<<EBrowsingBitOffset);

		if(aOperation.Find(ESearchOperationId) >= 0)
			{
			iFeatureBitmask[ESearchByteOffset] |= (1<<ESearchBitOffset);
			}
		
		if(aOperation.Find(EUIDPersistency) >= 0)
			{
			iFeatureBitmask[EUidPersistencyByteOffset] |= (1<<EUidPersistencyBitOffset);
			iFeatureBitmask[EDatabaseAwareByteOffset] |= (1<<EDatabaseAwareBitOffset);
			}
		}
	}


