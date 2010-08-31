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
* Description:  This is the ObexServiceManager ecom plugin class declaration.
*
*/


#ifndef OBEXSERVICEMANPLUGIN_H
#define OBEXSERVICEMANPLUGIN_H

#include <e32base.h>

#include <locodserviceplugin.h>
#include <locodservicepluginparams.h>
#include <locodbearer.h>


#include "obexsmclient.h"  

 
class CObexSMRequester; 
class RObexSMServer;
 
class MObexSMRequestObserver
    {
public:    
    /**
     * Tell service plugin that request has been completed.     
     *  
     * @since S60 v3.2
     * @param  aBearer a Request to be completed
     * @param  aError  Error code   
     */
    virtual void RequestComplete(CObexSMRequester* aRequest, TInt err) = 0;        
    
    /**
     * Get Obexservicemanager server interface
     *      
     * @since S60 v3.2
     * @return  Client handle.
     */
    virtual RObexSMServer& GetServer()=0; 
    
    };
    
 
    
/**
 *  ObexServiceManPlugin class
 *
 *  This is the ObexServiceMan ecom plugin class  
 *  @lib ?library
 *  @since S60 v3.2
 */
class CObexSMPlugin : public CLocodServicePlugin, public MObexSMRequestObserver
    {
public:

    /**
     * Two-phased constructor.
     * @param aParams the LocodServicePluginParams     
     */
    static CObexSMPlugin* NewL(TLocodServicePluginParams& aParams);

    /**
     * Destructor.
     */    
    ~CObexSMPlugin();

    /**
     * Tell service plugin which service should be enabled or disabled according to
     * specified bearer and its status.     
     * @since S60 v3.2
     * @param  aBearer the bearer identification 
     * @param  aStatus the status of this bearer, ETrue if it is available;
     *                 EFalse otherwise.
     */
    void ManageService(TLocodBearer aBearer, TBool aStatus);
    
    /**
     * Tell service plugin that request has been completed.     
     * @since S60 v3.2
     * @param  aRequest the Request to be completed.     
     * @param  aError  Error code   
     */
    void RequestComplete(CObexSMRequester* aRequest, TInt aError);
    
    /**
     * Get Obexservicemanager server interface     
     * @since S60 v3.2
     * @return  Client handle.
     */
    RObexSMServer&   GetServer();
    
private:

    CObexSMPlugin(TLocodServicePluginParams& aParams);
    void ConstructL();
    
private: // data 
    RObexSMServer                   iObexSMServer;
    RPointerArray<CObexSMRequester> iRequests;
    };



#endif // OBEXSERVICEMANPLUGIN_H
