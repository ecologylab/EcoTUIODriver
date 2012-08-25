//#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "TuioDump.h"
#include <vector>
#include "ServiceInstaller.h"
#include "ServiceBase.h"
#include "TUIOService.h"
#include "Global.h"
#include <atlconv.h>
#include <atlbase.h>
#include <wchar.h>
#include <locale.h>
#include "iostream"
#include <fstream>
#include <iostream>

   using namespace std;
 
    
 
   // Service start options. 
   #define SERVICE_START_TYPE       SERVICE_AUTO_START 
 
 
   // List of service dependencies - "dep1\0dep2\0\0" 
   #define SERVICE_DEPENDENCIES     L"" 
 
 
   // The name of the account under which the service should run 
   #define SERVICE_ACCOUNT          L".\\LocalSystem" 
 
 
   // The password to the service account name 
   #define SERVICE_PASSWORD         NULL 

wchar_t *SERVICE_NAME;
wchar_t *SERVICE_DISPLAY_NAME;
int TUIO_PORT;
int SERVICE_ID;
int _tmain(int argc, _TCHAR *argv[])
{
//	getchar();
	if (argc > 2)
    {
		
    
 			if (argc >3)TUIO_PORT = _tstoi( argv[3] );
		    SERVICE_ID = _tstoi(argv[2]);
			if(SERVICE_ID ==1 ){
				SERVICE_NAME = L"Tuio-To-vmulti-Device1" ;
				SERVICE_DISPLAY_NAME = L"Tuio-To-vmulti-Device1" ;
				 std::ofstream fs("D:\\tuioport1.txt"); 
				 fs<<TUIO_PORT;
				 fs.close();	 
				 
			}
			else if(SERVICE_ID ==2)
			{
				SERVICE_NAME = L"Tuio-To-vmulti-Device2" ;
				SERVICE_DISPLAY_NAME = L"Tuio-To-vmulti-Device2" ;
				 std::ofstream fs("tuioport2.txt"); 
				 fs<<TUIO_PORT;
				 fs.close();
			}
			else if(SERVICE_ID ==3)
			{
				SERVICE_NAME = L"Tuio-To-vmulti-Device3" ;
				SERVICE_DISPLAY_NAME = L"Tuio-To-vmulti-Device3" ;
				 std::ofstream fs("tuioport3.txt"); 
				 fs<<TUIO_PORT;
				 fs.close();
			}
			else if(SERVICE_ID ==4)
			{
				SERVICE_NAME = L"Tuio-To-vmulti-Device4" ;
				SERVICE_DISPLAY_NAME = L"Tuio-To-vmulti-Device4" ;
			    std::ofstream fs("tuioport4.txt"); 
				 fs<<TUIO_PORT;
				 fs.close();
			}
			else if(SERVICE_ID ==5)
			{
				SERVICE_NAME = L"Tuio-To-vmulti-Device5" ;
				SERVICE_DISPLAY_NAME = L"Tuio-To-vmulti-Device5" ;
			    std::ofstream fs("tuioport5.txt"); 
				 fs<<TUIO_PORT;
				 fs.close();
			}

		if(argc > 3 && _tcscmp( _T("install"), argv[1] ) == 0 )
        {
			
            // Install the service when the command is 
            // "-install" or "/install".
            InstallService(
                SERVICE_NAME,               // Name of service
                SERVICE_DISPLAY_NAME,       // Name to display
                SERVICE_START_TYPE,         // Service start type
                SERVICE_DEPENDENCIES,       // Dependencies
                SERVICE_ACCOUNT,            // Service running account
                SERVICE_PASSWORD            // Password of the account
                );     
		}
        else if (argc > 2 &&_tcscmp( argv[1], _T("remove")) == 0)
        {
            // Uninstall the service when the command is 
            // "-remove" or "/remove".
			
            UninstallService(SERVICE_NAME);
        }
		else{
		wprintf(L"Parameters:\n");
		wprintf(L" -remove   to remove the service.\n");
        wprintf(L" -install  to install the service.\n");
		}
    }
    else
    {
        wprintf(L"Parameters:\n");
        wprintf(L" -install  to install the service.\n");
        wprintf(L" -remove   to remove the service.\n");

        CSampleService service(SERVICE_NAME);
        if (!CServiceBase::Run(service))
        {
            wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
        }
    }
 
	
	return 0;
}


