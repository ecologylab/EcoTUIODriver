#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "TuioDump.h"
#include <vector>

 extern "C" 
 {
   #include "vmulticlient.h"
 }
		
 using namespace std;

void SendHidRequests_touchdown(pvmulti_client vmulti,BYTE requestType,int contact_id,float x,float y);
void SendHidRequests_touchup(pvmulti_client vmulti,BYTE requestType,int contact_id,float x,float y);
pvmulti_client vmulti;
BYTE   reportId = REPORTID_MTOUCH;

void TuioDump::addTuioObject(TuioObject *tobj) {
	std::cout << "add obj " << tobj->getSymbolID() << " (" << tobj->getSessionID() << ") "<< tobj->getX() << " " << tobj->getY() << " " << tobj->getAngle() << std::endl;
	vmulti_disconnect(vmulti);
    vmulti_free(vmulti);
	
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

int main(int argc, char* argv[])
{
	//declare the client
	std::cout << "Minimum coordinate on screen is " << MULTI_MIN_COORDINATE << std::endl;
    std::cout << "Maximum coordinate on screen is " << MULTI_MAX_COORDINATE << std::endl;
    vmulti = vmulti_alloc();

 
    if (!vmulti_connect(vmulti))
    {
        vmulti_free(vmulti);
       
    }
	
    printf("...sending request(s) to our device\n");
    
	if( argc >= 2 && strcmp( argv[1], "-h" ) == 0 ){
        	std::cout << "usage: TuioDump [port]\n";
        	return 0;
	}
	int port = 3333;
	if (argc == 1)
    {
     port = (int)argv[1];
	}
	if( argc >= 2 ) port = atoi( argv[1] );
	
	TuioDump dump;
	TuioClient client(port);
	client.addTuioListener(&dump);
	client.connect(true);

	return 0;
}


