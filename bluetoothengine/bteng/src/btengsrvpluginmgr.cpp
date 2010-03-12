/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Helper class for BTEng server to manage and interface with
*                the profile plug-ins the profile plug-ins.
*
*/



#include <ecom/ecom.h>
#include <btsdp.h>
#include <centralrepository.h>
#include <featmgr.h>

#include "btengsrvpluginmgr.h"
#include "btengserver.h"
#include "btengsrvsession.h"
#include "btengsrvbbconnectionmgr.h"
#include "btengprivatecrkeys.h"
#include "debug.h"
#include <btfeaturescfg.h>

/**  UID for BTEng plug-ins */
const TUid KBTEngPluginUid = { 0x2000277B };
/**  ECOM registration info for BTSAP plugin */
_LIT8( KEComBTSapPlugin, "112D" );
/**  The message argument which holds the Bluetooth address. */
const TInt KBTEngAddrSlot = 0;
/**  The message argument which holds the connection status parameter. */
const TInt KBTEngParamSlot = 1;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSrvPluginMgr::CBTEngSrvPluginMgr( CBTEngServer* aServer )
:    iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::ConstructL()
    {
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSrvPluginMgr* CBTEngSrvPluginMgr::NewL( CBTEngServer* aServer )
    {
    CBTEngSrvPluginMgr* self = new( ELeave ) CBTEngSrvPluginMgr( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSrvPluginMgr::~CBTEngSrvPluginMgr()
    {
    iPluginInfoArray.Close();
    iPluginArray.ResetAndDestroy();
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::ProcessCommandL( const RMessage2& aMessage )
    {
    TRACE_FUNC_ENTRY
    TInt opcode = aMessage.Function();
    TInt err = KErrNone;
    TBTDevAddrPckgBuf addr;
    switch( opcode )
        {
        case EBTEngConnectDevice:
            {
            aMessage.ReadL( KBTEngAddrSlot, addr );
            TBTEngDevClassPkg cod;
            aMessage.ReadL( KBTEngParamSlot, cod );
            err = Connect( addr(), TBTDeviceClass( cod() ) );
            }
            break;
        case EBTEngCancelConnectDevice:
            {
            aMessage.ReadL( KBTEngAddrSlot, addr );
            err = CancelConnect( addr() );
            }
            break;
        case EBTEngDisconnectDevice:
            {
            aMessage.ReadL( KBTEngAddrSlot, addr );
            TPckgBuf<TUint> discType;
            aMessage.ReadL( KBTEngParamSlot, discType );
            err = Disconnect( addr(), (TBTDisconnectType) discType() );
            }
            break;
        case EBTEngIsDeviceConnected:
            {
            aMessage.ReadL( KBTEngAddrSlot, addr );
            TBTEngConnectionStatus connectStatus = EBTEngNotConnected;
            connectStatus = IsDeviceConnected( addr() );
            TBTEngParamPkg statusPkg( connectStatus );
            aMessage.WriteL( KBTEngParamSlot, statusPkg );
            }
            break;
        case EBTEngIsDeviceConnectable:
            {
            aMessage.ReadL( KBTEngAddrSlot, addr );
            TBTEngDevClassPkg cod;
            aMessage.ReadL( KBTEngParamSlot, cod );
            TBool connectable = ( GetConnectablePluginIndex(cod(), addr() ) != KErrNotFound ) ;
            TPckgBuf<TBool> connPkg( connectable );
            aMessage.WriteL( 2, connPkg );
            }
            break;
        case EBTEngGetConnectedAddresses:
            {
            TPckgBuf<TBTProfile> profile;
            aMessage.ReadL( KBTEngParamSlot, profile );
            RBTDevAddrArray addrArray;
            if( profile() == EBTProfileUndefined )
                {
                    // Get all baseband connections
                iServer->iBBConnMgr->GetConnectedAddressesL( addrArray );
                }
            else
                {
                err = GetConnectedAddresses( addrArray, profile() );
                }
            HBufC8* buf = HBufC8::NewLC( aMessage.GetDesMaxLengthL( KBTEngAddrSlot ) );
            TPtr8 ptr = buf->Des();
            aMessage.ReadL( KBTEngAddrSlot, ptr );
            ptr.Zero();
            for( TInt i = 0; i < addrArray.Count(); i++ )
                {
                addr = addrArray[ i ];
                ptr.Append( addr );
                }
            aMessage.WriteL( KBTEngAddrSlot, ptr );
            CleanupStack::PopAndDestroy( buf );
            }
            break;
        default:
            {
            TRACE_INFO( ( _L( "ProcessCommandL: bad request (%d)" ), 
                           aMessage.Function() ) )
            err = KErrArgument;
            }
            break;
        }
    User::LeaveIfError( err );
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::DisconnectAllPlugins()
    {
    TRACE_FUNC_ENTRY
    TBTDevAddr nullAddr;
    for( TInt i = 0; i < iPluginArray.Count(); i++ )
        {
        (void) iPluginArray[ i ]->Disconnect( nullAddr, EBTDiscImmediate );
        }
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::LoadProfilePluginsL( const TEComResolverParams aParams )
    {
    TRACE_FUNC_ENTRY
    if( aParams.DataType().Length() )
        {
            // This is a request to enable a specific service, e.g. BT SAP.
        REComSession::ListImplementationsL( KBTEngPluginUid, aParams, 
                                             iPluginInfoArray );
        }
    else
        {
        if( iPluginArray.Count() > 0 || iPluginInfoArray.Count() > 0 )
            {
                // Could be the case if we received a command to turn BT on 
                // halfway through a power down sequence. Just ignore.
            return;
            }
        iPluginInfoArray.Reset();
        iPluginArray.Reset();
        REComSession::ListImplementationsL( KBTEngPluginUid, iPluginInfoArray );
        }
        // Ignore the number of plug-ins left to load; the server state machine 
        // will handle this at a later stage.
    (void) LoadPluginL();
    TRACE_FUNC_EXIT
    }

TBool CBTEngSrvPluginMgr::FilterByEnterpriseDisablementModeL(TUid aUid) const
	{
	TRACE_FUNC_ENTRY
	TBool want = EFalse;
	switch ( BluetoothFeatures::EnterpriseEnablementL() )
		{
	case BluetoothFeatures::EDisabled:
		// In Disabled mode all plugins are filtered out.
		break;
	case BluetoothFeatures::EDataProfilesDisabled:
		// In 'privileged profiles only' mode we only allow the following.
		if ( 		aUid == TUid::Uid(0x1020897B) // audio (i.e. allow HSP, HFP and A2DP)
				||	aUid == TUid::Uid(0x10208979) // remote control (i.e. allow AVRCP) )
				||	aUid == TUid::Uid(0x2001E309) // HID
			)
			{
			want = ETrue;
			}
		break;
	case BluetoothFeatures::EEnabled:
		// In Enabled mode we do not filter plugins.
		want = ETrue;
		break;
	default:
		__ASSERT_DEBUG( NULL, PanicServer( EBTEngPanicCorruptSettings ) );
		break;
		}
	TRACE_INFO( ( _L( "[BTENG]\t returning want = %d" ), want ) )
	TRACE_FUNC_EXIT
	return want;
	}

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::LoadPluginL()
    {
    TRACE_FUNC_ENTRY
    if( !iPluginInfoArray.Count() )
        {
            // All plug-ins have been loaded.
        return KErrNotFound;
        }

        // Simply pop the first info object, process it, and delete it.
        // There is no need to keep it after the plug-in has been constructed.
    CImplementationInformation* implInfo = iPluginInfoArray[ 0 ];
    iPluginInfoArray.Remove( 0 );
    CleanupStack::PushL( implInfo );
    TLex8 lex( implInfo->DataType() );
    TUint profile = (TInt) EBTProfileUndefined;
    TInt err = lex.Val( profile, EHex );
        // Check if the feature is allowed to be loaded
    if( !err && CheckFeatureEnabledL( profile ) && FilterByEnterpriseDisablementModeL(implInfo->ImplementationUid()) )
        {
        TRACE_INFO( ( _L( "[BTENG]\t loading profile 0x%04x" ), profile ) )
        TUid implUid = implInfo->ImplementationUid();
        CBTEngPlugin* plugin = CBTEngPlugin::NewL( implUid );
        CleanupStack::PushL( plugin );
        plugin->SetObserver( ( MBTEngPluginObserver* ) this );
        User::LeaveIfError( iPluginArray.Append( plugin ) );
        CleanupStack::Pop( plugin );
        }
    CleanupStack::PopAndDestroy( implInfo ); 
    TRACE_FUNC_RES( ( _L( "%d plug-ins left to load" ), iPluginInfoArray.Count() ) )
    return iPluginInfoArray.Count();
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::UnloadProfilePlugins()
    {
    TRACE_FUNC_ARG( ( _L( "[BTENG]\t unloading %d plug-ins" ), iPluginArray.Count() ) )

    // All plug-ins need to be unloaded at once. Otherwise it gets too
    // difficult to keep track in case of a power-on command during a 
    // power-off sequence.
    iPluginArray.ResetAndDestroy();

    REComSession::FinalClose();
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::LoadBTSapPluginL()
    {
    TRACE_FUNC_ENTRY

	// SAP is supported in neither Data Profiles Disabled nor Disabled mode.
    if ( BluetoothFeatures::EnterpriseEnablementL() != BluetoothFeatures::EEnabled )
        {
        TRACE_INFO( ( _L( "\tno we're not... Bluetooth is enterprise-IT-disabled" ) ) )
        User::Leave(KErrNotSupported);
        }
    
    if( CheckFeatureEnabledL( EBTProfileSAP ) )
        {
        TEComResolverParams params;
        TPtrC8 ptr( KEComBTSapPlugin );
        params.SetDataType( ptr );
        LoadProfilePluginsL( params );
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::UnloadBTSapPlugin()
    {
    TRACE_FUNC_ENTRY
    TInt ret = GetFirstPluginOfProfileSupported( EBTProfileSAP );
    if( ret != KErrNotFound )
        {
        CBTEngPlugin* plugin = iPluginArray[ ret ];
        iPluginArray.Remove( ret );
        delete plugin;
        }
    
    TRACE_FUNC_EXIT
    }
    
// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::DisconnectProfile( TBTProfile aProfile )
    {
    TRACE_FUNC_ENTRY
    RBTDevAddrArray addrArray;
    TInt count = iPluginArray.Count();
    while( count )
        {
        count--;
        if( iPluginArray[ count ]->IsProfileSupported( aProfile ) )
            {
            iPluginArray[ count ]->GetConnections( addrArray, aProfile );
            for ( TInt i = 0;  i < addrArray.Count(); i++ )
                {
                iPluginArray[ count ]->Disconnect( addrArray[i], EBTDiscImmediate );
                }
            break;
            }
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MBTEngPluginObserver.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::ConnectComplete( const TBTDevAddr& aAddr, 
    TBTProfile aProfile, TInt aErr, RBTDevAddrArray* aConflicts )
    {
    TRACE_FUNC_ENTRY
        // Inform listeners of this event.
    (void) aProfile;
    iServer->iSessionIter.SetToFirst();
    CBTEngSrvSession* session = (CBTEngSrvSession*) iServer->iSessionIter++;
    while( session )
        {
        TRACE_INFO( ( _L( "[BTEng]\t Notifying session %d" ), (TInt) session ) )
        session->NotifyConnectionEvent( aAddr, EBTEngConnected, aConflicts, aErr );
        session = (CBTEngSrvSession*) iServer->iSessionIter++;
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MBTEngPluginObserver.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvPluginMgr::DisconnectComplete( const TBTDevAddr& aAddr, 
    TBTProfile aProfile, TInt aErr )
    {
    TRACE_FUNC_ENTRY
        // Inform listeners of this event.
    (void) aProfile;
    iServer->iSessionIter.SetToFirst();
    CBTEngSrvSession* session = (CBTEngSrvSession*) iServer->iSessionIter++;
    while( session )
        {
        TRACE_INFO( ( _L( "[BTEng]\t Notifying session %d" ), (TInt) session ) )
        session->NotifyConnectionEvent( aAddr, EBTEngNotConnected, NULL, aErr );
        session = (CBTEngSrvSession*) iServer->iSessionIter++;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::Connect( const TBTDevAddr& aAddr, 
    const TBTDeviceClass& aDeviceClass )
    {
    TRACE_FUNC_ENTRY
    TRACE_BDADDR ( aAddr )
    TInt pindex = GetConnectablePluginIndex( aDeviceClass );    
    TRACE_INFO( ( _L( "[BTEng]\t The %dth of plugin in plugarray to connect" ), pindex ) )
    TInt err (KErrNotFound);
    if( pindex != KErrNotFound && iPluginArray.Count())
        {
        err = iPluginArray[ pindex ]->Connect( aAddr );
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::CancelConnect( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNotFound;
    TRACE_BDADDR(aAddr);
    for( TInt i = 0; i < iPluginArray.Count(); i++ )
        {
        TBTEngConnectionStatus status = iPluginArray[ i ]->IsConnected( aAddr );
        if( status == EBTEngConnecting || status == EBTEngConnected )
            {
            (void) iPluginArray[ i ]->CancelConnect( aAddr );
            err = KErrNone;
            break;
            }
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::Disconnect( const TBTDevAddr& aAddr, 
    TBTDisconnectType aDiscType )
    {
    TRACE_FUNC_ENTRY
    TRACE_BDADDR(aAddr);
    TInt err = KErrNotFound;
    for( TInt i = 0; i < iPluginArray.Count(); i++ )
        {
            // Should be ignored if the plug-in does not have 
            // a connection to the address.
        err = iPluginArray[ i ]->Disconnect( aAddr, aDiscType );
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), err ) )
    return err;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TBTEngConnectionStatus CBTEngSrvPluginMgr::IsDeviceConnected( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    TRACE_BDADDR(aAddr);
    TBTEngConnectionStatus status = EBTEngNotConnected;
    for( TInt i = 0; i < iPluginArray.Count(); i++ )
        {
        status = iPluginArray[ i ]->IsConnected( aAddr );
        if( status == EBTEngConnecting || status == EBTEngConnected )
            {
            break;  // Just exit the loop here, we have a connection status.
            }
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), (TInt) status ) )
    return status;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::GetConnectablePluginIndex( const TBTDeviceClass& aDeviceClass, const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    TInt plugindex( KErrNotFound );
    TRACE_BDADDR(aAddr);
    TRACE_INFO ( (_L("[BTENG] cod %b"), aDeviceClass.DeviceClass()))
    if( aAddr != TBTDevAddr() )
        {
        DoGetEirData( aAddr );
        }
    
    if( iUuidContainter.UUIDs().Count() > 0)
        {
        plugindex = GetConnectablePluginIndexByEir();
        }
    
    if ( plugindex == KErrNotFound )
        {
        TBTProfile profile = MapDeviceClassToProfile( aDeviceClass );
        if( profile == EBTProfileUndefined )
            {
            return plugindex;
            }
        
        plugindex = GetFirstPluginOfProfileSupported( profile );
        
        if ( !iPluginArray.Count() )
            {
            // In case BT is off and plugins are not loaded
            if ( profile == EBTProfileHFP || profile == EBTProfileA2DP )
                {
                plugindex = KErrNone; 
                }
            }
        }
    
    TRACE_FUNC_RES( ( _L( "result: %d" ), plugindex ) )
    return plugindex;
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::GetConnectedAddresses( RBTDevAddrArray& aAddrArray, 
    TBTProfile aProfile )
    {
    TRACE_FUNC_ENTRY    
    TInt ret = GetFirstPluginOfProfileSupported( aProfile );
    if( ret != KErrNotFound )
        {
        iPluginArray[ ret ]->GetConnections( aAddrArray, aProfile );
        ret = KErrNone;
        }
    TRACE_FUNC_RES( ( _L( "result: %d" ), ret ) )
    return ret;
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TBool CBTEngSrvPluginMgr::CheckFeatureEnabledL( TInt aProfile )
    {
    TRACE_FUNC_ARG( ( _L( "requested feature: 0x%04x" ), aProfile ) )
        // By default, a feature is supported. This allowd features that do not
        // have a related feature flag to be loaded too.
    TBool supported = ETrue;
    if( aProfile == EBTProfileSAP )
        {
            // First check from central repository.
        CRepository* cenrep = CRepository::NewL( KCRUidBTEngPrivateSettings );
        TInt enabled = 0;
        TInt err = cenrep->Get( KBTSapEnabled, enabled );
        delete cenrep;
        if( err || !enabled )
            {
            return EFalse;
            }
        }

    TInt feature = MapProfileToFeature( aProfile );
    if( feature )
        {
            // Check from feature manager if this phone enables this feature.
        FeatureManager::InitializeLibL();
        supported = FeatureManager::FeatureSupported( feature );
        FeatureManager::UnInitializeLib();
        }
    TRACE_FUNC_RES( ( _L ( "result: %d" ), supported ) )
    return supported;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TBTProfile CBTEngSrvPluginMgr::MapDeviceClassToProfile( 
    const TBTDeviceClass& aDeviceClass )
    {
    TRACE_FUNC_ARG( ( _L( "Mapping CoD %b ..." ), aDeviceClass.DeviceClass() ) )
    // Could (should?) be done more dynamically or with some header file definition.
    // Right now these are the only known/possible ECOM plug-ins.
    TBTProfile profile = EBTProfileUndefined;
    if( aDeviceClass.MajorServiceClass() & EMajorServiceAudio )
        {
        profile = EBTProfileHFP;
        }
    else if( aDeviceClass.MajorServiceClass() & EMajorServiceRendering &&
            ( aDeviceClass.MajorDeviceClass() != EMajorDeviceImaging ) )
        // Printer or camera or other imaging device may set EMajorServiceRendering bit
        // as well as stereo audio device, so check EMajorDeviceImaging too.
        {
        profile = EBTProfileA2DP;
        }
    else if( aDeviceClass.MajorDeviceClass() == EMajorDevicePeripheral &&
            ( (aDeviceClass.MinorDeviceClass() & EMinorDevicePeripheralKeyboard) ||
              (aDeviceClass.MinorDeviceClass() & EMinorDevicePeripheralPointer) ) )
        {
        profile = EBTProfileHID;
        }
    else if( aDeviceClass.MajorDeviceClass() == EMajorDeviceLanAccessPoint )
        {
            // Mainly for testing now; PAN profile is a personal favorite.
        profile = EBTProfilePANU;
        }
    
    TRACE_FUNC_RES( ( _L( "... to profile 0x%04x." ), profile ) )
    return profile;
    }


// ---------------------------------------------------------------------------
// Maps a profile UUID to any Bluetooth-related pflatfom feature flag.
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::MapProfileToFeature( TInt aProfile )
    {
    TInt feature = 0;
    switch( aProfile )
        {
        case EBTProfileHSP:
        case EBTProfileHFP:
            {
            feature = KFeatureIdBtAudio;
            }
            break;
        case EBTProfileA2DP:
            {
            feature = KFeatureIdBtStereoAudio;
            }
            break;
        case EBTProfileSAP:
            {
            feature = KFeatureIdBtSap;
            }
            break;
        case EBTProfileDUN:
            {
            feature = KFeatureIdDialupNetworking;
            }
            break;
        case EBTProfileFAX:
            {
            feature = KFeatureIdBtFaxProfile;
            }
            break;
        case EBTProfilePANU:
        case EBTProfileNAP:
        case EBTProfileGN:
            {
            //feature = KFeatureIdBtPanProfile;
            feature = 0;// Testin'
            }
            break;
        case EBTProfileBIP:
            {
            feature = KFeatureIdBtImagingProfile;
            }
            break;
        case EBTProfileBPP:
            {
            feature = KFeatureIdBtPrintingProfile;
            }
            break;
        default:
            break;
        }
    TRACE_FUNC_RES( ( _L( "selected feature %d" ), feature ) )
    return feature;
    }

// ---------------------------------------------------------------------------
// Get Eir Data by hostResolver in Cache
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::DoGetEirData( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC_ENTRY
    TInt err = KErrNone;
    iUuidContainter.Close();
    
    if( !iServer->iSocketServ.Handle() )
        {
        err = iServer->iSocketServ.Connect();
        }
    
    TProtocolDesc pInfo;
    if(err == KErrNone)
        {
        _LIT(KBTLinkManagerProtocol, "BTLinkManager");
        err = iServer->iSocketServ.FindProtocol( KBTLinkManagerProtocol(), pInfo );
        }
    if (err == KErrNone)
        {
        err = iHostResolver.Open(iServer->iSocketServ, pInfo.iAddrFamily, pInfo.iProtocol);
        }
    if(err == KErrNone)
        {
        iInquirySockAddr = TInquirySockAddr();
        iInquirySockAddr.SetBTAddr(aAddr);
        iInquirySockAddr.SetAction(KHostResCache | KHostResEir);
        
        TRequestStatus status;
        iHostResolver.GetByAddress(iInquirySockAddr, iNameEntry, status);
        User::WaitForRequest(status);
        err = status.Int();
        TRACE_FUNC_RES(( _L( "HostResolver GetByAddress status: %d" ), err))
        iHostResolver.Close();
        }
    if(err == KErrNone)
        {
        TBluetoothNameRecordWrapper eirWrapper( iNameEntry() );
        err = eirWrapper.GetServiceClassUuids( iUuidContainter );
        }
    
    TRACE_FUNC_EXIT
    return err;
    }

// ---------------------------------------------------------------------------
// Check if Service UUID is supported by some plugin.
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::GetConnectablePluginIndexByEir()
    {
    CBTEngPlugin::RProfileArray profiles;
    TInt count = iUuidContainter.UUIDs().Count();
    
    for( TInt u = 0; u < count; u++)
        {
        for( TInt i = 0; i < iPluginArray.Count(); i++ )
            {
            profiles.Reset();
            iPluginArray[ i ]->GetSupportedProfiles( profiles );
            for (TInt x = 0; x < profiles.Count(); x++)
                {
                if (iUuidContainter.UUIDs().At(u) == TUUID(profiles[x]))
                    {
                    profiles.Close();
    				TRACE_INFO( ( _L( "connectable plugin index %d" ), i ) )
                    return i;
                    }
                }
            }
        }
    profiles.Close();
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
// Internal utility function 
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvPluginMgr::GetFirstPluginOfProfileSupported(TBTProfile aProfile )
    {
    TRACE_FUNC_ENTRY
    for( TInt i = 0; i < iPluginArray.Count(); i++ )
        {
        if( iPluginArray[ i ]->IsProfileSupported( aProfile ) )
            {
            return i;
            }
        }
    return KErrNotFound;
    }

// ---------------------------------------------------------------------------
//  Check if any audio connection exists. 
// ---------------------------------------------------------------------------
//
	
TBool CBTEngSrvPluginMgr::CheckAudioConnectionsL()
    {
    TRACE_FUNC_ENTRY
    TBool result = EFalse;
    RArray <TBTProfile> profiles;
	CleanupClosePushL( profiles );
    profiles.AppendL( EBTProfileHSP );
    profiles.AppendL( EBTProfileHFP );
    profiles.AppendL( EBTProfileA2DP );
    profiles.AppendL( EBTProfileAVRCP );
    
	RBTDevAddrArray addrArray;	
    for ( TInt i = 0; i < profiles.Count(); i++ )
        {
        GetConnectedAddresses( addrArray, profiles[i] );
        if ( addrArray.Count() > 0 )
            {
            result = ETrue;
            break;
            }
        addrArray.Reset();
        }
	addrArray.Close();
	
	CleanupStack::PopAndDestroy( &profiles );
    TRACE_FUNC_RES( ( _L( "result: %d" ), result ) )
    return result;
    }
