/* 
 * File:   serial.h
 * Author: Mark Bersalona
 *
 * Created on 2023.01.16
 */

#ifndef SERIAL_H
#define SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Public variables
//
///////////////////////////////////////////////////////////////////////////////

//
// serial-to-USB
//
extern gboolean isUSBConnectionOK;
extern gboolean isFirstSerialFail;

///////////////////////////////////////////////////////////////////////////////
//
// Public routines
//
///////////////////////////////////////////////////////////////////////////////

int serial_open(char *name, int baud);
int serial_write(char * paucMessage);
gboolean serial_read(GIOChannel *gio, GIOCondition condition, gpointer data); // GdkInputCondition condition )
gboolean serial_error(GIOChannel *gio, GIOCondition condition, gpointer data);



#ifdef __cplusplus
}
#endif

#endif /* SERIAL_H */

