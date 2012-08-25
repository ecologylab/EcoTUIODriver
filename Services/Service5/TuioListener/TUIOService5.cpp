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
#include <map>
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

void SendHidRequests_updatetouch(pvmulti_client vmulti,BYTE requestType,bool remove);

pvmulti_client vmulti;
BYTE   reportId = REPORTID_MTOUCH;
int port;
string invert_x="False";
string invert_y="False";
int offset=0;
std::ofstream fslog("C://log5.txt"); 
map<int,float> tcur_x;
map<int,float> tcur_y;
map<int,BYTE> tcur_status;

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
		fslog<<"touch ";
			fslog<<tcur->getCursorID();
			fslog<<" added \n";
	tcur_x[tcur->getCursorID()]=tcur->getX();
	tcur_y[tcur->getCursorID()]=tcur->getY();
	tcur_status[tcur->getCursorID()]=MULTI_CONFIDENCE_BIT | MULTI_IN_RANGE_BIT | MULTI_TIPSWITCH_BIT;
    SendHidRequests_updatetouch(vmulti,reportId,false);

}

void TuioDump::updateTuioCursor(TuioCursor *tcur) {
	tcur_x[tcur->getCursorID()]=tcur->getX();
	tcur_y[tcur->getCursorID()]=tcur->getY();
		fslog<<"touch ";
			fslog<<tcur->getCursorID();
			fslog<<" updated \n";
	SendHidRequests_updatetouch(vmulti,reportId,false);
}
  
int cursor_id_to_remove;
void TuioDump::removeTuioCursor(TuioCursor *tcur) {
	cursor_id_to_remove=tcur->getCursorID();
	tcur_x.erase(tcur->getCursorID());
	tcur_y.erase(tcur->getCursorID());
		fslog<<"touch ";
		fslog<<tcur->getCursorID();
		fslog<<" removed \n";
	SendHidRequests_updatetouch(vmulti,reportId,true);
}

void  TuioDump::refresh(TuioTime frameTime) {
}

float x,y;
int i=0;

void SendHidRequests_updatetouch(pvmulti_client vmulti,BYTE requestType,bool remove)
{
	/*if(remove==false){		
	i=0;*/
			i=0;
	     	int actualCount = tcur_x.size();
			fslog<<"The no of touches is ";
			fslog<<tcur_x.size();
			fslog<<"\n";
			// set whatever number you want, lower than MULTI_MAX_COUNT
            if(remove == true)
			{
				actualCount=actualCount+1;
			}
			PTOUCH pTouch = (PTOUCH)malloc(actualCount * sizeof(TOUCH));
		    for( map<int,float>::iterator ii=tcur_x.begin(); ii!=tcur_x.end(); ++ii)
            {
				
				x=(*ii).second;
				y=tcur_y[(*ii).first];
				pTouch[i].ContactID = (*ii).first;
				pTouch[i].Status = tcur_status[(*ii).first];
				pTouch[i].XValue = USHORT(x * (int)MULTI_MAX_COORDINATE);
                pTouch[i].YValue = USHORT(y * (int)MULTI_MAX_COORDINATE);
				pTouch[i].Width = 20;
                pTouch[i].Height = 30;
                i=i+1; 
            }
			if(remove==true){
			 
			 pTouch[i].ContactID= cursor_id_to_remove;
			 pTouch[i].Status=0;}
			 if (!vmulti_update_multitouch(vmulti, pTouch, actualCount,requestType,REPORTID_CONTROL))
            { 
				 fslog<<"touch ";
				fslog<<" failed \n";
			}
	//}
	//else
	//{
	//	// set whatever number you want, lower than MULTI_MAX_COUNT
 //           PTOUCH pTouch = (PTOUCH)malloc(1 * sizeof(TOUCH));
	//	    pTouch[0].ContactID= cursor_id_to_remove;
	//		pTouch[0].Status=0;
	//		if (!vmulti_update_multitouch(vmulti, pTouch, 1))
 //           { 
	//		   fslog<<"touch ";
	//			fslog<<" failed \n";
	//		}
	//}

           
	
	
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

	
				 fslog<<"service started";
				

	 //std::ofstream fs("C://Users//AppData//TUIO-To-Vmulti//Data//service5.txt"); 
		//		 fs<<"The Service is Running";
			//	 fs.close();
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

 
    if (!vmulti_connect(vmulti,5))
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
	infile.open ("C://Users//AppData//TUIO-To-Vmulti//Data//tuioport5.txt");
    
	getline(infile,STRING); // Saves the line in STRING.
	cout<<"rajat2";
	
	infile.close();
	char *a=new char[STRING.size()+1];
	a[STRING.size()]=0;
	memcpy(a,STRING.c_str(),STRING.size());
	int port = atoi( a );

	ifstream infile2;
	infile2.open ("C://Users//AppData//TUIO-To-Vmulti//Data//inverthorizontal5.txt");
    getline(infile2,invert_x); // Saves the line in STRING.
	infile2.close();

	ifstream infile3;
	infile3.open ("C://Users//AppData//TUIO-To-Vmulti//Data//invertverticle5.txt");
    getline(infile3,invert_y); // Saves the line in STRING.
	infile3.close();


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
//TuioClient Client();
void CSampleService::OnStop()
{
	 fslog.close();
    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);
	//std::ofstream fs("C://Users//AppData//TUIO-To-Vmulti//Data//service5.txt"); 
		//		 fs<<"The Service is Stopped";
			//	 fs.close();
    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
	
	vmulti_disconnect(vmulti);
    vmulti_free(vmulti);
	
   /* if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }*/
}