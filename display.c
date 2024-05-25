/* 
 * File:   display.c
 * Author: Mark Bersalona
 *
 * Created on 2023.01.16
 * Sensaphone 400 Cellular Diagnostic
 */


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
///////////////////////////////////////////////////////////////////////////////

#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gconfig.h"
#include "main.h"
#include "serial.h"
#include "display.h"

///////////////////////////////////////////////////////////////////////////////
//
// Variables and tables
//
///////////////////////////////////////////////////////////////////////////////

// Main display
GtkCssProvider *cssProvider;
GtkWindow *window;

GtkWidget *lblTimestampTitle, *lblAlarmHITitle, *lblAlarmLOTitle;
GtkWidget *lblTimestamp, *lblAlarmHI, *lblAlarmLO;
GtkWidget *lblAlarmTitle, *lblSampleRateTitle, *lblACKTitle, *lblXBeeSNTitle;
GtkWidget *lblAlarm, *lblSampleRate, *lblACK, *lblXBeeSN;
GtkWidget *lblDeviceTitle, *lblConnectionTitle, *lblFWVerTitle;
GtkWidget *lblDevice, *lblConnection, *lblFWVer;

GtkWidget *lblHostCalTitle, *lblBuzzerTitle;
GtkWidget *lblHostCal, *lblBuzzer;

GtkWidget *lblPANIDTitle, *lblChannelTitle;
GtkWidget *lblPANID, *lblChannel;

GtkWidget *lblCalDateTitle, *lblSerialNumTitle, *lblPCBRevTitle, *lblVrefTitle;
GtkWidget *txtentCalDate, *txtentSerialNum, *txtentPCBRev, *txtentVref;
GtkWidget *btnCalDate, *btnSerialNum, *btnPCBRev, *btnVref;

GtkWidget *cbtMenu, *btnMenu;
GtkWidget *btnRTD, *btnReboot;

GtkWidget *lblBatteryVoltageTitle, *lblElapsedTimeTitle, *lblBatteryPercentageTitle, *lblMainsTitle;
GtkWidget *lblBatteryVoltage, *lblElapsedTime, *lblBatteryPercentage, *lblMains;

GtkWidget *lblMinimumTitle, *lblTemperatureTitle, *lblMaximumTitle;
GtkWidget *lblMinimum, *lblTemperature, *lblMaximum;

GtkWidget *lblHostCalTitle, *lblBuzzerTitle;
GtkWidget *lblHostCal, *lblBuzzer;

GtkWidget *lblStatusTitle, *textviewStatus;
GtkWidget *lblReceiveTitle, *lblLogfileTitle, *swLogfileEnable, *lblLogfile;
GtkWidget *textviewReceive;

// Receive view
GtkScrolledWindow *scrolledwindowReceive;
GtkAdjustment *adjReceive;
GtkTextBuffer *textbufReceive;
GtkTextIter textiterReceiveStart;
GtkTextIter textiterReceiveEnd;
// GtkTextMark *textmarkReceive;

// Status view
GtkScrolledWindow *scrolledwindowStatus;
GtkAdjustment *adjStatus;
GtkTextBuffer *textbufStatus;
GtkTextIter textiterStatusStart;
GtkTextIter textiterStatusEnd;
// GtkTextMark *textmarkStatus;

// Menu table
char* pucMenuItems[] =
{
    "Display menu",
    "Toggle 5sec status",
    "Buzzer",
    "Read cal date",
    "Toggle LCD blink",

    "Music notes",
    "Read SN",
    "Read PCB rev",
    "Read Vref",
    "RESET to defaults",

    "REBOOT",
    "ENDOFMENU"
};
char* pucMenuCMD[] =
{
    "0", // Display menu
    "1", // Toggle 5-second periodic status
    "B", // Buzzer test
    "d", // Read calibration date
    "L", // Toggle LCD blink

    "M", // Play musical notes
    "n", // Read serial number
    "p", // Read PCB revision
    "v", // Read Vref (mV)
    "X", // RESET to defaults

    "Z", // REBOOT
    "-", // end of menu
 
    
};

