/*
 * File:   main.h
 * Author: Mark Bersalona
 *
 * Created on 2023.01.16
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////////////

// Limits for data age
#define DATA_TIMEOUT_MIN_MINUTES (60 * 2)
#define DATA_TIMEOUT_MAX_MINUTES (60 * 120)
#define DATA_TIMEOUT_MIN_HOURS   (DATA_TIMEOUT_MAX_MINUTES)
#define DATA_TIMEOUT_MAX_HOURS   (60 * 60 * 48)
#define DATA_TIMEOUT_MIN_DAYS    (DATA_TIMEOUT_MAX_HOURS)
#define DATA_TIMEOUT_MAX_DAYS    (60 * 60 * 24 * 64)
#define DATA_TIMEOUT_MIN_MONTHS  (DATA_TIMEOUT_MAX_DAYS)
#define DATA_TIMEOUT_MAX_MONTHS  (60 * 60 * 24 * 30 * 24)
#define DATA_TIMEOUT_MIN_YEARS   (DATA_TIMEOUT_MAX_MONTHS)

// Alarms, from Windchime inputs.h
#define VAL_ALARM_NONE               0x00
#define MSK_ALARM_LOW                0x01
#define MSK_ALARM_HIGH               0x02
#define MSK_ALARM_BATTERY            0x04 // Wireless
#define MSK_ALARM_RESPONSE           0x08 // IMS and Wireless
#define VAL_RANGE_OK                 0x00
#define VAL_RANGE_OFF                0x00 // Outputs: Digital, Analog, M2M
#define VAL_RANGE_ALARM              0x01
#define VAL_RANGE_CLOSED             0x01
#define VAL_RANGE_LOW                0x01
#define VAL_RANGE_ON                 0x01 // Outputs: Digital, Analog, M2M
#define VAL_RANGE_OPEN               0x02
#define VAL_RANGE_HIGH               0x02
#define VAL_RANGE_CYCLE              0x02 // Outputs: Digital, Analog, M2M
#define VAL_RANGE_CONNECTION_PENDING 0x03 // Wireless TODO convert to bitmask-compatible value, e.g. 0x08
#define VAL_RANGE_NOT_RESPONDING     0x04 // IMS and Wireless
#define VAL_RANGE_ROUTE_DOWN         0x05 // IP TODO convert to bitmask-compatible value, e.g. 0x10

///////////////////////////////////////////////////////////////////////////////
//
// Public routines
//
///////////////////////////////////////////////////////////////////////////////
char *main_receive_msg_read(void);
void main_receive_msg_write(char *paucReceiveMsg);

void main_ATCommand_clicked(void);
void main_BOARDREV_clicked(void);
void main_LOGENABLE_state_set(void);
void main_MAC_clicked(void);
void main_MENU_clicked(void);
void main_REBOOT_clicked(void);
void main_RTD_clicked(void);

gboolean is_valid_mac(char *paucTestMAC);
char *trim(char *paucInputString);
int main_logfile_write(char * paucMessage);

///////////////////////////////////////////////////////////////////////////////
//
// Public variables
//
///////////////////////////////////////////////////////////////////////////////

// GTK builder
extern GtkBuilder *builder;

// Elapsed time since last data update
extern guint32 gulElapsedTimeSinceDataUpdate_sec;

// I/O channel for serial-to-USB port
extern GIOChannel *gIOChannelSerialUSB;

// Status timestamp-related
extern guint16 guiStatusTimestampCountdown_minutes;
extern guint8  guiStatusTimestampCountdownIndex;

// Sticky error status
extern char gucStickyErrorStatus[200];
extern char gucStickyErrorStatusOLD[200];

// Device initial and current timestamps
extern guint32 gulDeviceStartTimestamp;
extern guint32 gulDeviceCurrentTimestamp;
extern guint32 gulDeviceRuntime_seconds;


#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */
