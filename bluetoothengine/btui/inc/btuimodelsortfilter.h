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

#ifndef BTUIMODELSORTFILTER_H
#define BTUIMODELSORTFILTER_H

#include <QSortFilterProxyModel>
#include <btuimodeltypes.h>

/*!
    \class BtuiModelSortFilter
    \brief the class handling sorting and filtering Bluetooth devices.

    BtuiModelSortFilter handles filtering/sorting a list of Bluetooth devices.

    \\sa bluetoothuimodel
 */
class BTUIMODEL_IMEXPORT BtuiModelSortFilter : public QSortFilterProxyModel 
{
    Q_OBJECT

public:
    
    // matching mechanism applied for filters defined in Btuim::DeviceListFilter
    enum FilterMode {
        ExactMatch, // The filter value of the device must equal to the specified filters
        AtLeastMatch, // The filter value of the device must contains 
                      // all the specified filters at least.
        RoughMatch, // match at least one of the specified filters
        Exclusive,  // not match any of the specified filters
    };
    
    explicit BtuiModelSortFilter( QObject *parent = 0 );
    virtual ~BtuiModelSortFilter();
    
    void setDeviceMajorFilter( int filter, FilterMode mode );
    void addDeviceMajorFilter( int filter, FilterMode mode );
    void clearDeviceMajorFilter( int filter, FilterMode mode );
    void clearDeviceMajorFilters();
    
    bool hasFilter( int filter, FilterMode mode );
    
signals:
    void deviceAcceptedByFilter(int sourceRow);
    
protected:
    
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    class FilterItem {
    public:
        int mFilter;
        FilterMode mMode;
        FilterItem(int filter, FilterMode mode ) {
            mFilter = filter; mMode = mode;
        }
        bool operator== ( const FilterItem & other ) const { 
            return mFilter == other.mFilter && mMode == other.mMode;
        }
    };
    
private:
    QList<FilterItem> mFilters;
};

#endif // BTUIMODELSORTFILTER_H