char lcTempString[40];

////////////////////////////////////////////////////////////////////////////
// Name:         display_main_initialize
// Description:  Initialize Main window
// Parameters:   None
// Return:       None
////////////////////////////////////////////////////////////////////////////
void
display_main_initialize(void)
{
    //
    // Connect widgets
    //
    // Main display labels
    lblFWVerTitle             = GTK_WIDGET(gtk_builder_get_object(builder, "lblFWVerTitle"));
    lblFWVer                  = GTK_WIDGET(gtk_builder_get_object(builder, "lblFWVer"));
    lblTimestampTitle         = GTK_WIDGET(gtk_builder_get_object(builder, "lblTimestampTitle"));
    lblTimestamp              = GTK_WIDGET(gtk_builder_get_object(builder, "lblTimestamp"));
    lblElapsedTimeTitle       = GTK_WIDGET(gtk_builder_get_object(builder, "lblElapsedTimeTitle"));
    lblElapsedTime            = GTK_WIDGET(gtk_builder_get_object(builder, "lblElapsedTime"));
    lblBatteryVoltageTitle    = GTK_WIDGET(gtk_builder_get_object(builder, "lblBatteryVoltageTitle"));
    lblBatteryVoltage         = GTK_WIDGET(gtk_builder_get_object(builder, "lblBatteryVoltage"));
    lblBatteryPercentageTitle = GTK_WIDGET(gtk_builder_get_object(builder, "lblBatteryPercentageTitle"));
    lblBatteryPercentage      = GTK_WIDGET(gtk_builder_get_object(builder, "lblBatteryPercentage"));
    lblMainsTitle             = GTK_WIDGET(gtk_builder_get_object(builder, "lblMainsTitle"));
    lblMains                  = GTK_WIDGET(gtk_builder_get_object(builder, "lblMains"));

    lblAlarmHITitle    = GTK_WIDGET(gtk_builder_get_object(builder, "lblAlarmHITitle"));
    lblAlarmHI         = GTK_WIDGET(gtk_builder_get_object(builder, "lblAlarmHI"));
    lblAlarmLOTitle    = GTK_WIDGET(gtk_builder_get_object(builder, "lblAlarmLOTitle"));
    lblAlarmLO         = GTK_WIDGET(gtk_builder_get_object(builder, "lblAlarmLO"));
    lblAlarmTitle      = GTK_WIDGET(gtk_builder_get_object(builder, "lblAlarmTitle"));
    lblAlarm           = GTK_WIDGET(gtk_builder_get_object(builder, "lblAlarm"));
    lblSampleRateTitle = GTK_WIDGET(gtk_builder_get_object(builder, "lblSampleRateTitle"));
    lblSampleRate      = GTK_WIDGET(gtk_builder_get_object(builder, "lblSampleRate"));
    lblACKTitle        = GTK_WIDGET(gtk_builder_get_object(builder, "lblACKTitle"));
    lblACK             = GTK_WIDGET(gtk_builder_get_object(builder, "lblACK"));
    lblHostCalTitle    = GTK_WIDGET(gtk_builder_get_object(builder, "lblHostCalTitle"));
    lblHostCal         = GTK_WIDGET(gtk_builder_get_object(builder, "lblHostCal"));
    lblBuzzerTitle     = GTK_WIDGET(gtk_builder_get_object(builder, "lblBuzzerTitle"));
    lblBuzzer          = GTK_WIDGET(gtk_builder_get_object(builder, "lblBuzzer"));

    lblXBeeSNTitle     = GTK_WIDGET(gtk_builder_get_object(builder, "lblXBeeSNTitle"));
    lblXBeeSN          = GTK_WIDGET(gtk_builder_get_object(builder, "lblXBeeSN"));
    lblDeviceTitle     = GTK_WIDGET(gtk_builder_get_object(builder, "lblDeviceTitle"));
    lblDevice          = GTK_WIDGET(gtk_builder_get_object(builder, "lblDevice"));
    lblPANIDTitle      = GTK_WIDGET(gtk_builder_get_object(builder, "lblPANIDTitle"));
    lblPANID           = GTK_WIDGET(gtk_builder_get_object(builder, "lblPANID"));
    lblChannelTitle    = GTK_WIDGET(gtk_builder_get_object(builder, "lblChannelTitle"));
    lblChannel         = GTK_WIDGET(gtk_builder_get_object(builder, "lblChannel"));
    lblConnectionTitle = GTK_WIDGET(gtk_builder_get_object(builder, "lblConnectionTitle"));
    lblConnection      = GTK_WIDGET(gtk_builder_get_object(builder, "lblConnection"));


    lblCalDateTitle   = GTK_WIDGET(gtk_builder_get_object(builder, "lblCalDateTitle"));
    txtentCalDate     = GTK_WIDGET(gtk_builder_get_object(builder, "txtentCalDate"));
    btnCalDate        = GTK_WIDGET(gtk_builder_get_object(builder, "btnCalDate"));
    lblSerialNumTitle = GTK_WIDGET(gtk_builder_get_object(builder, "lblSerialNumTitle"));
    txtentSerialNum   = GTK_WIDGET(gtk_builder_get_object(builder, "txtentSerialNum"));
    btnSerialNum      = GTK_WIDGET(gtk_builder_get_object(builder, "btnSerialNum"));
    lblPCBRevTitle    = GTK_WIDGET(gtk_builder_get_object(builder, "lblPCBRevTitle"));
    txtentPCBRev      = GTK_WIDGET(gtk_builder_get_object(builder, "txtentPCBRev"));
    btnPCBRev         = GTK_WIDGET(gtk_builder_get_object(builder, "btnPCBRev"));
    lblVrefTitle      = GTK_WIDGET(gtk_builder_get_object(builder, "lblVrefTitle"));
    txtentVref        = GTK_WIDGET(gtk_builder_get_object(builder, "txtentVref"));
    btnVref           = GTK_WIDGET(gtk_builder_get_object(builder, "btnVref"));

    lblMinimumTitle     = GTK_WIDGET(gtk_builder_get_object(builder, "lblMinimumTitle"));
    lblMinimum          = GTK_WIDGET(gtk_builder_get_object(builder, "lblMinimum"));
    lblTemperatureTitle = GTK_WIDGET(gtk_builder_get_object(builder, "lblTemperatureTitle"));
    lblTemperature      = GTK_WIDGET(gtk_builder_get_object(builder, "lblTemperature"));
    lblMaximumTitle     = GTK_WIDGET(gtk_builder_get_object(builder, "lblMaximumTitle"));
    lblMaximum         = GTK_WIDGET(gtk_builder_get_object(builder, "lblMaximum"));

    cbtMenu             = GTK_WIDGET(gtk_builder_get_object(builder, "cbtMenu"));
    btnMenu             = GTK_WIDGET(gtk_builder_get_object(builder, "btnMenu"));
		
    btnRTD              = GTK_WIDGET(gtk_builder_get_object(builder, "btnRTD"));
    btnReboot           = GTK_WIDGET(gtk_builder_get_object(builder, "btnReboot"));
		
    lblStatusTitle  = GTK_WIDGET(gtk_builder_get_object(builder, "lblStatusTitle"));
    textviewStatus  = GTK_WIDGET(gtk_builder_get_object(builder, "textviewStatus"));
		
    lblReceiveTitle = GTK_WIDGET(gtk_builder_get_object(builder, "lblReceiveTitle"));
    lblLogfileTitle = GTK_WIDGET(gtk_builder_get_object(builder, "lblLogfileTitle"));
    swLogfileEnable = GTK_WIDGET(gtk_builder_get_object(builder, "swLogfileEnable"));
    lblLogfile      = GTK_WIDGET(gtk_builder_get_object(builder, "lblLogfile"));
    textviewReceive = GTK_WIDGET(gtk_builder_get_object(builder, "textviewReceive"));
		
    // Receive text buffer
    scrolledwindowReceive = GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "scrolledwindow2"));
    textbufReceive        = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textviewReceive));
    gtk_text_buffer_get_start_iter(textbufReceive, &textiterReceiveStart);
    gtk_text_buffer_get_end_iter  (textbufReceive, &textiterReceiveEnd);
    // gtk_text_buffer_add_mark(textbufStatus, textmarkStatus, textiterStatusEnd);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textviewReceive), TRUE);
    
    // Status text buffer
    scrolledwindowStatus = GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "scrolledwindow1"));
    textbufStatus  = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textviewStatus));
    gtk_text_buffer_get_start_iter(textbufStatus, &textiterStatusStart);
    gtk_text_buffer_get_end_iter  (textbufStatus, &textiterStatusEnd);
    // gtk_text_buffer_add_mark(textbufStatus, textmarkStatus, textiterStatusEnd);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textviewStatus), TRUE);
    
    // 
    // Initialize stylings
    //

    // Titles
    gtk_widget_set_name((lblFWVerTitle),             "DiagnosticsTitle");
    gtk_widget_set_name((lblTimestampTitle),         "DiagnosticsTitle");
    gtk_widget_set_name((lblElapsedTimeTitle),       "DiagnosticsTitle");
    gtk_widget_set_name((lblBatteryVoltageTitle),    "DiagnosticsTitle");
    gtk_widget_set_name((lblBatteryPercentageTitle), "DiagnosticsTitle");
    gtk_widget_set_name((lblMainsTitle),             "DiagnosticsTitle");

    gtk_widget_set_name((lblAlarmHITitle),    "DiagnosticsTitle");
    gtk_widget_set_name((lblAlarmLOTitle),    "DiagnosticsTitle");
    gtk_widget_set_name((lblAlarmTitle),      "DiagnosticsTitle");
    gtk_widget_set_name((lblSampleRateTitle), "DiagnosticsTitle");
    gtk_widget_set_name((lblACKTitle),        "DiagnosticsTitle");
    gtk_widget_set_name((lblHostCalTitle),    "DiagnosticsTitle");
    gtk_widget_set_name((lblBuzzerTitle),     "DiagnosticsTitle");

    gtk_widget_set_name((lblXBeeSNTitle),     "DiagnosticsTitle");
    gtk_widget_set_name((lblDeviceTitle),     "DiagnosticsTitle");
    gtk_widget_set_name((lblPANIDTitle),      "DiagnosticsTitle");
    gtk_widget_set_name((lblChannelTitle),    "DiagnosticsTitle");
    gtk_widget_set_name((lblConnectionTitle), "DiagnosticsTitle");

    gtk_widget_set_name((lblCalDateTitle),   "DiagnosticsTitle");
    gtk_widget_set_name((lblSerialNumTitle), "DiagnosticsTitle");
    gtk_widget_set_name((lblPCBRevTitle),    "DiagnosticsTitle");
    gtk_widget_set_name((lblVrefTitle),      "DiagnosticsTitle");

    gtk_widget_set_name((lblMinimumTitle),     "DiagnosticsTitle");
    gtk_widget_set_name((lblTemperatureTitle), "DiagnosticsTitle");
    gtk_widget_set_name((lblMaximumTitle),     "DiagnosticsTitle");

    gtk_widget_set_name((lblStatusTitle),         "DiagnosticsTitle");
    gtk_widget_set_name((lblReceiveTitle),        "DiagnosticsTitle");
    gtk_widget_set_name((lblLogfileTitle),        "DiagnosticsTitle");

    // Fixed label values
    //gtk_widget_set_name((lblXXXXXXXX),    "DiagnosticsFixed");
		
	// Values
    gtk_widget_set_name((lblFWVer),             "DiagnosticValue");
    gtk_widget_set_name((lblTimestamp),         "DiagnosticValue");
    gtk_widget_set_name((lblElapsedTime),       "DiagnosticValue");
    gtk_widget_set_name((lblBatteryVoltage),    "DiagnosticValue");
    gtk_widget_set_name((lblBatteryPercentage), "DiagnosticValue");
    gtk_widget_set_name((lblMains),             "DiagnosticValue");

    gtk_widget_set_name((lblAlarmHI),    "DiagnosticValue");
    gtk_widget_set_name((lblAlarmLO),    "DiagnosticValue");
    gtk_widget_set_name((lblAlarm),      "DiagnosticValue");
    gtk_widget_set_name((lblSampleRate), "DiagnosticValue");
    gtk_widget_set_name((lblACK),        "DiagnosticValue");
    gtk_widget_set_name((lblHostCal),    "DiagnosticValue");
    gtk_widget_set_name((lblBuzzer),     "DiagnosticValue");

    gtk_widget_set_name((lblXBeeSN),     "DiagnosticValue");
    gtk_widget_set_name((lblDevice),     "DiagnosticValue");
    gtk_widget_set_name((lblPANID),      "DiagnosticValue");
    gtk_widget_set_name((lblChannel),    "DiagnosticValue");
    gtk_widget_set_name((lblConnection), "DiagnosticValue");

    gtk_widget_set_name((lblMinimum),     "DiagnosticValue");
    gtk_widget_set_name((lblTemperature), "DiagnosticValue");
    gtk_widget_set_name((lblMaximum),     "DiagnosticValue");



    // Buttons
    gtk_widget_set_name((btnCalDate),   "button");
    gtk_widget_set_name((btnSerialNum), "button");
    gtk_widget_set_name((btnPCBRev),    "button");
    gtk_widget_set_name((btnVref),      "button");
    gtk_widget_set_name((btnMenu),        "button");
    gtk_widget_set_name((btnRTD),         "button");
    gtk_widget_set_name((btnReboot),      "button");
		
    //
    // Initialize values
	//
    display_clear_UUT_values();
        gtk_label_set_text(GTK_LABEL(lblLogfile), "----------------------------------------------------");
    // Add items to the menu combo box
    int i = 1; // skipping the first menu item, because it's already in the menu combo box
    while (!strstr(pucMenuItems[i], "ENDOFMENU"))
    {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cbtMenu), pucMenuItems[i++]);
    }

    //
    // Link button presses to callback routines
    //
    g_signal_connect(btnCalDate,   "clicked", G_CALLBACK(main_CALDATE_clicked), NULL);
    g_signal_connect(btnSerialNum, "clicked", G_CALLBACK(main_SERIALNUM_clicked), NULL);
    g_signal_connect(btnPCBRev,    "clicked", G_CALLBACK(main_BOARDREV_clicked), NULL);
    g_signal_connect(btnVref,      "clicked", G_CALLBACK(main_VREF_clicked), NULL);
    g_signal_connect(btnReboot,      "clicked", G_CALLBACK(main_REBOOT_clicked), NULL);
    g_signal_connect(btnRTD,         "clicked", G_CALLBACK(main_RTD_clicked), NULL);
    g_signal_connect(btnMenu,        "clicked", G_CALLBACK(main_MENU_clicked), NULL);
    g_signal_connect(swLogfileEnable, "state-set", G_CALLBACK(main_LOGENABLE_state_set), NULL);

}
// end display_main_initialize


