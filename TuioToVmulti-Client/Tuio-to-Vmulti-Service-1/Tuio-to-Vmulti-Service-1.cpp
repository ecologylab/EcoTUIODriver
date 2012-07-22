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


int main(int argc, char *argv[])
{
	
		wchar_t *SERVICE_NAME = L"Tuio-To-vmulti-Device1" ;
		wchar_t *SERVICE_DISPLAY_NAME = L"Tuio-To-vmulti-Device1" ;
		if(argc > 2 && strcmp( "install", argv[1] ) == 0 )
        {
			int TUIO_PORT = atoi( argv[2] );
			     std::ofstream fs("C://Users//AppData//TUIO-To-Vmulti//Data//tuioport1.txt"); 
				 fs<<TUIO_PORT;
				 fs.close();
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
			  CSampleService service(SERVICE_NAME);
      
		}
        else if (argc > 2 &&strcmp( argv[1], "remove") == 0)
        {
            // Uninstall the service when the command is 
            // "-remove" or "/remove".
			
            UninstallService(SERVICE_NAME);
        }
		else{
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

   
 



