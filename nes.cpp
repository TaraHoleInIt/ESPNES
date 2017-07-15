#include <ESP8266WiFi.h>
#include "controller.h"
#include "nes.h"

#define BIT( n ) ( 1 << n )

#define CLK_PIN D4
#define LATCH_PIN D5
#define DATA_PIN D6

#define KEY_A BIT( 7 )
#define KEY_B BIT( 6 )
#define KEY_SELECT BIT( 5 )
#define KEY_START BIT( 4 )
#define KEY_UP BIT( 3 )
#define KEY_DOWN BIT( 2 )
#define KEY_LEFT BIT( 1 )
#define KEY_RIGHT BIT( 0 )

extern void Pulse( int Pin, int Delay );
extern void uDelay( int TimeInUS );

bool NESController::Init( void ) {
    PadState = 0;

    pinMode( CLK_PIN, OUTPUT );
    pinMode( LATCH_PIN, OUTPUT );
    pinMode( DATA_PIN, INPUT );

    digitalWrite( CLK_PIN, LOW );
    digitalWrite( LATCH_PIN, LOW );

    return true;
}

bool NESController::Poll( void ) {
    const int LatchPulseDelay = 12;
    const int ClockPulseDelay = 6;
    bool HasChanged = false;
    uint8_t Result = 0;
    int Data = 0;
    int i = 0;

    /* After this pulse the data pin will go high until 8 bits have been clocked out */
    Pulse( LATCH_PIN, LatchPulseDelay );

    /* Read the first bit which happens to be the A button */
    Data = digitalRead( DATA_PIN );
    Result |= ( Data == LOW ) ? BIT( 7 ) : 0;

    for ( i = 6; i >= 0; i-- ) {
        /* Pulse the clock pin and get the next bit of data, repeat */
        Pulse( CLK_PIN, ClockPulseDelay );
        Data = digitalRead( DATA_PIN );
       
        /* A Button is pressed when the data pin is pulled low */
        Result |= ( Data == LOW ) ? BIT( i ) : 0;
    }

    HasChanged = PadState != Result;
    PadState = Result;

    //Serial.printf( "ms: %d, res: %x, ps: %x\n", millis( ), Result, PadState );

    return HasChanged;
}

bool NESController::A( void ) {
    return ( PadState & KEY_A ) ? true : false;
}

bool NESController::B( void ) {
    return ( PadState & KEY_B ) ? true : false;
}

bool NESController::Start( void ) {
    return ( PadState & KEY_START ) ? true : false;
}

bool NESController::Select( void ) {
    return ( PadState & KEY_SELECT ) ? true : false;
}

bool NESController::Up( void ) {
    return ( PadState & KEY_UP ) ? true : false;
}

bool NESController::Down( void ) {
    return ( PadState & KEY_DOWN ) ? true : false;
}

bool NESController::Left( void ) {
    return ( PadState & KEY_LEFT ) ? true : false;
}

bool NESController::Right( void ) {
    return ( PadState & KEY_RIGHT ) ? true : false;
}