////////////////////////////////////////////////////////////////////////////
// Name:         display_clear_UUT_values
// Description:  Clear the values of the unit under test
// Parameters:   None
// Return:       None
////////////////////////////////////////////////////////////////////////////
void display_clear_UUT_values(void)
{
    gtk_label_set_text(GTK_LABEL(lblFWVer), "------");
    gtk_label_set_text(GTK_LABEL(lblTimestamp), "0000000000");
    gtk_label_set_text(GTK_LABEL(lblElapsedTime), "------");
    gtk_label_set_text(GTK_LABEL(lblBatteryVoltage), "------");
    gtk_label_set_text(GTK_LABEL(lblBatteryPercentage), "------");
    gtk_label_set_text(GTK_LABEL(lblMains), "------");

    gtk_label_set_text(GTK_LABEL(lblAlarmHI), "------");
    gtk_label_set_text(GTK_LABEL(lblAlarmLO), "------");
    gtk_label_set_text(GTK_LABEL(lblAlarm), "------");
    gtk_label_set_text(GTK_LABEL(lblSampleRate), "-");
    gtk_label_set_text(GTK_LABEL(lblACK), "------");
    gtk_label_set_text(GTK_LABEL(lblHostCal), "------");
    gtk_label_set_text(GTK_LABEL(lblBuzzer), "------");

    gtk_label_set_text(GTK_LABEL(lblXBeeSN), "------");
    gtk_label_set_text(GTK_LABEL(lblDevice), "------");
    gtk_label_set_text(GTK_LABEL(lblPANID), "------");
    gtk_label_set_text(GTK_LABEL(lblChannel), "------");
    gtk_label_set_text(GTK_LABEL(lblConnection), "------");
    gtk_widget_set_name((lblConnection),     "DiagnosticValue");

    gtk_label_set_text(GTK_LABEL(lblMinimum), "------");
    gtk_label_set_text(GTK_LABEL(lblTemperature), "------");
    gtk_label_set_text(GTK_LABEL(lblMaximum), "------");

    //gtk_label_set_text(GTK_LABEL(lblXXXXXXXXX), "------");


    
    
    // Clear the Status text buffer
    gtk_text_buffer_get_start_iter(textbufStatus, &textiterStatusStart);
    gtk_text_buffer_get_end_iter  (textbufStatus, &textiterStatusEnd);
    gtk_text_buffer_delete(textbufStatus, &textiterStatusStart, &textiterStatusEnd);

    // Clear the Receive text buffer
    gtk_text_buffer_get_start_iter(textbufReceive, &textiterReceiveStart);
    gtk_text_buffer_get_end_iter  (textbufReceive, &textiterReceiveEnd);
    gtk_text_buffer_delete(textbufReceive, &textiterReceiveStart, &textiterReceiveEnd);

    // Clear sticky error status, prep for the next one
    memset(gucStickyErrorStatus,    0x00, sizeof(gucStickyErrorStatus));
    memset(gucStickyErrorStatusOLD, 0x00, sizeof(gucStickyErrorStatusOLD));
    gtk_label_set_text(GTK_LABEL(lblStatusTitle),  "Status");

    // Clear device start time
    gulDeviceStartTimestamp   = 0;
}
// end display_clear_UUT_values


