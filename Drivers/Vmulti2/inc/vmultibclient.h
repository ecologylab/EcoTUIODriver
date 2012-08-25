#if !defined(_vmultib_CLIENT_H_)
#define _vmultib_CLIENT_H_

#include "vmultibcommon.h"

typedef struct _vmultib_client_t* pvmultib_client;

pvmultib_client vmultib_alloc(void);

void vmultib_free(pvmultib_client vmultib);

BOOL vmultib_connect(pvmultib_client vmultib);

void vmultib_disconnect(pvmultib_client vmultib);

BOOL vmultib_update_mouse(pvmultib_client vmultib, BYTE button, USHORT x, USHORT y, BYTE wheelPosition);

BOOL vmultib_update_relative_mouse(pvmultib_client vmultib, BYTE button, BYTE x, BYTE y, BYTE wheelPosition);

BOOL vmultib_update_digi(pvmultib_client vmultib, BYTE status, USHORT x, USHORT y);

BOOL vmultib_update_multitouch(pvmultib_client vmultib, PTOUCH pTouch, BYTE actualCount);

BOOL vmultib_update_joystick(pvmultib_client vmultib, USHORT buttons, BYTE hat, BYTE x, BYTE y, BYTE rx, BYTE ry, BYTE throttle);

BOOL vmultib_update_keyboard(pvmultib_client vmultib, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES]);

BOOL vmultib_write_message(pvmultib_client vmultib, vmultibMessageReport* pReport);

BOOL vmultib_read_message(pvmultib_client vmultib, vmultibMessageReport* pReport);

#endif
