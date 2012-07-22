/****************************** Module Header ******************************\
* Module Name:  SampleService.cpp
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma region Includes
#include "TUIOService.h"
#include "ThreadPool.h"
#include <Windows.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "TuioDump.h"
#include <vector>
#include "ServiceInstaller.h"
#include "ServiceBase.h"
#include "TUIOService.h"
#include "Global.h"
#include "iostream"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
extern "C" 
 {
   #include "vmulticlient.h"
 }
#pragma endregion


CSampleService::CSampleService(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
	
}


CSampleService::~CSampleService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}


void SendHidRequests_touchdown(pvmulti_client vmulti,BYTE requestType,int contact_id,float x,float y);
void SendHidRequests_touchup(pvmulti_client vmulti,BYTE requestType,int contact_id,float x,float y);
pvmulti_client vmulti;
BYTE   reportId = REPORTID_MTOUCH;
int port;

void TuioDump::addTuioObject(TuioObject *tobj) {
	std::cout << "add obj " << tobj->getSymbolID() << " (" << tobj->getSessionID() << ") "<< tobj->getX() << " " << tobj->getY() << " " << tobj->getAngle() << std::endl;
	
}

void TuioDump::updateTuioObject(TuioObject *tobj) {
	std::cout << "set obj " << tobj->getSymbolID() << " (" << tobj->getSessionID() << ") "<< tobj->getX() << " " << tobj->getY() << " " << tobj->getAngle() 
				<< " " << tobj->getMotionSpeed() << " " << tobj->getRotationSpeed() << " " << tobj->getMotionAccel() << " " << tobj->getRotationAccel() << std::endl;
}

void TuioDump::removeTuioObject(TuioObject *tobj) {
	std::cout << "del obj " << tobj->getSymbolID() << " (" << tobj->getSessionID() << ")" << std::endl;
}

void TuioDump::addTuioCursor(TuioCursor *tcur) {
	std::cout << "add cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() << std::endl;
	SendHidRequests_touchdown(vmulti,reportId,tcur->getCursorID() ,tcur->getX(),tcur->getY());
}

void TuioDump::updateTuioCursor(TuioCursor *tcur) {
	std::cout << "set cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ") " << tcur->getX() << " " << tcur->getY() 
				<< " " << tcur->getMotionSpeed() << " " << tcur->getMotionAccel() << " " << std::endl;
	SendHidRequests_touchdown(vmulti,reportId,tcur->getCursorID() ,tcur->getX(),tcur->getY());
}

void TuioDump::removeTuioCursor(TuioCursor *tcur) {
	std::cout << "del cur " << tcur->getCursorID() << " (" <<  tcur->getSessionID() << ")" << std::endl;
	SendHidRequests_touchup(vmulti, reportId,tcur->getCursorID(),tcur->getX(),tcur->getY());
		
}

void  TuioDump::refresh(TuioTime frameTime) {
	//std::cout << "refresh " << frameTime.getTotalMilliseconds() << std::endl;
}


void SendHidRequests_touchdown(pvmulti_client vmulti,BYTE requestType,int contact_id,float x,float y)
{
	
		        PTOUCH pTouch = (PTOUCH)malloc(1 * sizeof(TOUCH));
                pTouch[0].ContactID = contact_id;
                pTouch[0].Status = MULTI_CONFIDENCE_BIT | MULTI_IN_RANGE_BIT | MULTI_TIPSWITCH_BIT;
				
                pTouch[0].XValue = USHORT(x * (int)MULTI_MAX_COORDINATE);
                pTouch[0].YValue = USHORT(y * (int)MULTI_MAX_COORDINATE);
				std::cout << USHORT(x * (int)MULTI_MAX_COORDINATE) <<std::endl;
				std::cout << USHORT(y * (int)MULTI_MAX_COORDINATE) <<std::endl;
				pTouch[0].Width = 20;
                pTouch[0].Height = 30;

				if (!vmulti_update_multitouch(vmulti, pTouch, 1))
				printf("TOUCH_DOWN FAILED\N");
				free(pTouch);
    
}

void SendHidRequests_touchup(pvmulti_client vmulti,BYTE requestType,int contact_id,float x,float y)
{
	PTOUCH pTouch = (PTOUCH)malloc(1 * sizeof(TOUCH));
    pTouch[0].ContactID = contact_id;
	pTouch[0].XValue = USHORT(x * (int)MULTI_MAX_COORDINATE);
    pTouch[0].YValue = USHORT(y * (int)MULTI_MAX_COORDINATE);
	pTouch[0].Width = 20;
    pTouch[0].Height = 30;
	pTouch[0].Status = 0;
	if (!vmulti_update_multitouch(vmulti, pTouch, 1))
    printf("TOUCH_UP FAILED\n");
	free(pTouch);
}

//
//   FUNCTION: CSampleService::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the 
//   service by the SCM or when the operating system starts (for a service 
//   that starts automatically). It specifies actions to take when the 
//   service starts. In this code sample, OnStart logs a service-start 
//   message to the Application log, and queues the main service function for 
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore, 
//   it usually polls or monitors something in the system. The monitoring is 
//   set up in the OnStart method. However, OnStart does not actually do the 
//   monitoring. The OnStart method must return to the operating system after 
//   the service's operation has begun. It must not loop forever or block. To 
//   set up a simple monitoring mechanism, one general solution is to create 
//   a timer in OnStart. The timer would then raise events in your code 
//   periodically, at which time your service could do its monitoring. The 
//   other solution is to spawn a new thread to perform the main service 
//   functions, which is demonstrated in this code sample.
//


void CSampleService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
    // Log a service start message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStart", 
        EVENTLOG_INFORMATION_TYPE);

    // Queue the main service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
	

}


//
//   FUNCTION: CSampleService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void CSampleService::ServiceWorkerThread(void)
{
    // Periodically check if the service is stopping.
    while (!m_fStopping)
    {
        // Perform main service function here...
    //declare the client
	//std::cout << "Minimum coordinate on screen is " << MULTI_MIN_COORDINATE << std::endl;
    //std::cout << "Maximum coordinate on screen is " << MULTI_MAX_COORDINATE << std::endl;
    vmulti = vmulti_alloc();

 
    if (!vmulti_connect(vmulti))
    {
        vmulti_free(vmulti);
       
    }
	
    //printf("...sending request(s) to our device\n");
    

	/*if( argc >= 2 && strcmp( argv[1], "-h" ) == 0 ){
        	std::cout << "usage: TuioDump [port]\n";
        	return 0;
	}*/
	
	
     //	if( argc >= 2 ) port = atoi( argv[1] );
//	 CIniReader iniReader(".\\Logger.ini");
	
	TuioDump dump;
	//Getting the port from the file . 
	port=0;
	string STRING;
	ifstream infile;
	infile.open ("../Data/tuioport5.txt");
    
	getline(infile,STRING); // Saves the line in STRING.
	cout<<"rajat2";
	
	infile.close();
	char *a=new char[STRING.size()+1];
	a[STRING.size()]=0;
	memcpy(a,STRING.c_str(),STRING.size());
	int port = atoi( a );
	//ends here
	TuioClient client(port);
	client.addTuioListener(&dump);
	client.connect(true);
	
    }

    // Signal the stopped event.
    SetEvent(m_hStoppedEvent);
}


//
//   FUNCTION: CSampleService::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the 
//   service by SCM. It specifies actions to take when a service stops 
//   running. In this code sample, OnStop logs a service-stop message to the 
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with 
//   SERVICE_STOP_PENDING if the procedure is going to take long time. 
//

void CSampleService::OnStop()
{
    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
	//getting the port from the file
	
	vmulti_disconnect(vmulti);
    vmulti_free(vmulti);
	
   /* if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }*/
}