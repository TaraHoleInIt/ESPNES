#ifndef _UTIL_H_
#define _UTIL_H_

void PulsePin( int Pin, int DelayUS );
void PrintByte( uint8_t Data );

void PrintControllerInfo( struct Controller* C );
void PrintPressedButtons( struct Controller* C );


#endif
