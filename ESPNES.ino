/*
 * 3DS Input redirection client for the ESP8266 and an NES controller.
 * 2017 - Tara Keeling
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#define BIT( n ) ( 1 << n )

#define WIFI_CONNECT_TIMEOUT 5000
#define SERIAL_BAUD 9600

#define BLINK_RATE_ERROR 100
#define BLINK_RATE_OK 1000

#define CLK_PIN 4
#define LATCH_PIN 5
#define DATA_PIN 12

#define NES_KEY_A BIT( 7 )
#define NES_KEY_B BIT( 6 )
#define NES_KEY_SELECT BIT( 5 )
#define NES_KEY_START BIT( 4 )
#define NES_KEY_UP BIT( 3 )
#define NES_KEY_DOWN BIT( 2 )
#define NES_KEY_LEFT BIT( 1 )
#define NES_KEY_RIGHT BIT( 0 )

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
 * Maps NES Keypad input (NESPadState) to the 3DS controller and
 * sends it out via UDP.
 */
void SendPadState( uint32_t NESPadState ) {
    uint32_t Buffer[ 5 ] = {
        0,  /* HID state */
        0,  /* CPad state */
        0,  /* CStick state */
        0,  /* Touch state */
        0   /* Special buttons */
    };
    uint32_t DSPadState = 0;

    DSPadState |= ( NESPadState & NES_KEY_UP ) ? DS_KEY_DUP : 0;
    DSPadState |= ( NESPadState & NES_KEY_DOWN ) ? DS_KEY_DDOWN : 0;
    DSPadState |= ( NESPadState & NES_KEY_LEFT ) ? DS_KEY_DLEFT : 0;
    DSPadState |= ( NESPadState & NES_KEY_RIGHT ) ? DS_KEY_DRIGHT : 0;
    DSPadState |= ( NESPadState & NES_KEY_START ) ? DS_KEY_START : 0;
    DSPadState |= ( NESPadState & NES_KEY_SELECT ) ? DS_KEY_SELECT : 0;
    DSPadState |= ( NESPadState & NES_KEY_A ) ? DS_KEY_A : 0;
    DSPadState |= ( NESPadState & NES_KEY_B ) ? DS_KEY_B : 0;
    
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

    for ( i = 0; i < TimeInUS; i++ ) {
        asm( "nop" );
    }
}

/* Pulse:
 * Brings a (Pin) high or low for (Delay) microseconds and back again.
 */
void Pulse( int Pin, int Delay ) {
  digitalWrite( Pin, HIGH );
  delay( Delay );
  digitalWrite( Pin, LOW );
}

/* ReadNESPad:
 * Returns a bitmask of all pressed keys on the NES keypad.
 *
 * Protocol info courtesy of: https://tresi.github.io/nes/
 */
uint32_t ReadNESPad( void ) {
    const int LatchPulseDelay = 2;
    const int ClockPulseDelay = 1;
    uint32_t Result = 0;
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

    return Result;
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
    pinMode( CLK_PIN, OUTPUT );
    pinMode( LATCH_PIN, OUTPUT );
    pinMode( DATA_PIN, INPUT );

    digitalWrite( LED_BUILTIN, LOW );
    digitalWrite( CLK_PIN, LOW );
    digitalWrite( LATCH_PIN, LOW );
    //digitalWrite( DATA_PIN, LOW );

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

void PrintBits( uint32_t Value, int Count ) {
    int i = 0;

    for ( i = Count -1; i >= 0; i-- ) {
        Serial.print( ( Value & BIT( i ) ) ? 1 : 0 );
    }

    Serial.println( );
}

void loop( void ) {
    uint32_t PADState = 0;
    int i = 0;

    if ( InitSuccessful == true ) {
        while ( true ) {
            PADState = ReadNESPad( );
            PrintBits( PADState, 8 );

            /* TODO:
             * Stuff.
             */
            delay( 1000 );
        }
    }

    /* Keep the watchdog timer happy.
     * bork bork
     */
    yield( );
}
