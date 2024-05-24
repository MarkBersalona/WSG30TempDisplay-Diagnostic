/* 
 * File:   gconfig.h
 * Author: Mark Bersalona
 *
 * Created on 2024.05.22
 */

#ifndef GCONFIG_H
#define GCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////////////


// version number
#define VERSION_A     "0"
#define VERSION_B     "1"
#define VERSION_C     "0"
#define VERSION_DATE  "2024.05.24"
    
// Period of the periodic callback
#define MAIN_PERIODIC_INTERVAL_MSEC (250)

// Receive message FIFO
#define RECEIVE_FIFO_MSG_COUNT (200)
#define RECEIVE_FIFO_MSG_LENGTH_MAX (10000)
    

#ifdef __cplusplus
}
#endif

#endif /* GCONFIG_H */

