#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp/gpio.h>
#include <FreeRTOS.h>
#include <esp8266.h>
#include <stdio.h>
#include <stdint.h>

#include "controller.h"
#include "util.h"
#include "config.h"

#define LATCH_PULSE_US 12
#define CLK_PULSE_US 6

#define KEY_A BIT( 7 )
#define KEY_B BIT( 6 )
#define KEY_SELECT BIT( 5 )
#define KEY_START BIT( 4 )
#define KEY_UP BIT( 3 )
#define KEY_DOWN BIT( 2 )
#define KEY_LEFT BIT( 1 )
#define KEY_RIGHT BIT( 0 )

static bool Get_A( void );
static bool Get_B( void );
static bool Get_Start( void );
static bool Get_Select( void );
static bool Get_Up( void );
static bool Get_Down( void );
static bool Get_Left( void );
static bool Get_Right( void );
static bool Poll( void );
static bool Init( void );
static void Close( void );

static bool DPadAsCPad = false;
static uint8_t PadState = 0;

static int Pin_CLK = 14;
static int Pin_LATCH = 13;
static int Pin_DATA = 12;

static struct Button NESButtons[ ] = {
    { "A", Get_A },
    { "B", Get_B },
    { "Start", Get_Start },
    { "Select", Get_Select },
    { "Up", Get_Up },
    { "Down", Get_Down },
    { "Left", Get_Left },
    { "Right", Get_Right }
};

static struct ConfigOption NESConfig[ ] = {
    { "Map NES DPad to CPad", ConfigType_Bool, ( void* ) &DPadAsCPad },
    { "Clock GPIO pin", ConfigType_Int, ( void* ) &Pin_CLK },
    { "Latch GPIO pin", ConfigType_Int, ( void* ) &Pin_LATCH },
    { "Data GPIO pin", ConfigType_Int, ( void* ) &Pin_DATA }
};

struct Controller NESController = {
    "NES Controller",
    Init,
    Close,
    Poll,
    NESButtons,
    ( sizeof( NESButtons ) / sizeof( NESButtons[ 0 ] ) ),
    NESConfig,
    ( sizeof( NESConfig ) / sizeof( NESConfig[ 0 ] ) )
};

static bool Init( void ) {
    gpio_enable( Pin_CLK, GPIO_OUTPUT );
    gpio_enable( Pin_LATCH, GPIO_OUTPUT );
    gpio_enable( Pin_DATA, GPIO_INPUT );

    gpio_write( Pin_CLK, false );
    gpio_write( Pin_LATCH, false );    

    return true;
}

static void Close( void ) {
}

static bool Poll( void ) {
    bool HasChanged = false;
    uint8_t Result = 0;
    int i = 7;

    PulsePin( Pin_LATCH, LATCH_PULSE_US );

    do {
        Result |= ( gpio_read( Pin_DATA ) == false ) ? BIT( i ) : 0;

        PulsePin( Pin_CLK, CLK_PULSE_US );
        i--;
    } while ( i >= 0 );

    HasChanged = ( Result != PadState ) ? true : false;
    PadState = Result;

    return HasChanged;
}

static bool Get_A( void ) {
    return ( PadState & KEY_A );
}

static bool Get_B( void ) {
    return ( PadState & KEY_B );
}

static bool Get_Start( void ) {
    return ( PadState & KEY_START );
}

static bool Get_Select( void ) {
    return ( PadState & KEY_SELECT );
}

static bool Get_Up( void ) {
    return ( PadState & KEY_UP );
}

static bool Get_Down( void ) {
    return ( PadState & KEY_DOWN );
}

static bool Get_Left( void ) {
    return ( PadState & KEY_LEFT );
}

static bool Get_Right( void ) {
    return ( PadState & KEY_RIGHT );
}
