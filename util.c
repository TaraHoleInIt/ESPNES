#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp/gpio.h>
#include <FreeRTOS.h>
#include <esp8266.h>
#include <stdio.h>
#include <stdint.h>

#include "controller.h"
#include "util.h"

void PulsePin( int Pin, int DelayUS ) {
    gpio_toggle( Pin );
    sdk_os_delay_us( DelayUS );

    gpio_toggle( Pin );
    sdk_os_delay_us( DelayUS );
}

void PrintByte( uint8_t Data ) {
    int i = 7;

    printf( "%s: ", __FUNCTION__ );
    
    for ( ; i >= 0; i-- ) {
        uart_putc( 0, ( Data & BIT( i ) ) ? '1' : '0' );
    }

    printf( "\n" );
}

#if 0
static const char* ConfigTypeToStr( int Type ) {
    const char* Types[ ] = {
        "Undefined",
        "Boolean",
        "Integer",
        "String"
    };

    return ( Type < ConfigType_NumTypes ) ? Types[ Type ] : "";
}
#endif

static size_t ConfigVarToString( struct ConfigOption* C, char* OutStr, size_t OutStrLen ) {
    char* StringValue = ( char* ) C->VariablePtr;
    bool* BoolValue = ( bool* ) C->VariablePtr;
    int* IntValue = ( int* ) C->VariablePtr;
    size_t Result = 0;

    if ( C && C->VariablePtr ) {
        switch ( C->ConfigType ) {
            case ConfigType_Bool:
                Result = snprintf( OutStr, OutStrLen, "%s", ( *BoolValue == false ) ? "false" : "true" );
                break;
            case ConfigType_Int:
                Result = snprintf( OutStr, OutStrLen, "%d", *IntValue );
                break;
            case ConfigType_String:
                Result = snprintf( OutStr, OutStrLen, "%s", StringValue );
                break;
            default:
                Result = snprintf( OutStr, OutStrLen, "Undefined" );
                break;
        };
    }

    return Result;
}

void PrintControllerInfo( struct Controller* C ) {
    char ValueStr[ 64 ];
    int i = 0;

    printf( "Controller: %s\n", C->ControllerName );
    printf( "Buttons:\n" );

    for ( ; i < C->ButtonCount; i++ ) {
        printf( "\t%s\n", C->Buttons[ i ].Name );
    }

    printf( "Configuration options:\n" );

    for ( i = 0; i < C->OptionsCount; i++ ) {
        ConfigVarToString( &C->Options[ i ], ValueStr, sizeof( ValueStr ) );
        printf( "\t%s: %s\n", C->Options[ i ].OptionName, ValueStr );
    }

    printf( "\n" );
}

void PrintPressedButtons( struct Controller* C ) {
    int i = 0;

    printf( "%s buttons pressed: ", C->ControllerName );

    for ( ; i < C->ButtonCount; i++ ) {
        printf( "%s, ", ( C->Buttons[ i ].StateFn != NULL && C->Buttons[ i ].StateFn( ) == true ) ? C->Buttons[ i ].Name : "" );
    }

    printf( "\n" );
}
