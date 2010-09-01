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
* Description:  Registry observer is listening to registry changes and
* 				  and reporting the changed devices to the listener.
*
*/

#ifndef BTREGISTRYOBSERVER_H
#define BTREGISTRYOBSERVER_H

#include <e32base.h>
#include <e32property.h>
#include <btmanclient.h>
#include <btengdevman.h> 

/** This class will inform the listener about the new BTregistry status
*after registry change events. 
*
* This class will listen to events about registry changes and inform
* the listener about the latest registry status. The listener can
* also ask the status using refresh. This is usefull for for
* for example retrieval of initial lists of devices. 
*
*@lib btdevmodel.dll
*@since S60 V3.2
*/
NONSHARABLE_CLASS(MBTRegistryObserver)
    {
public:
	/** The aDeviceArray constents are deleted after this
	* call returns. If storage is wanted the array and its contents
	* must be copied by implementor.
	*/
    virtual void RegistryChangedL(const CBTDeviceArray* aDeviceArray) = 0;
    };
    
NONSHARABLE_CLASS(CBTRegistryObserver) : public CActive, public MBTEngDevManObserver
    {
public: 
	/** NewL
	 *@param aObserver The class that is listening to registry changes
	 *@param aPattern The pattern to be used to retrieve the changed objects.
	 */
    static CBTRegistryObserver* NewL(MBTRegistryObserver* aObserver, 
        const TBTRegistrySearch aPattern);
    
    /** Destructor*/
    virtual ~CBTRegistryObserver();       

	/** Start listening to registry changes. 
	 *
	 *Calling this while registry observer is running is silently ignored.
	 */
    void StartIfNotRunning();

    /* Registry observer will retrieve list of devices in the backround 
     * and send it to the listener, when one is retrieved.
     */
    void Refresh();
    
    /** Check if this is running.       
     *@return ETrue if not listening to registry changes.
     */
    inline TBool IsStopped() {return iIsStopped;}

    /** This will stop listening and cancel any retrieval of devicelists,
    * if any.
    * @param none.
    * @return none.
    */
    void Cancel();    
private:
	/* Constructor that may not leave.
	 *@param aObserver The class that is listening to registry changes
	 *@param aPattern The pattern to be used to retrieve the changed objects.
	 */
    CBTRegistryObserver(MBTRegistryObserver* aObserver, const TBTRegistrySearch aPattern);
	/** 2nd phase constructor. Called by NewL only.
    * @param none.
    * @return none.	
	 */
    void ConstructL();

	/** This is called when registry changes occur, but not when deviceLists are
	* retrieved, since HandleGetDevicesComplete is called then. 
	*
	* If one or more registry events occur during devicelist retrieval, there
	* will be one new devicelist retrieval after the current retrieval is done.
	*
    * @param none.
    * @return none.
	*/
    void RunL();
    
    /** This will clear iRegDeviceArray and delete its contents.
    * @param none.
    * @return none.     
    */
    void DeleteAllocatedDevices();
    
    
    /** This will cancel listening. To be called by Cancel only.
     * @param none.
     * @return none.
	 */
    void DoCancel();    
    
	/** This is called when devicelist have been read from BtDevMan 
	* @param aErr KErrNone or the accurred error.
	* @param aDeviceArray the devices that have been retrieved.
    * @return none.
    */
    void HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* aDeviceArray);
    
private:
    MBTRegistryObserver* iObserver; // the class listening to this class
    CBTDeviceArray* iRegDeviceArray;  // this is used for retrieving devices
    TBTRegistrySearch iSearchPattern; // the search used to retrieve devices
    								 // NOTE: this class does not filter the results,
    								 // but the one listening to this (if filtering is done).
    RProperty iProperty;
    CBTEngDevMan* iDevMan;			// the object that is used to retrieve the device lists.
    TBool iIsStopped;				// is this runining.
    TBool iAnotherEventPending;		// Has there been one or more changes during this retrieval.
    								// if this is true then HandleGetDevices will start a new search.
    };
    
#endif 
