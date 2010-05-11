// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#ifndef AVRCPUTILS_H
#define AVRCPUTILS_H

/**
@file
@internalComponent
@released
*/

_LIT(KAvrcpPanicName, "AVRCP Panic");

enum TAvrcpPanic
	{
	EAvrcpNoAVCFrame = 0,
	EAvrcpNotConnected = 1,
	EAvrcpNoOutstandingSend = 2,
	EAvrcpHoldExpiryForRelease = 3,
	EAvrcpReleaseExpiryForRelease = 4,
	EAvrcpUnknownAvctpTransId = 5,
	EAvrcpNoResponsesAvailable = 6,
	EAvrcpResponseToUnknownCommand = 7,
	EAvrcpBadBTAddr = 8,
	EAvrcpCommandStillInUse = 9,
	EAvrcpCommandDataTooLong = 10,
	EAvrcpFunnyLengthData = 11,
	EAvrcpTimerNotCancelled = 12,
	EAvrcpNotVendorDependent = 13,
	EAvrcpMismatchedConnectDisconnect = 14,
	EAvrcpDummyCallbackCalled = 15,
	EAvrcpUnknownButtonAction = 16,
	EAvrcpPressHasPhantomResponse = 17,
	EAvrcpReleaseTimerStartedWithoutResponse = 18,
	EAvrcpReleaseExpiryForOldCommand = 19,
	EAvrcpSendingMessageFailed = 20,
	EAvrcpDisconnectRequestWhilePartiallyConstructed = 21,
	EAvrcpReleaseExpiryForClick = 22,
	EAvrcpIncomingCommandsNotHandled = 23,
	EAvrcpCommandStillQueuedForHandling = 24,
	EAvrcpCommandStillQueuedAsReady = 25,
	EAvrcpCommandStillQueuedForSending = 26,
	EAvrcpPressNotPreviousPassthroughCommand = 27,
	EAvrcpInterfaceUidNotSet = 28,
	EAvrcpCTypeNotSet = 29,
	EAvrcpCompanyIDNotSet = 30,
	EAvrcpNotFullyConstructed = 31,
	EAvrcpInternalHandlingRequestedOnWrongInterface = 32,
	EAvrcpVolumeBeyondMaxVolume = 33,
	EAvrcpInvalidEventId = 34,
	EAvrcpInvalidPlayerId = 35,
	EAvrcpConnectConfirmOnBrowseChannel = 36,
	EAvrcpDisconnectConfirmOnBrowseChannel = 37,
	EAlreadyWatchingPlayer = 38,
	ENotWatchingPlayer = 39,
	EAvctpMessageTypeRequestedForInternalCommand = 40,
	ELowerInterfaceUsedOnInternalHandler = 41,
	EFurtherProcessingRequiredForNonSetBrowsedPlayer = 42,
	ESetBrowsePlayerRequestCorruptedLocally = 43,
	ESpecificAddressUsedForBrowsingCommand = 44,
	EUidUpdateRequestWriteFailure = 45,
	EUnmatchedResponseFromRemCon = 46,
	EResponseForWrongInterface = 47,
	EInvalidBtAddrInResponse = 48,
	EInvalidPlayerId = 49,
	EMysteryInternalResponse = 50,
	ECommandAlreadyQueuedForSending = 52,
	EPassthroughQueueNotEmptyAfterDisconnect = 53,
	EPreviousPassthroughNonNullReplacing = 54,
	ETooManyRemotesRegisterForLocalAddressedPlayerUpdates = 55,
	ETooFewRemotesRegisterForLocalAddressedPlayerUpdates = 56,
	};

template <typename XAny>
struct TAvrcpPanicCodeTypeChecker
	{
	inline static void Check(XAny) { }
	};

#define AVRCP_PANIC(CODE) \
	TAvrcpPanicCodeTypeChecker<TAvrcpPanic>::Check(CODE), \
	PANIC(KAvrcpPanicName, CODE)


class TBTDevAddr;
class TRemConAddress;
class AvrcpUtils
	{
public:
	static void Panic(TAvrcpPanic aPanic); // The macro is probably more useful so this can be removed
		
	static void SetCommandDataFromInt(RBuf8& aCommandData, TInt aOffset, TInt aLength, TInt aValue);
	static void ReadCommandDataToInt(const RBuf8& aCommandData, TInt aOffset, TInt aLength, TInt& aValue);
	
	static TInt RemConToBTAddr(const TRemConAddress& aRemoteAddress, TBTDevAddr& aBTAddr);
	static void BTToRemConAddr(const TBTDevAddr& aBTAddr, TRemConAddress& aRemConAddress);
	};

//
// Thread Assertions
//

enum TAvrcpThreadId
	{
	EControlThread	= 0x00000001,
	EBulkThread		= 0x00000002,
	};
	
#define ASSERT_THREAD(aaa) ASSERT_DEBUG(reinterpret_cast<TUint32>(Dll::Tls()) == aaa)
#define WEAK_ASSERT_THREAD(aaa) if(Dll::Tls()) { ASSERT_THREAD(aaa); }
#define ASSERT_CONTROL_THREAD ASSERT_THREAD(EControlThread)
#define ASSERT_BULK_THREAD ASSERT_THREAD(EBulkThread)
#define WEAK_ASSERT_BULK_THREAD WEAK_ASSERT_THREAD(EBulkThread)

//
// Cleanup Stack utils
//

template <class T>
class CleanupSignal
	{
public:
	inline static void PushL(T& aRef) {CleanupStack::PushL(TCleanupItem(&Signal,&aRef));};
private:
	static void Signal(TAny *aPtr) {(static_cast<T*>(aPtr))->Signal();};
	};

template <class T>
inline void CleanupSignalPushL(T& aRef)
	{CleanupSignal<T>::PushL(aRef);}

template <class T>
class CleanupDeleteAndNull
	{
public:
	inline static void PushL(T*& aRef) {CleanupStack::PushL(TCleanupItem(&DeleteAndNull,&aRef));};
private:
	static void DeleteAndNull(TAny *aPtr) {T*& ptr = *static_cast<T**>(aPtr); delete ptr; ptr = NULL;};
	};

template <class T>
inline void CleanupDeleteAndNullPushL(T*& aRef)
	{CleanupDeleteAndNull<T>::PushL(aRef);}

class CSpecificThreadCallBackBody;
NONSHARABLE_CLASS(RSpecificThreadCallBack)
	{
public:
	RSpecificThreadCallBack();
	
	TInt Create(const TCallBack& aCallBack, TInt aPriority);
	void Close();
	
	TInt Start();
	TInt CallBack();
	void Cancel();
	
private:
	CSpecificThreadCallBackBody*	iBody;
	};

/**
An extended double queue link class to provide additional features.
*/
NONSHARABLE_CLASS(TAvrcpDblQueLink) : public TDblQueLink
	{
	public:
		inline TBool IsQueued() const;
	};

/**
Indicates whether the queue link is attached to a queue.
@return True if the link is queued, false otherwise.
*/
inline TBool TAvrcpDblQueLink::IsQueued() const
	{
	return iNext ? ETrue : EFalse;
	}

#endif // AVRCPUTILS_H
