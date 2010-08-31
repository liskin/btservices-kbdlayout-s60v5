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
* Description: 
*
*/

#include "btdevicemodel_p.h"
#include <QDateTime>
#include <btservices/advancedevdiscoverer.h>
#include <btengconnman.h>
#include "btuiutil.h"
#include "btuidevtypemap.h"
#include "btqtconstants.h"

/*!
    Constructor.
 */
BtDeviceModelPrivate::BtDeviceModelPrivate( BtDeviceModel& model, QObject *parent )
    : QObject( parent ), mModel( model ), mDiscover( 0 ), mSeqNum( 0 )
{
    mDeviceRepo = 0;
    isSearchingDevice = false;
    TRAP_IGNORE({
        mDeviceRepo = CBtDevRepository::NewL();
    });
    Q_CHECK_PTR( mDeviceRepo );
    TRAP_IGNORE( mDeviceRepo->AddObserverL( this ) );
    
    if ( mDeviceRepo->IsInitialized() ) {
        initializeDataStore();
    }
}

/*!
    Destructor.
 */
BtDeviceModelPrivate::~BtDeviceModelPrivate()
{
    delete mDeviceRepo;
    delete mDiscover;
}


/*!
    Tells whether the given column is in the range of the setting list.
    
    \param row the row number to be checked
    \param col the column number to be checked
    
    \return true if the given row and column are valid; false otherwise.
*/
bool BtDeviceModelPrivate::isValid( int row, int column) const
{
    return row >= 0 && row < mData.count() && column == 0;
}

/*!
    \return the total amount of rows.
    
*/
int BtDeviceModelPrivate::rowCount() const
{
    return mData.count();
}

/*!
    \return the total amount of columns.
    
*/
int BtDeviceModelPrivate::columnCount() const
{
    return 1;
}

/*!
    Gets the value within a data item.
    \param val contains the value at return.
    \param row the row number which the value is from
    \param col the column number which the value is from
    \param role the role identifier of the value.
 */
void BtDeviceModelPrivate::data(QVariant& val, int row,  int col, int role ) const
{
    if ( isValid( row, col ) ) {
        val = mData.at( row ).value( role );
    }
    else {
        val = QVariant( QVariant::Invalid );
    }
}

/*!
    Gets the whole item data at the specified column
    \param row the row number of the item data to be returned
    \param col the column number of the item data to be returned
    \return the item data
 */
BtuiModelDataItem BtDeviceModelPrivate::itemData( int row, int col ) const
{
    if ( isValid( row, col ) ) {
        return mData.at( row );
    }
    return BtuiModelDataItem();
}


/*!
    Requests the model to searching Bluetooth devices.
    \return true if the request is accepted; false otherwise
 */
bool BtDeviceModelPrivate::searchDevice()
{
    int err ( 0 );
    removeTransientDevices();
    if ( !mDiscover ) {
        TRAP(err, mDiscover = CAdvanceDevDiscoverer::NewL( *mDeviceRepo, *this) );
    }
    if ( !err ) {
        TRAP(err, mDiscover->DiscoverDeviceL() );
    }
    isSearchingDevice = true;
    return err == 0;
}

/*!
    Cancels a possible outstanding device search request.
 */
void BtDeviceModelPrivate::cancelSearchDevice()
{
    if ( mDiscover ) {
        isSearchingDevice = false;
        mDiscover->CancelDiscovery();
    }
}

/*!
    Removes transient (not-in-registry) devices 
    (added as the result of device search).
 */
