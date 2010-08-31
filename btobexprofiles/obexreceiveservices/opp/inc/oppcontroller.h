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
* Description: 
*     Class handles Bluetooth OPP reception.
*
*
*/


#ifndef OPPCONTROLLER_H
#define OPPCONTROLLER_H

//  INCLUDES


#include <e32base.h>
#include <obexutilsmessagehandler.h>
#include "obexutilspropertynotifier.h"
#include <SrcsInterface.h>
#include "btengdevman.h"

#include <obexutilspropertynotifier.h>

#include <obexutilsdialog.h>
#include <hbdevicedialogsymbian.h>
#include <hbsymbianvariant.h>


const TInt KBtStartReserveChannel   = 9;
const TInt KBtEndReserveChannel     = 30;

// todo @ QT migration: take official definition from Messaging at app layer (btmsgtypeuid.h)
const TUid KUidMsgTypeBt                 = {0x10009ED5};
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
/**
*  CBtListenActive
*  Class to implement IrObex permanent listen
*/
class COPPController : public CSrcsInterface, public MObexServerNotify, 
                       public MObexUtilsPropertyNotifyHandler,
                       public MObexUtilsDialogObserver,
                       public MBTEngDevManObserver,
                       public MHbDeviceDialogObserver     
    {
public:
    static COPPController* NewL();
    ~COPPController();   

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
    COPPController();
    void ConstructL();
    
    void CancelTransfer();
    void HandlePutRequestL();
    TInt HandlePutCompleteIndication();
    void HandleError(TBool aAbort);
    
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
    enum TObexTransferState
        {
        ETransferIdle,
        ETransferPut,
        ETransferPutDiskError,
        ETransferPutInitError,
        ETransferPutCancel,
        };

private:
    CObexServer*                iObexServer;
    TObexTransferState          iObexTransferState;
    CObexBufObject*             iObexObject;
    TInt                        iDrive;
    TBool                       iListening;
    CObexUtilsPropertyNotifier* iLowMemoryActiveCDrive;
    CObexUtilsPropertyNotifier* iLowMemoryActiveMMC;
    TMsvId                      iMsvIdParent;
    TMsvId                      iMsvIdAttach;
    TFileName                   iFullPathFilename;
    TFileName                   iDefaultFolder;
    TFileName                   iPreviousDefaultFolder;
    TFileName                   iCenRepFolder;
    RFs                         iFs;
    RFile                       iFile;
    CBufFlat*                   iBuf;
    TBool                       iLengthHeaderReceived;
    TSrcsMediaType              iMediaType;
    TInt                        iTotalSizeByte;
    TFileName                   iReceivingFileName;
    TBool                       iNoteDisplayed;
    CBTEngDevMan*               iDevMan;
    CBTDeviceArray*             iResultArray;
    TBTDeviceName               iRemoteDeviceName;
    CObexUtilsDialog*           iDialog;
    CHbDeviceDialogSymbian*     iProgressDialog;
    TBool                       iDialogActive;
    TInt                        iFileCount;
    TBool                       iReceivingFailed;
    CHbDeviceDialogSymbian*     iRecvDoneDialog;
    TBool                       iShowRecvCompleteDialog;
    };

#endif      // OPPCONTROLLER_H
            
// End of File
