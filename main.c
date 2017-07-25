#include <espressif/esp_common.h>
#include <espressif/esp_wifi.h>
#include <esp/uart.h>
#include <esp/gpio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <esp8266.h>
#include <stdio.h>
#include <stdint.h>

#include "controller.h"
#include "util.h"

static const int LED_Pin = 2;

extern struct Controller NESController;
struct Controller* CurrentController = &NESController;

void PrintPadTask( void* Param ) {
    bool HasChanged = false;

    while ( true ) {
        if ( CurrentController && CurrentController->Poll )
            HasChanged = CurrentController->Poll( );

        if ( HasChanged == true )
            PrintPressedButtons( CurrentController );

        gpio_toggle( LED_Pin );
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}

void user_init( void ) {
    uart_set_baud( 0, 115200 );
    
    printf( "Ready...\r\n" );

    gpio_enable( LED_Pin, GPIO_OUTPUT );
    gpio_write( LED_Pin, true );

    if ( CurrentController->Init != NULL && CurrentController->Init( ) == true ) {
        PrintControllerInfo( CurrentController );
        xTaskCreate( PrintPadTask, "PrintPadTask", 256, NULL, 2, NULL );
    }

    sdk_wifi_set_opmode( NULL_MODE );
}
