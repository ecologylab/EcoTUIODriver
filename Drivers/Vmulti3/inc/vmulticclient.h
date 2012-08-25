#if !defined(_vmultic_CLIENT_H_)
#define _vmultic_CLIENT_H_

#include "vmulticcommon.h"

typedef struct _vmultic_client_t* pvmultic_client;

pvmultic_client vmultic_alloc(void);

void vmultic_free(pvmultic_client vmultic);

BOOL vmultic_connect(pvmultic_client vmultic);

void vmultic_disconnect(pvmultic_client vmultic);

BOOL vmultic_update_mouse(pvmultic_client vmultic, BYTE button, USHORT x, USHORT y, BYTE wheelPosition);

BOOL vmultic_update_relative_mouse(pvmultic_client vmultic, BYTE button, BYTE x, BYTE y, BYTE wheelPosition);

BOOL vmultic_update_digi(pvmultic_client vmultic, BYTE status, USHORT x, USHORT y);

BOOL vmultic_update_multitouch(pvmultic_client vmultic, PTOUCH pTouch, BYTE actualCount);

BOOL vmultic_update_joystick(pvmultic_client vmultic, USHORT buttons, BYTE hat, BYTE x, BYTE y, BYTE rx, BYTE ry, BYTE throttle);

BOOL vmultic_update_keyboard(pvmultic_client vmultic, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES]);

BOOL vmultic_write_message(pvmultic_client vmultic, vmulticMessageReport* pReport);

BOOL vmultic_read_message(pvmultic_client vmultic, vmulticMessageReport* pReport);

#endif
