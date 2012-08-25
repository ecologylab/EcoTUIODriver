#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmulticclient.h"

//
// Function prototypes
//

VOID
SendHidRequests(
    pvmultic_client vmultic,
    BYTE requestType
    );

//
// Implementation
//

void
Usage(void)
{
    printf("Usage: testvmultic </multitouch | /mouse | /digitizer | /joystick | /keyboard | /message>\n");
}

INT __cdecl
main(
    int argc,
    PCHAR argv[]
    )
{
    BYTE   reportId;
    pvmultic_client vmultic;

    UNREFERENCED_PARAMETER(argv);

    //
    // Parse command line
    //

    if (argc == 1)
    {
        Usage();
        return 1;
    }
    if (strcmp(argv[1], "/multitouch") == 0)
    {
        reportId = REPORTID_MTOUCH;
    }
    else if (strcmp(argv[1], "/mouse") == 0)
    {
        reportId = REPORTID_MOUSE;
    }
    else if (strcmp(argv[1], "/digitizer") == 0)
    {
        reportId = REPORTID_DIGI;
    }
    else if (strcmp(argv[1], "/joystick") == 0)
    {
        reportId = REPORTID_JOYSTICK;
    }
    else if (strcmp(argv[1], "/keyboard") == 0)
    {
        reportId = REPORTID_KEYBOARD;
    }
    else if (strcmp(argv[1], "/message") == 0)
    {
        reportId = REPORTID_MESSAGE;
    }
    else
    {
        Usage();
        return 1;
    }

    //
    // File device
    //

    vmultic = vmultic_alloc();

    if (vmultic == NULL)
    {
        return 2;
    }

    if (!vmultic_connect(vmultic))
    {
        vmultic_free(vmultic);
        return 3;
    }

    printf("...sending request(s) to our device\n");
    SendHidRequests(vmultic, reportId);

    vmultic_disconnect(vmultic);

    vmultic_free(vmultic);

    return 0;
}

VOID
SendHidRequests(
    pvmultic_client vmultic,
    BYTE requestType
    )
{
    switch (requestType)
    {
        case REPORTID_MTOUCH:
        {
            //
            // Send the multitouch reports
            //
            
            BYTE i;
            BYTE actualCount = 4; // set whatever number you want, lower than MULTI_MAX_COUNT
            PTOUCH pTouch = (PTOUCH)malloc(actualCount * sizeof(TOUCH));

            printf("Sending multitouch report\n");
            Sleep(3000);

            for (i = 0; i < actualCount; i++)
            {
                pTouch[i].ContactID = i;
                pTouch[i].Status = MULTI_CONFIDENCE_BIT | MULTI_IN_RANGE_BIT | MULTI_TIPSWITCH_BIT;
                pTouch[i].XValue = (i + 1) * 1000;
                pTouch[i].YValue = (i + 1) * 1500 + 5000;
                pTouch[i].Width = 20;
                pTouch[i].Height = 30;
            }

            if (!vmultic_update_multitouch(vmultic, pTouch, actualCount))
              printf("vmultic_update_multitouch TOUCH_DOWN FAILED\n");
              
            for (i = 0; i < actualCount; i++)
            {
                pTouch[i].XValue += 1000;
                pTouch[i].YValue += 1000;
            }              

            if (!vmultic_update_multitouch(vmultic, pTouch, actualCount))
                printf("vmultic_update_multitouch TOUCH_MOVE FAILED\n");

            for (i = 0; i < actualCount; i++)
              pTouch[i].Status = 0;

            if (!vmultic_update_multitouch(vmultic, pTouch, actualCount))
                printf("vmultic_update_multitouch TOUCH_UP FAILED\n");
                        
            free(pTouch);
            
            break;
        }

        case REPORTID_MOUSE:
            //
            // Send the mouse report
            //
            printf("Sending mouse report\n");
            vmultic_update_mouse(vmultic, 0, 1000, 10000, 0);
            break;

        case REPORTID_DIGI:
            //
            // Send the digitizer reports
            //
            printf("Sending digitizer report\n");
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT, 1000, 10000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT, 1000, 12000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT, 1000, 14000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT | DIGI_TIPSWITCH_BIT, 1000, 16000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT | DIGI_TIPSWITCH_BIT, 1000, 18000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT | DIGI_TIPSWITCH_BIT, 1000, 20000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT | DIGI_TIPSWITCH_BIT, 2000, 20000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT | DIGI_TIPSWITCH_BIT, 3000, 20000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT, 3000, 20000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT, 3000, 15000);
            Sleep(100);
            vmultic_update_digi(vmultic, DIGI_IN_RANGE_BIT, 3000, 10000);
            vmultic_update_digi(vmultic, 0, 3000, 10000);
            break;

        case REPORTID_JOYSTICK:
        {
            //
            // Send the joystick report
            //
            USHORT buttons = 0;
            BYTE hat = 0, x = 0, y = 128, rx = 128, ry = 64, throttle = 0;
            printf("Sending joystick report\n");
            while (1)
            {
                vmultic_update_joystick(vmultic, buttons, hat, x, y, rx, ry, throttle);
                buttons = rand() | ((rand()&1) << 15); //rand() | rand() << 15 | rand() % 4 << 30;
                hat++;
                x++;
                y++;
                rx++;
                ry--;
                throttle++;
                Sleep(10);
            }
            break;
        }
        
        case REPORTID_KEYBOARD:
        {
            //
            // Send the keyboard report
            //

            // See http://www.usb.org/developers/devclass_docs/Hut1_11.pdf
            // for a list of key codes            
                        
            BYTE shiftKeys = KBD_LGUI_BIT;
            BYTE keyCodes[KBD_KEY_CODES] = {0, 0, 0, 0, 0, 0};
            printf("Sending keyboard report\n");

            // Windows key
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            shiftKeys = 0;
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            Sleep(100);
            
            // 'Hello'
            shiftKeys = KBD_LSHIFT_BIT;
            keyCodes[0] = 0x0b; // 'h'
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            shiftKeys = 0;
            keyCodes[0] = 0x08; // 'e'
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            keyCodes[0] = 0x0f; // 'l'
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            keyCodes[0] = 0x0;
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            keyCodes[0] = 0x0f; // 'l'
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            keyCodes[0] = 0x12; // 'o'
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            keyCodes[0] = 0x0;
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            
            // Toggle caps lock
            keyCodes[0] = 0x39; // caps lock
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);
            keyCodes[0] = 0x0;
            vmultic_update_keyboard(vmultic, shiftKeys, keyCodes);

            break;
        }

        case REPORTID_MESSAGE:
        {
            vmulticMessageReport report;

            printf("Writing vendor message report\n");

            memcpy(report.Message, "Hello vmultic\x00", 13);

            if (vmultic_write_message(vmultic, &report))
            {
                memset(&report, 0, sizeof(report));
                printf("Reading vendor message report\n");
                if (vmultic_read_message(vmultic, &report))
                {
                    printf("Success!\n    ");
                    printf(report.Message);
                }
            }

            break;
        }
    }
}


