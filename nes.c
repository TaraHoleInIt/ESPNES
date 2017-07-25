#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp/gpio.h>
#include <FreeRTOS.h>
#include <esp8266.h>
#include <stdio.h>
#include <stdint.h>

#include "controller.h"
#include "util.h"

#define PIN_NES_CLK 14
#define PIN_NES_LATCH 13
#define PIN_NES_DATA 12

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

static uint8_t PadState = 0;

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

struct Controller NESController = {
    "NES Controller",
    Init,
    Close,
    Poll,
    NESButtons,
    ( sizeof( NESButtons ) / sizeof( NESButtons[ 0 ] ) ),
};

static bool Init( void ) {
    gpio_enable( PIN_NES_CLK, GPIO_OUTPUT );
    gpio_enable( PIN_NES_LATCH, GPIO_OUTPUT );
    gpio_enable( PIN_NES_DATA, GPIO_INPUT );

    gpio_write( PIN_NES_CLK, false );
    gpio_write( PIN_NES_LATCH, false );    

    return true;
}

static void Close( void ) {
}

static bool Poll( void ) {
    bool HasChanged = false;
    uint8_t Result = 0;
    int i = 7;

    PulsePin( PIN_NES_LATCH, LATCH_PULSE_US );

    do {
        Result |= ( gpio_read( PIN_NES_DATA ) == false ) ? BIT( i ) : 0;

        PulsePin( PIN_NES_CLK, CLK_PULSE_US );
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
