/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bip controller class declaration.
*
*/


#ifndef _BIPCONTROLLER_H
#define _BIPCONTROLLER_H

//INCLUDES

#include <e32base.h>
#include <obex.h>
#include <obexutilsmessagehandler.h>
#include "obexutilspropertynotifier.h"
#include "debug.h"
#include <SrcsInterface.h>
#include <obexutilspropertynotifier.h>
#include <btengsettings.h>
#include "btengdevman.h"
#include <obexutilsdialog.h>
#include <hbdevicedialogsymbian.h>
#include <hbsymbianvariant.h>

// FORWARD DECLARATIONS

class CBIPCapabilityHandler;
class CBIPImageHandler;

/**
* Backup status.
* The value is controlled by FileManager
*/
enum TFileManagerBkupStatusType
    {
    EFileManagerBkupStatusUnset   = 0x00000000,
    EFileManagerBkupStatusBackup  = 0x00000001,
    EFileManagerBkupStatusRestore = 0x00000002
    };

const TUid KUidMsgTypeBt                 = {0x10009ED5};

// CLASS DECLARATION

/**
*  The main controller for Basic Imaging Profile.
*/
NONSHARABLE_CLASS (CBIPController): public CSrcsInterface, public MObexServerNotify,
                                    public MObexUtilsPropertyNotifyHandler, 
                                    public MObexUtilsDialogObserver,
                                    public MBTEngDevManObserver,
                                    public MHbDeviceDialogObserver
    {
public:
    static CBIPController* NewL();
    ~CBIPController();
    
private: // from CSrcsInterface
    TBool IsOBEXActive();
    void SetMediaType(TSrcsMediaType aMediaType);
    TInt SetObexServer(CObexServer* aServer);

private: // from MObexServerNotify
    void ErrorIndication(TInt aError);
    void TransportUpIndication();
    void TransportDownIndication();
    TInt ObexConnectIndication(const TObexConnectInfo& aRemoteInfo, const TDesC8& aInfo);
    void ObexDisconnectIndication(const TDesC8& aInfo);
    CObexBufObject* PutRequestIndication();
    TInt PutPacketIndication();
    TInt PutCompleteIndication();
    CObexBufObject* GetRequestIndication(CObexBaseObject* aRequiredObject);
    TInt GetPacketIndication();
    TInt GetCompleteIndication();
    TInt SetPathIndication(const CObex::TSetPathInfo& aPathInfo, const TDesC8& aInfo);
    void AbortIndication();
    
private: // from MObexUtilsPropertyNotifyHandler
    void HandleNotifyL(TMemoryPropertyCheckType aCheckType);
    
    
private: // from MBTEngDevManObserver
    void HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* aDeviceArray);
    
private: //from MObexUtilsDialogObserver
    void DialogDismissed(TInt aButtonId);
    
private:
    CBIPController();
    void ConstructL();
    
    void CancelTransfer();
    
    void HandleError(TBool aAbort);
    void HandleGetCapabilityRequestL();
    void HandlePutImageRequestL();
    TInt HandlePutCompleteIndication();
    TInt RemoveCapabilityObject();
    
    TBool CheckCapacityL();
    void LaunchReceivingIndicatorL();
    inline TBool ReceivingIndicatorActive() const { return (iDialogActive); }
    void UpdateReceivingIndicatorL();
    void CloseReceivingIndicator(TBool aResetDisplayedState = ETrue);
    TInt GetDriveWithMaximumFreeSpaceL();
    TBool IsBackupRunning();
    TBool ProcessExists( const TSecureId& aSecureId );
    
private:
    void DataReceived(CHbSymbianVariantMap& aData);
    void DeviceDialogClosed(TInt aCompletionCode);
    
private:
    enum TBipTransferState
        {
        ETransferIdle,
        ETransferPut,
        ETransferGet,
        ETransferPutDiskError,
        ETransferPutInitError,
        ETransferPutCancel,
        };
    
    
private: // Data
    CBIPCapabilityHandler*      iBIPCapabilityHandler;
    CObexBufObject*             iBTObject;
    CObexServer*                iBTObexServer;
    TBipTransferState           iBTTransferState;
    TInt                        iDrive;
    CBIPImageHandler*           iBIPImageHandler;
    CObexBufObject*             iGetObject;
    CObexUtilsPropertyNotifier* iLowMemoryActiveCDrive;
    CObexUtilsPropertyNotifier* iLowMemoryActiveMMC;
    TMsvId                      iMsvIdParent;
    TMsvId                      iMsvIdAttach;
    RFile                       iFile;
    RFs                         iFs;
    TFileName                   iFullPathFilename;
    TFileName                   iCapabilityFileName;
    TFileName                   iDefaultFolder;
    TFileName                   iPreviousDefaultFolder;
    CBufFlat                    *iBuf;
    TBool                       iLengthHeaderReceived;
    TInt                        iTotalSizeByte;
    TBool                       iNoteDisplayed;
    CBTEngDevMan*               iDevMan;
    CBTDeviceArray*             iResultArray;
    TBTDeviceName               iRemoteDeviceName;
    TFileName                   iReceivingFileName;
    TFileName                   iCenRepFolder;
    CObexUtilsDialog*           iDialog;
    CHbDeviceDialogSymbian*     iProgressDialog;
    TBool                       iDialogActive;
    TInt                        iFileCount;
    TBool                       iReceivingFailed;
    CHbDeviceDialogSymbian*     iRecvDoneDialog;
    TBool                       iShowRecvCompleteDialog;
    };
    
_LIT(KBipPanicCategory, "BIP");
enum TBipPanicCode
    {
    EBipPanicNotBluetoothMediaType = 0,
    };

#endif      //_BIPCONTROLLER_H

// End of File
