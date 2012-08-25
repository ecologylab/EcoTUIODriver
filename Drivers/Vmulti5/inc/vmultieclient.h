#if !defined(_vmultie_CLIENT_H_)
#define _vmultie_CLIENT_H_

#include "vmultiecommon.h"

typedef struct _vmultie_client_t* pvmultie_client;

pvmultie_client vmultie_alloc(void);

void vmultie_free(pvmultie_client vmultie);

BOOL vmultie_connect(pvmultie_client vmultie);

void vmultie_disconnect(pvmultie_client vmultie);

BOOL vmultie_update_mouse(pvmultie_client vmultie, BYTE button, USHORT x, USHORT y, BYTE wheelPosition);

BOOL vmultie_update_relative_mouse(pvmultie_client vmultie, BYTE button, BYTE x, BYTE y, BYTE wheelPosition);

BOOL vmultie_update_digi(pvmultie_client vmultie, BYTE status, USHORT x, USHORT y);

BOOL vmultie_update_multitouch(pvmultie_client vmultie, PTOUCH pTouch, BYTE actualCount);

BOOL vmultie_update_joystick(pvmultie_client vmultie, USHORT buttons, BYTE hat, BYTE x, BYTE y, BYTE rx, BYTE ry, BYTE throttle);

BOOL vmultie_update_keyboard(pvmultie_client vmultie, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES]);

BOOL vmultie_write_message(pvmultie_client vmultie, vmultieMessageReport* pReport);

BOOL vmultie_read_message(pvmultie_client vmultie, vmultieMessageReport* pReport);

#endif
