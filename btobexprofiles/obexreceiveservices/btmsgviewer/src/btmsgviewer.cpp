/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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


#include "btmsgviewer.h"
#include "apmstd.h"
#include <xqaiwrequest.h>
#include <xqconversions.h>
#include <apgcli.h>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <hbapplication.h>

const QString KMimeTypeVCard("text/X-vCard");


BTMsgViewer::BTMsgViewer(QObject* parent)
: XQServiceProvider("com.nokia.services.btmsgdispservices.displaymsg", parent), mCurrentRequestIndex(0)
    {    
    publishAll();
    connect(this, SIGNAL(returnValueDelivered()), qApp, SLOT(quit()));
    }

BTMsgViewer::~BTMsgViewer ()
    {

    }

void BTMsgViewer::displaymsg( int messageId )
    { 
    mCurrentRequestIndex = setCurrentRequestAsync(); 
    CBtMsgViewerUtils* btViewerUtils = 0;
    
    TRAPD(error, btViewerUtils = CBtMsgViewerUtils::NewL());  
    if(isError(error))
        {
        if(btViewerUtils)
            delete btViewerUtils;
        
        QVariant retVal(error); 
        completeRequest(mCurrentRequestIndex, retVal);
        return;
        }
        
    HBufC* fileName = 0;
    fileName = btViewerUtils->GetMessagePath(messageId, error);
    if(isError(error))
        {
        if(fileName)
            delete fileName;
        
        delete btViewerUtils;
        
        QVariant retVal(error); 
        completeRequest(mCurrentRequestIndex, retVal);
        return;
        }
    
    QString attachmentFName = XQConversions::s60DescToQString(fileName->Des());
    QString mimeType = XQConversions::s60Desc8ToQString(btViewerUtils->GetMimeType()->Des());
    delete fileName;
    delete btViewerUtils;
    
    if(mimeType == KMimeTypeVCard)
        {
        int error = KErrGeneral;
        
        /*todo: copyVCardToTemp() has to be removed when phonebook updates it's capabilites to
                access messages from private folder*/
        QString newfilepath = copyVCardToTemp(attachmentFName);
    
        QString service("com.nokia.services.phonebookservices");
        QString interface("Fetch");
        QString operation("editCreateNew(QString)");
        XQApplicationManager appManager;
        XQAiwRequest* request = appManager.create(service, interface, operation, true); //embedded
        if(request)
        {
            QList<QVariant> args;
            args << newfilepath;
            request->setArguments(args);
            QVariant retValue;
            bool res = request->send(retValue);
            if  (!res) 
                {
                error = request->lastError();
                }
            else
                {
                error = retValue.toInt();
                }
            
            delete request;
        }

        /*todo: copyVCardToTemp() has to be removed when phonebook updates it's capabilites to
                access messages from private folder*/        
        deleteVCardFromTemp(newfilepath);
        
        QVariant retVal(error); 
        completeRequest(mCurrentRequestIndex, retVal);
        return;
        }
    
    XQSharableFile sf;
    XQAiwRequest* request = 0;

    if (!sf.open(attachmentFName)) {
    QVariant retVal(KErrGeneral); 
    completeRequest(mCurrentRequestIndex, retVal);
    return;
    }

    // Get handlers
    XQApplicationManager appManager;
    QList<XQAiwInterfaceDescriptor> fileHandlers = appManager.list(sf);
    if (fileHandlers.count() > 0) {
        XQAiwInterfaceDescriptor d = fileHandlers.first();
        request = appManager.create(sf, d);

        if (!request) {
            sf.close();
            
            QVariant retVal(KErrGeneral); 
            completeRequest(mCurrentRequestIndex, retVal);
            return;
        }
    }
    else {
        sf.close();
        
        QVariant retVal(KErrGeneral); 
        completeRequest(mCurrentRequestIndex, retVal);
        return;
    }

    request->setEmbedded(true);
    request->setSynchronous(true);

    // Fill args
    QList<QVariant> args;
    args << qVariantFromValue(sf);
    request->setArguments(args);

    int err = KErrNone;
    bool res = request->send();
    if  (!res) 
        {
        err = request->lastError();
        }

    // Cleanup
    sf.close();
    delete request;
    
    QVariant retVal(err); 
    completeRequest(mCurrentRequestIndex, retVal);
    return;
    }

bool BTMsgViewer::isError(int aError)
    {
    return ((aError < KErrNone)?true:false);
    }

QString BTMsgViewer::copyVCardToTemp(const QString& filepath)
{
    QDir tempDir;
    QString tempFilePath(QDir::toNativeSeparators(tempDir.tempPath()));
    tempFilePath.append(QDir::separator());
    QFileInfo fInfo(filepath);
    tempFilePath.append(fInfo.fileName());
    QFile::copy(filepath, tempFilePath);
    return tempFilePath;
}

void BTMsgViewer::deleteVCardFromTemp(const QString& filepath)
{
    QFile::remove(filepath);
}
