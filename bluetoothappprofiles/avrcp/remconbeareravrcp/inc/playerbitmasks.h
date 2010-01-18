// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// TPlayerFeatureBitmask.h
//

#ifndef PLAYERBITMASKS_H_
#define PLAYERBITMASKS_H_

/**
@file
@internalComponent
@released
*/
#include <e32base.h>

const TInt KFeatureBitmaskLength = 16;

enum TByteOffsets
	{
	ESelectByteOffset = 0,
	EUpByteOffset = 0,
	EDownByteOffset = 0,
	ELeftByteOffset = 0,
	ERightByteOffset = 0,
	ERightUpByteOffset = 0,
	ERightDownByteOffset = 0,
	ELeftUpByteOffset = 0,
	ELeftDownByteOffset = 1,
	ERootMenuByteOffset = 1,
	ESetupMenuByteOffset = 1,
	EContentsMenuByteOffset = 1,
	EFavoriteMenuByteOffset = 1,
	EExitByteOffset = 1,
	E0ByteOffset = 1,
	E1ByteOffset = 1,
	E2ByteOffset = 2,
	E3ByteOffset = 2,
	E4ByteOffset = 2,
	E5ByteOffset = 2,
	E6ByteOffset = 2,
	E7ByteOffset = 2,
	E8ByteOffset = 2,
	E9ByteOffset = 2,
	EDotByteOffset = 3,
	EEnterByteOffset = 3,
	EClearByteOffset = 3,
	EChannelUpByteOffset = 3,
	EChannelDownByteOffset = 3,
	EPreviousChannelByteOffset = 3,
	ESoundSelectByteOffset = 3,
	EInputSelectByteOffset = 3,
	EDisplayInformationByteOffset = 4,
	EHelpByteOffset = 4,
	EPageUpByteOffset = 4,
	EPageDownByteOffset = 4,
	EPowerByteOffset = 4,
	EVolumeUpByteOffset = 4,
	EVolumeDownByteOffset = 4,
	EMuteByteOffset = 4,
	EPlayByteOffset = 5,
	EStopByteOffset = 5,
	EPauseByteOffset = 5,
	ERecordByteOffset = 5,
	ERewindByteOffset = 5,
	EFastForwardByteOffset = 5,
	EEjectByteOffset = 5,
	EForwardByteOffset = 5,
	EBackwardByteOffset = 6,
	EAngleByteOffset = 6,
	ESubpictureByteOffset = 6,
	EF1ByteOffset = 6,
	EF2ByteOffset = 6,
	EF3ByteOffset = 6,
	EF4ByteOffset = 6,
	EF5ByteOffset = 6,
	EVendorUniqueByteOffset = 7,
	EGroupNavigationByteOffset = 7,
	EAdvancedControlPlayerByteOffset = 7,
	EBrowsingByteOffset = 7,
	ESearchByteOffset = 7,
	EAddToNowPlayingByteOffset = 7,
	EDatabaseAwareByteOffset = 7,
	EBrowsableOnlyWhenAddressedByteOffset = 7,
	ESearchableOnlyWhenAddressedByteOffset = 8,
	ENowPlayingByteOffset = 8,
	EUidPersistencyByteOffset = 8,
	};

enum TBitOffset
	{
	ESelectBitOffset = 0,
	EUpBitOffset = 1,
	EDownBitOffset = 2,
	ELeftBitOffset = 3,
	ERightBitOffset = 4,
	ERightUpBitOffset = 5,
	ERightDownBitOffset = 6,
	ELeftUpBitOffset = 7,
	ELeftDownBitOffset = 0,
	ERootMenuBitOffset = 1,
	ESetupMenuBitOffset = 2,
	EContentsMenuBitOffset = 3,
	EFavoriteMenuBitOffset = 4,
	EExitBitOffset = 5,
	E0BitOffset = 6,
	E1BitOffset = 7,
	E2BitOffset = 0,
	E3BitOffset = 1,
	E4BitOffset = 2,
	E5BitOffset = 3,
	E6BitOffset = 4,
	E7BitOffset = 5,
	E8BitOffset = 6,
	E9BitOffset = 7,
	EDotBitOffset = 0,
	EEnterBitOffset = 1,
	EClearBitOffset = 2,
	EChannelUpBitOffset = 3,
	EChannelDownBitOffset = 4,
	EPreviousChannelBitOffset = 5,
	ESoundSelectBitOffset = 6,
	EInputSelectBitOffset = 7,
	EDisplayInformationBitOffset = 0,
	EHelpBitOffset = 1,
	EPageUpBitOffset = 2,
	EPageDownBitOffset = 3,
	EPowerBitOffset = 4,
	EVolumeUpBitOffset = 5,
	EVolumeDownBitOffset = 6,
	EMuteBitOffset = 7,
	EPlayBitOffset = 0,
	EStopBitOffset = 1,
	EPauseBitOffset = 2,
	ERecordBitOffset = 3,
	ERewindBitOffset = 4,
	EFastForwardBitOffset = 5,
	EEjectBitOffset = 6,
	EForwardBitOffset = 7,
	EBackwardBitOffset = 0,
	EAngleBitOffset = 1,
	ESubpictureBitOffset = 2,
	EF1BitOffset = 3,
	EF2BitOffset = 4,
	EF3BitOffset = 5,
	EF4BitOffset = 6,
	EF5BitOffset = 7,
	EVendorUniqueBitOffset = 0,
	EGroupNavigationBitOffset = 1,
	EAdvancedControlPlayerBitOffset = 2,
	EBrowsingBitOffset = 3,
	ESearchBitOffset = 4,
	EAddToNowPlayingBitOffset = 5,
	EDatabaseAwareBitOffset = 6,
	EBrowsableOnlyWhenAddressedBitOffset = 7,
	ESearchableOnlyWhenAddressedBitOffset = 0,
	ENowPlayingBitOffset = 1,
	EUidPersistencyBitOffset = 2,
	};

class TPlayerFeatureBitmask
	{
public:
	TPlayerFeatureBitmask();
	TPlayerFeatureBitmask(const TPlayerFeatureBitmask& aFeatureBitmask);
	
	inline const TDesC8& FeatureBitmask() const;

	void SetCoreApiFeatures(RArray<TUint> aOperation);
	void SetAbsoluteVolumeApiFeatures(RArray<TUint> aOperation);
	void SetGroupNavigationApiFeatures(RArray<TUint> aOperation);
	void SetNowPlayingApiFeatures(RArray<TUint> aOperation);
	void SetMediaBrowseApiFeatures(RArray<TUint> aOperation);

private:
	void SetCoreApiFeatures();

private:
	TBuf8<KFeatureBitmaskLength> iFeatureBitmask;
	};

const TDesC8& TPlayerFeatureBitmask::FeatureBitmask() const
	{
	return iFeatureBitmask;
	}
#endif /*PLAYERBITMASKS_H_*/