void BtDeviceModelPrivate::removeTransientDevices()
{
    // clear in-range property for all device items in this model.
    int cnt = mData.count();
    for ( int i = mData.count() - 1; i > -1; --i)
        {
        const BtuiModelDataItem& qtdev = mData.at(i);
        if(isDeviceInRange(qtdev)) {
            if(isDeviceInRegistry(qtdev)) {
                // we cannot remove this device as it is in registry.
                // remove it in-range property.
                setMajorProperty(mData[i], BtuiDevProperty::InRange, false);
                updateRssi(mData[i], RssiInvalid);
                updateSeqNum(mData[i], -1);
                emit deviceDataChanged(i, this);
            }
            else {
                // this device is not in-registry. Delete it from local
                // store.
                emit beginRemoveDevices(i, i, this);
                mData.removeAt( i );
                emit endRemoveDevices();
            }
        }
    }
}

/*!
    callback from repository.
    re-initialize our store.
 */
void BtDeviceModelPrivate::RepositoryInitialized() 
{
    initializeDataStore();
}

/*!
    callback from repository.
    update our store.
 */
void BtDeviceModelPrivate::DeletedFromRegistry( const TBTDevAddr& addr ) 
{
    int i = indexOf( addr );
    if ( i > -1 ) {
        if ( isSearchingDevice && isDeviceInRange( mData.at(i) ) ) {
            // device searching is ongoing, and it is in-range. we can not 
            // remove it from model now.
            // clear-registry related properties, so that
            // we get a chance to clean it after device searching later.
            setMajorProperty(mData[i], BtuiDevProperty::RegistryProperties, false);
            emit deviceDataChanged(i, this);
        }
        else {
            emit beginRemoveDevices(i, i, this);
            mData.removeAt( i );
            emit endRemoveDevices();
        }
    }
}

/*!
    callback from repository.
    update our store.
 */
void BtDeviceModelPrivate::AddedToRegistry( const CBtDevExtension& dev ) 
{
    ChangedInRegistry( dev, 0 );
}

/*!
    callback from repository.
    update our store.
 */
void BtDeviceModelPrivate::ChangedInRegistry( 
        const CBtDevExtension& dev, TUint similarity )
{
    int i = indexOf( dev.Addr() );
    if ( i == -1 ) {
        BtuiModelDataItem devData;
        if ( !isSearchingDevice ) {
            // Rssi is only available at device inquiry stage. 
            // We initialize this property to an invalid value
            updateRssi(devData, RssiInvalid);
        }
        // add device-in-registry property:
        setMajorProperty(devData, BtuiDevProperty::InRegistry, true);
        updateDeviceProperty(devData, dev, 0 );
        emit beginInsertDevices( mData.count(), mData.count(), this );
        mData.append( devData );
        emit endInsertDevices();
    }
    else {
        updateDeviceProperty(mData[i], dev, similarity );
        setMajorProperty(mData[i], BtuiDevProperty::InRegistry, true);
        emit deviceDataChanged( i, this );
    }
}

/*!
    callback from repository.
    update our store.
 */
void BtDeviceModelPrivate::ServiceConnectionChanged(
        const CBtDevExtension& dev, TBool connected )
{
    int i = indexOf( dev.Addr() );
    if ( i > -1 ) {
        int preconn =  BtuiDevProperty::Connected 
                & mData[i][BtDeviceModel::MajorPropertyRole].toInt();
        // we only update and signal if connection status is really
        // changed:
        if ( ( preconn != 0 && !connected )
            || ( preconn == 0 && connected ) ) {
            setMajorProperty(mData[i], BtuiDevProperty::Connected, connected );
            emit deviceDataChanged( i, this );
        }
    }
    // it is impossible that a device has connected but it is not in
    // our local store according to current bteng services.
    // need to take more care in future when this becomes possible.
}

/*!
    callback from device search.
    update our store.
 */
