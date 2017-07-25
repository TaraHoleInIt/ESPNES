#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "config.h"

typedef bool ( *GetButtonState ) ( void );
typedef bool ( *GetAxisState ) ( int* DX, int* DY );
typedef bool ( *PollCallback ) ( void );

struct Button {
    const char* Name;
    GetButtonState StateFn;
};

struct Controller {
    const char* ControllerName;

    bool ( *Init ) ( void );
    void ( *Close ) ( void );
    bool ( *Poll ) ( void );

    struct Button* Buttons;
    int ButtonCount;

    struct ConfigOption* Options;
    int OptionsCount;
};

#endif
