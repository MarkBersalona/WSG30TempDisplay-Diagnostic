/* 
 * File:   display.h
 * Author: Mark Bersalona
 *
 * Created on 2023.01.16
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Public variables
//
///////////////////////////////////////////////////////////////////////////////

// Main display
extern GtkCssProvider *cssProvider;
extern GtkWindow *window;

extern GtkWidget *lblTimestampTitle, *lblAlarmHITitle, *lblAlarmLOTitle;
extern GtkWidget *lblTimestamp, *lblAlarmHI, *lblAlarmLO;
extern GtkWidget *lblAlarmTitle, *lblSampleRateTitle, *lblACKTitle, *lblXBeeSNTitle;
extern GtkWidget *lblAlarm, *lblSampleRate, *lblACK, *lblXBeeSN;
extern GtkWidget *lblDeviceTitle, *lblConnectionTitle, *lblFWVerTitle;
extern GtkWidget *lblDevice, *lblConnection, *lblFWVer;

extern GtkWidget *lblHostCalTitle, *lblBuzzerTitle;
extern GtkWidget *lblHostCal, *lblBuzzer;

extern GtkWidget *lblPANIDTitle, *lblChannelTitle;
extern GtkWidget *lblPANID, *lblChannel;

extern GtkWidget *lblCalDateTitle, *lblSerialNumTitle, *lblPCBRevTitle, *lblVrefTitle;
extern GtkWidget *txtentCalDate, *txtentSerialNum, *txtentPCBRev, *txtentVref;
extern GtkWidget *btnCalDate, *btnSerialNum, *btnPCBRev, *btnVref;

extern GtkWidget *cbtMenu, *btnMenu;
extern GtkWidget *btnRTD, *btnReboot;

extern GtkWidget *lblBatteryVoltageTitle, *lblElapsedTimeTitle, *lblBatteryPercentageTitle, *lblMainsTitle;
extern GtkWidget *lblBatteryVoltage, *lblElapsedTime, *lblBatteryPercentage, *lblMains;

extern GtkWidget *lblMinimumTitle, *lblTemperatureTitle, *lblMaximumTitle;
extern GtkWidget *lblMinimum, *lblTemperature, *lblMaximum;

extern GtkWidget *lblHostCalTitle, *lblBuzzerTitle;
extern GtkWidget *lblHostCal, *lblBuzzer;

extern GtkWidget *swLogfileEnable, *lblLogfile;

extern GtkWidget *lblStatusTitle;
extern GtkTextBuffer *textbufStatus;
extern GtkTextBuffer *textbufReceive;
extern GtkTextIter textiterReceiveStart;
extern GtkTextIter textiterReceiveEnd;
extern GtkTextIter textiterStatusStart;
extern GtkTextIter textiterStatusEnd;
extern GtkScrolledWindow *scrolledwindowStatus;
extern GtkAdjustment *adjStatus;

    
// Menu tables
extern char* pucMenuItems[];
extern char* pucMenuCMD[];

///////////////////////////////////////////////////////////////////////////////
//
// Public routines
//
///////////////////////////////////////////////////////////////////////////////

void display_clear_UUT_values(void);
void display_diagnostics_enter(GtkWindow *parent);
void display_diagnostics_exit(void);
void display_main_initialize(void);
void display_receive_write(char * paucWriteBuf);
void display_status_write(char * paucWriteBuf);
void display_update_zones(void);
void display_update_display_connection(void);
void display_update_data_age(void);



#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H */