void BtDeviceModelPrivate::HandleNextDiscoveryResultL( 
        const TInquirySockAddr& inqAddr, const TDesC& name )
{
    int pos = indexOf( inqAddr.BTAddr() );
    const CBtDevExtension* dev = mDeviceRepo->Device( inqAddr.BTAddr() );
    
    //RssiRole
    int rssi( RssiInvalid ); // initialize to an invalid value.
    if( inqAddr.ResultFlags() & TInquirySockAddr::ERssiValid ) {
        rssi = inqAddr.Rssi();
    }
    
    if ( pos == -1 ) {
        BtuiModelDataItem devData;
        setMajorProperty(devData, BtuiDevProperty::InRange, true);
        updateRssi(devData, rssi);
        updateSeqNum( devData, mSeqNum++ );
        CBtDevExtension* devExt(NULL);
        TRAP_IGNORE( {
            devExt = CBtDevExtension::NewLC( inqAddr, name );
            CleanupStack::Pop(); });
        updateDeviceProperty(devData, *devExt, 0);
        delete devExt;
        emit beginInsertDevices( mData.count(), mData.count(), this );
        mData.append( devData );
        emit endInsertDevices();
    }
    else {
        setMajorProperty(mData[pos], BtuiDevProperty::InRange, true);
        updateRssi(mData[pos], rssi);
        updateSeqNum( mData[pos], mSeqNum++ );
        emit deviceDataChanged( pos, this );
    }
}

/*!
    callback from device search.
    inform client.
 */
void BtDeviceModelPrivate::HandleDiscoveryCompleted( TInt error )
{
    isSearchingDevice = false;
    // Reset the sequence number for the next search
    mSeqNum = 0;
    emit deviceSearchCompleted( (int) error );
}

void BtDeviceModelPrivate::initializeDataStore()
    {
    
    mSeqNum = 0;  // reset when starting search again
    
    // it is possible that we are searching devices.
    // We use a simple but not-so-efficient method to update the model.
    
    // If the device store is not empty, we clear
    // registry property from these devices first.
    for (int i = 0; i < mData.count(); ++i) {
        setMajorProperty(mData[i], BtuiDevProperty::RegistryProperties, false);
    }
    if ( mData.count() ) {
        // need to update view because we have changed device properties.
        emit deviceDataChanged( 0, mData.count() - 1, this );
    }

    const RDevExtensionArray& devs = mDeviceRepo->AllDevices();
    for (int i = 0; i < devs.Count(); ++i) {
        int pos = indexOf( devs[i]->Addr() );
        if ( pos > -1 ) {
            // add device-in-registry property:
            setMajorProperty(mData[pos], BtuiDevProperty::InRegistry, true);            
            updateDeviceProperty(mData[pos], *(devs[i]), 0);
            updateSeqNum(mData[pos], -1);
            emit deviceDataChanged( pos, this );
        }
        else {
            BtuiModelDataItem devData;
            // add device-in-registry property:
            setMajorProperty(devData, BtuiDevProperty::InRegistry, true);
            updateDeviceProperty(devData, *( devs[i] ), 0 );
            updateSeqNum(devData, -1);
            emit beginInsertDevices(mData.count(), mData.count(), this );
            mData.append( devData );
            emit endInsertDevices();
        }
    }
}

