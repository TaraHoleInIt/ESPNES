/*
 * 3DS Input redirection client for the ESP8266 and an NES controller.
 * 2017 - Tara Keeling
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include "controller.h"
#include "nes.h"

NESController NES;

#define BIT( n ) ( 1 << n )

#define WIFI_CONNECT_TIMEOUT 5000
#define SERIAL_BAUD 9600

#define BLINK_RATE_ERROR 100
#define BLINK_RATE_OK 1000

#define DS_KEY_A BIT( 0 )
#define DS_KEY_B BIT( 1 )
#define DS_KEY_SELECT BIT( 2 )
#define DS_KEY_START BIT( 3 )
#define DS_KEY_DRIGHT BIT( 4 )
#define DS_KEY_DLEFT BIT( 5 )
#define DS_KEY_DUP BIT( 6 )
#define DS_KEY_DDOWN BIT( 7 )

const char* const TargetIP = "192.168.2.205";
const uint16_t TargetPort = 4950;

const char* const RouterSSID = "";
const char* const RouterPSK = "";

Controller* CurrentController = NULL;
Ticker LEDStatusTicker;
WiFiUDP UDPClient;

bool InitSuccessful = false;

void SendPadState( void );
void uDelay( int TimeInUS );
void Pulse( int Pin, int Delay );
uint32_t ReadNESPad( void );
void BlinkLED( void );
void InitSerial( void );
void TryWifiConnect( void );

/* SendPadState:
 * Maps controller input to the 3DS HID and sends it along
 * via UDP.
 */
void SendPadState( void ) {
    uint32_t Buffer[ 5 ] = {
        0x0,            /* HID state */
        0x800800,       /* CPad state */
        0x800800,       /* CStick state */
        0x2000000,      /* Touch state */
        0               /* Special buttons */
    };
    uint32_t DSPadState = 0;
  
    if ( CurrentController->A( ) ) DSPadState |= DS_KEY_A;
    if ( CurrentController->B( ) ) DSPadState |= DS_KEY_B;

    if ( CurrentController->Start( ) ) DSPadState |= DS_KEY_START;
    if ( CurrentController->Select( ) ) DSPadState |= DS_KEY_SELECT;

    if ( CurrentController->Up( ) ) DSPadState |= DS_KEY_DUP;
    if ( CurrentController->Down( ) ) DSPadState |= DS_KEY_DDOWN;
    if ( CurrentController->Left( ) ) DSPadState |= DS_KEY_DLEFT;
    if ( CurrentController->Right( ) ) DSPadState |= DS_KEY_DRIGHT;
    
    Buffer[ 0 ] = ~DSPadState;

    /* Send our keypad state to the input redirection host */
    UDPClient.beginPacket( TargetIP, TargetPort );
    UDPClient.write( ( byte* ) Buffer, sizeof( Buffer ) );
    UDPClient.endPacket( );
}

/* uDelay:
 * Rough delay in microseconds.
 * Probably really, really, really, rough.
 */
void uDelay( int TimeInUS ) {
    const int NOPsPerSecond = 16000000;
    const int NOPsPerUSec = NOPsPerSecond / 1000000;
    int i = 0;

    for ( i = 0; i < ( TimeInUS * NOPsPerUSec ); i++ ) {
        asm( "nop" );
    }
}

/* Pulse:
 * Brings a (Pin) high or low for (Delay) microseconds and back again.
 */
void Pulse( int Pin, int Delay ) {
  digitalWrite( Pin, HIGH );
  uDelay( Delay );
  
  digitalWrite( Pin, LOW );
  uDelay( Delay );
}

/* BlinkLED:
 * Just alternates the built-in LED between off and on states.
 */
void BlinkLED( void ) {
    static bool LEDState = false;
    
    digitalWrite( LED_BUILTIN, LEDState ? LOW : HIGH );
    LEDState = ! LEDState;
}

/* InitSerial:
 * Does what it says on the tin, initializes the UART.
 */
void InitSerial( void ) {
    Serial.begin( SERIAL_BAUD );
    delay( 100 );

    Serial.print( "Ready...\n" );
}

/* TryWifiConnect:
 * Attempts to connect to the router specified in RouterSSID using the PSK in RouterPSK.
 * It will try until WIFI_CONNECT_TIMEOUT milliseconds has been reached.
 *
 * If connection is successful it will write the local IP address to the serial console and
 * the built in LED will blink once per second.
 *
 * If the connection failed it will write an error message to the serial console and
 * the built in LED will blink every BLINK_RATE_ERROR milliseconds.
 */
void TryWifiConnect( void ) {
    unsigned long TimeToTimeout = millis( ) + WIFI_CONNECT_TIMEOUT;

    Serial.printf( "Attempting to connect to \"%s\"\n", RouterSSID );

    WiFi.begin( RouterSSID, RouterPSK );

    while ( millis( ) < TimeToTimeout ) {
        if ( WiFi.status( ) == WL_CONNECTED )
            break;

        Serial.printf( "." );
        delay( 250 );
    }

    Serial.printf( "\n" );

    InitSuccessful = WiFi.status( ) == WL_CONNECTED ? true : false;

    if ( InitSuccessful == false ) {
        Serial.printf( "Failed to connect to \"%s\" after %d seconds.\n", RouterSSID, ( WIFI_CONNECT_TIMEOUT / 1000 ) );
        Serial.printf( "Check your settings and try again.\n" );
    } else {
        Serial.printf( "Connected to \"%s\"\n", RouterSSID );
        Serial.printf( "Local IP: %s\n", WiFi.localIP( ).toString( ).c_str( ) );
    }
}

void setup( void ) {
    pinMode( LED_BUILTIN, OUTPUT );
    digitalWrite( LED_BUILTIN, LOW );

    Controller* CurrentController = ( Controller* ) &NES;
    CurrentController->Init( );

    InitSerial( );
    TryWifiConnect( );

    /* Blink the LED with the speed depending on whether we connected to the router
     * successfully.
     */
    LEDStatusTicker.attach_ms( InitSuccessful ? BLINK_RATE_OK : BLINK_RATE_ERROR, BlinkLED );

    if ( InitSuccessful == true ) {
        UDPClient.begin( TargetPort );
    }
}


void loop( void ) {
    if ( InitSuccessful == true ) {
        while ( true ) {
            if ( CurrentController->Poll( ) ) {
                SendPadState( );
            }

            delay( 16 );
        }
    }

    /* Keep the watchdog timer happy.
     * bork bork
     */
    yield( );
}
