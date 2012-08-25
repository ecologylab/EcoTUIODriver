#if !defined(_vmultid_CLIENT_H_)
#define _vmultid_CLIENT_H_

#include "vmultidcommon.h"

typedef struct _vmultid_client_t* pvmultid_client;

pvmultid_client vmultid_alloc(void);

void vmultid_free(pvmultid_client vmultid);

BOOL vmultid_connect(pvmultid_client vmultid);

void vmultid_disconnect(pvmultid_client vmultid);

BOOL vmultid_update_mouse(pvmultid_client vmultid, BYTE button, USHORT x, USHORT y, BYTE wheelPosition);

BOOL vmultid_update_relative_mouse(pvmultid_client vmultid, BYTE button, BYTE x, BYTE y, BYTE wheelPosition);

BOOL vmultid_update_digi(pvmultid_client vmultid, BYTE status, USHORT x, USHORT y);

BOOL vmultid_update_multitouch(pvmultid_client vmultid, PTOUCH pTouch, BYTE actualCount);

BOOL vmultid_update_joystick(pvmultid_client vmultid, USHORT buttons, BYTE hat, BYTE x, BYTE y, BYTE rx, BYTE ry, BYTE throttle);

BOOL vmultid_update_keyboard(pvmultid_client vmultid, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES]);

BOOL vmultid_write_message(pvmultid_client vmultid, vmultidMessageReport* pReport);

BOOL vmultid_read_message(pvmultid_client vmultid, vmultidMessageReport* pReport);

#endif
