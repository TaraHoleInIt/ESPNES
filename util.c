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

    printf( "\r\n" );
}


void PrintControllerInfo( struct Controller* C ) {
    int i = 0;

    printf( "Controller: %s\r\n", C->ControllerName );
    printf( "Buttons:\n" );

    for ( ; i < C->ButtonCount; i++ ) {
        printf( "\t%s\r\n", C->Buttons[ i ].Name );
    }

    printf( "\r\n" );
}

void PrintPressedButtons( struct Controller* C ) {
    int i = 0;

    printf( "%s buttons pressed: ", C->ControllerName );

    for ( ; i < C->ButtonCount; i++ ) {
        printf( "%s, ", ( C->Buttons[ i ].StateFn != NULL && C->Buttons[ i ].StateFn( ) == true ) ? C->Buttons[ i ].Name : "" );
    }

    printf( "\r\n" );
}
