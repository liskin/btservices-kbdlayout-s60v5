/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BtDeviceDialogWidget class declaration.
*
*/

#ifndef BTRECVPRGRSDIALOGWIDGET_H
#define BTRECVPRGRSDIALOGWIDGET_H

#include <QObject>
#include <QVariantMap>
#include <hbdialog.h>
#include <hbdevicedialoginterface.h>
#include <hbpopup.h>
#include <hbdocumentloader.h>
#include <qstandarditemmodel.h>
#include <hbprogressbar.h>
#include <hblabel.h>
#include <hbaction.h>


class BTRecvPrgrsDialogWidget : public QObject, 
                                public HbDeviceDialogInterface
    {
    Q_OBJECT
    
public:
    BTRecvPrgrsDialogWidget(const QVariantMap &parameters);
    ~BTRecvPrgrsDialogWidget();
    
public: // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbPopup *deviceDialogWidget() const;
    virtual QObject *signalSender() const;
    
public slots:
    void hideClicked();
    void cancelClicked();

private:
    bool constructDialog(const QVariantMap &parameters);
    
signals:
    void deviceDialogClosed();
    void deviceDialogData(QVariantMap data);
    
private:
    HbDocumentLoader    *mLoader;
    QStandardItemModel  *mContentItemModel;
    HbLabel             *mHeading;
    HbLabel             *mFileName;
    HbLabel             *mFileSize;
    HbLabel             *mFileCount;
    HbAction            *mHide;
    HbAction            *mCancel;
    HbDialog            *mDialog;
    HbProgressBar       *mProgressBar;
    int                 mFileSz;
    
    Q_DISABLE_COPY(BTRecvPrgrsDialogWidget)
    };

#endif /* BTRECVPRGRSDIALOGWIDGET_H */