////////////////////////////////////////////////////////////////////////////
// Name:         display_receive_write
// Description:  Write a new string buffer to Receive
// Parameters:   paucWriteBuf - pointer to buffer containing chars to
//                              write to Receive; assumes NULL-terminated
// Return:       None
////////////////////////////////////////////////////////////////////////////
void
display_receive_write(char * paucWriteBuf)
{
    // Move cursor to end of text buffer
    gtk_text_iter_forward_to_end(&textiterReceiveEnd);
    gtk_text_buffer_place_cursor(textbufReceive, &textiterReceiveEnd);

    // Write contents of paucWriteBuf string buffer to Receive text buffer
    gtk_text_buffer_insert(textbufReceive, &textiterReceiveEnd, paucWriteBuf, -1);

    // Move to bottom of Receive window
    adjReceive = gtk_scrolled_window_get_vadjustment(scrolledwindowReceive);
    gtk_adjustment_set_value( adjReceive, gtk_adjustment_get_upper(adjReceive) );
}
// end display_status_write


////////////////////////////////////////////////////////////////////////////
// Name:         display_status_write
// Description:  Write a new string buffer to Status
// Parameters:   paucWriteBuf - pointer to buffer containing chars to
//                              write to Status; assumes NULL-terminated
// Return:       None
////////////////////////////////////////////////////////////////////////////
void
display_status_write(char * paucWriteBuf)
{
    // Move cursor to end of text buffer
    gtk_text_iter_forward_to_end(&textiterStatusEnd);
    gtk_text_buffer_place_cursor(textbufStatus, &textiterStatusEnd);

    // Write contents of paucWriteBuf string buffer to Status text buffer
    gtk_text_buffer_insert(textbufStatus, &textiterStatusEnd, paucWriteBuf, -1);

    // Move to bottom of Status window
    adjStatus = gtk_scrolled_window_get_vadjustment(scrolledwindowStatus);
    gtk_adjustment_set_value( adjStatus, gtk_adjustment_get_upper(adjStatus) );

    // Reset Status timestamp timer
    guiStatusTimestampCountdownIndex = 0;
    guiStatusTimestampCountdown_minutes = 1;
}
// end display_status_write