void BtDeviceModelPrivate::updateDeviceProperty(BtuiModelDataItem& qtdev,
        const CBtDevExtension& dev, TUint similarity )
{
    // similarity is not used currently. 
    // It is possible to gain better performance
    // with this info to avoid re-manipulate
    // unchanged properties.
    Q_UNUSED(similarity);
    
    //DevDisplayNameRole
    QString str = QString::fromUtf16( 
            dev.Alias().Ptr(), dev.Alias().Length() );
    qtdev[BtDeviceModel::NameAliasRole] = QVariant( str );

    //DevAddrReadableRole
    addrSymbianToReadbleString( str, dev.Addr() );
    qtdev[BtDeviceModel::ReadableBdaddrRole] = QVariant( str );

    //LastUsedTimeRole
    TDateTime symDt = dev.Device().Used().DateTime();
    QDate date( symDt.Year(), symDt.Month(), symDt.Day() );
    QTime time( symDt.Hour(), symDt.Minute(), symDt.MicroSecond() / 1000 );
    QDateTime qdt(date, time);
    qtdev[BtDeviceModel::LastUsedTimeRole] = QVariant(qdt);
 
    // set paired status:
    setMajorProperty(qtdev, BtuiDevProperty::Bonded, isBonded( dev.Device() ));
    
    // set blocked status:
    setMajorProperty(qtdev, BtuiDevProperty::Blocked, 
            dev.Device().GlobalSecurity().Banned() );
    // set trusted status:
    setMajorProperty(qtdev, BtuiDevProperty::Trusted, 
            dev.Device().GlobalSecurity().NoAuthorise() );
    // set connected status:
    // EBTEngConnecting is an intermediate state between connected and not-connected, 
    // we do not treat it as connected:         
    setMajorProperty(qtdev, BtuiDevProperty::Connected, dev.ServiceConnectionStatus() == EBTEngConnected);

    // Check whether the device has services that are connectable in bteng scope.
    CBTEngConnMan* connMan( 0 );
    TRAP_IGNORE( connMan = CBTEngConnMan::NewL(0));
    TBool connectable(EFalse);
    if ( connMan ) {
        (void) connMan->IsConnectable(dev.Addr(), dev.Device().DeviceClass(), connectable);
        delete connMan;
    }
    setMajorProperty(qtdev, BtuiDevProperty::Connectable, connectable);
    
    int cod = static_cast<int>( dev.Device().DeviceClass().DeviceClass() );
    qtdev[BtDeviceModel::CoDRole] = QVariant(cod);

    int majorDeviceType;
    int minorDeviceType;
    // device type is mapped according to CoD:
    BtuiDevProperty::mapDeiveType(majorDeviceType, minorDeviceType, cod);



    
    qtdev[BtDeviceModel::MajorPropertyRole] = 
            QVariant( qtdev[BtDeviceModel::MajorPropertyRole].toInt() | majorDeviceType );
    qtdev[BtDeviceModel::MinorPropertyRole] = QVariant( minorDeviceType );
}

int BtDeviceModelPrivate::indexOf( const TBTDevAddr& addr ) const
{
    QString addrStr;
    addrSymbianToReadbleString( addrStr, addr );
    for (int i = 0; i < mData.count(); ++i ) {
        if ( mData.at( i ).value( BtDeviceModel::ReadableBdaddrRole ) 
                == addrStr ) {
            return i;
        }
    }
    return -1;
}

void BtDeviceModelPrivate::updateRssi(BtuiModelDataItem& qtdev, int rssi )
{
    qtdev[BtDeviceModel::RssiRole] = QVariant( rssi );
}

void BtDeviceModelPrivate::updateSeqNum(BtuiModelDataItem& qtdev, int seqNum )
{
    qtdev[BtDeviceModel::SeqNumRole] = QVariant( seqNum );
}

/*!
    Add the specified major property to the device if addto is true.
    Otherwise the property is removed from the device. 
 */
void BtDeviceModelPrivate::setMajorProperty(
        BtuiModelDataItem& qtdev, int prop, bool addto)
{
    if ( addto ) {
        qtdev[BtDeviceModel::MajorPropertyRole] = 
            QVariant( qtdev[BtDeviceModel::MajorPropertyRole].toInt() | prop);
    }
    else {
        qtdev[BtDeviceModel::MajorPropertyRole] = 
            QVariant( qtdev[BtDeviceModel::MajorPropertyRole].toInt() & ~prop);
    }
}

bool BtDeviceModelPrivate::isDeviceInRange( const BtuiModelDataItem& qtdev )
{   
    return BtuiDevProperty::InRange & qtdev[BtDeviceModel::MajorPropertyRole].toInt();
}

bool BtDeviceModelPrivate::isDeviceInRegistry( const BtuiModelDataItem& qtdev )
{
    return BtuiDevProperty::InRegistry & qtdev[BtDeviceModel::MajorPropertyRole].toInt();
}
