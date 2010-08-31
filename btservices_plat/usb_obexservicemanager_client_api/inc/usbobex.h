/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  USB obexservicemanager client definition
*
*/


#ifndef USBOBEX_H
#define USBOBEX_H

#include <e32base.h>


class CUSBClientWrapper;


/**
*	Transportation string definition
*/
_LIT8(KSrcsTransportBT,"OBEX/BT");
_LIT8(KSrcsTransportUSB,"OBEX/USB");
_LIT8(KSrcsTransportIrDA,"OBEX/IrDA");


/**
 *  obexservicemanager client for usb.
 *  @lib obexusbapi.lib
 *  @since S60 v3.2
 */
class CObexUSB : public CBase
    {
public:

   /**
    * Two-phased constructor.    
    */
    IMPORT_C static CObexUSB* NewL();
        
   /**
    * Destructor.
    */
    ~CObexUSB();
    
   /**
    * Turn USB services on or off
    *
    * @since S60 v3.2
    * @param aUSBStatus the requested USB service status
    * @param aStatus the Request status
    * @return None
    */
    IMPORT_C void  ManageUSBServices( TBool aUSBStatus, TRequestStatus &aStatus );     
      
   /**
    * Cancel managment request and turn services off   
    * @since S60 v3.2
    *
    */
    IMPORT_C void  CancelManageUSBServices();     

private: 

    CObexUSB();
    void ConstructL();

private: //data

   /**
    * Obexserviceman client wrapper
    */
    CUSBClientWrapper* iUSBWrapper;        
    };
#endif // USBOBEX_H