////////////////////////////////////////////////////////////////////////////
// Name:         display_update_data_age
// Description:  Update data age if data hasn't updated "in a while"
// Parameters:   None
// Return:       None
////////////////////////////////////////////////////////////////////////////
void
display_update_data_age(void)
{
    if (gulElapsedTimeSinceDataUpdate_sec < DATA_TIMEOUT_MIN_MINUTES)
    {
        // Data has been updated recently, so connection is OK
    }
    else if (gulElapsedTimeSinceDataUpdate_sec < DATA_TIMEOUT_MIN_DAYS)
    {
        // Data was at least updated within the minimum number of days,
        // so issue a connection warning
    }
    else
    {
        // Data has NOT been updated within the minimum number of days,
        // so issue a connection error
    }

    if (gulElapsedTimeSinceDataUpdate_sec >= DATA_TIMEOUT_MIN_MINUTES &&
        gulElapsedTimeSinceDataUpdate_sec <  DATA_TIMEOUT_MAX_MINUTES    )
    {
        // Data is less than an hour old, report data age in minutes
        sprintf(lcTempString, "Data %d minutes old\r\n", 
                gulElapsedTimeSinceDataUpdate_sec/60);
        display_status_write(lcTempString);
    }
    else if (gulElapsedTimeSinceDataUpdate_sec >= DATA_TIMEOUT_MIN_HOURS &&
             gulElapsedTimeSinceDataUpdate_sec <  DATA_TIMEOUT_MAX_HOURS    )
    {
        // Data is less than a day old, report data age in hours
        sprintf(lcTempString, "Data %d hours old\r\n", 
                gulElapsedTimeSinceDataUpdate_sec/(60*60));
        display_status_write(lcTempString);
    }
    else if (gulElapsedTimeSinceDataUpdate_sec >= DATA_TIMEOUT_MIN_DAYS &&
             gulElapsedTimeSinceDataUpdate_sec <  DATA_TIMEOUT_MAX_DAYS    )
    {
        // Data is less than a month old, report data age in days
        sprintf(lcTempString, "Data %d days old\r\n", 
                gulElapsedTimeSinceDataUpdate_sec/(60*60*24));
        display_status_write(lcTempString);
    }
    else if (gulElapsedTimeSinceDataUpdate_sec >= DATA_TIMEOUT_MIN_MONTHS &&
             gulElapsedTimeSinceDataUpdate_sec <  DATA_TIMEOUT_MAX_MONTHS    )
    {
        // Data is less than a year old, report data age in months
        sprintf(lcTempString, "Data %.01f months old\r\n", 
                (float)gulElapsedTimeSinceDataUpdate_sec/(60*60*24*30));
        display_status_write(lcTempString);
    }
    else if (gulElapsedTimeSinceDataUpdate_sec >= DATA_TIMEOUT_MIN_YEARS)
    {
        // Report data age in years
        sprintf(lcTempString, "Data %.01f years old\r\n", 
                (float)gulElapsedTimeSinceDataUpdate_sec/(60*60*24*365));
        display_status_write(lcTempString);
    }
}
// end display_update_data_age

