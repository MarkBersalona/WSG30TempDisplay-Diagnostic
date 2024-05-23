/* 
 * File:   main.c
 * Author: Mark Bersalona
 *
 * Created on 2024.05.21
 * Main file for Sensaphone WSG30 Temperature Display Diagnostic
 */

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
///////////////////////////////////////////////////////////////////////////////

#include <gtk/gtk.h>
//#include <json-glib/json-glib.h>
//#include <glib-object.h>
//#include <gobject/gvaluecollector.h>
#include <stdio.h>
#include <stdlib.h>
//#include <fcntl.h>          // File Control Definitions
//#include <termios.h>        // POSIX Terminal Control definitions
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "main.h"
#include "gconfig.h"
#include "serial.h"
#include "display.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////////////

#define GLADE_LAYOUT ("WSG30TemperatureDisplayDiagnostic.glade")






///////////////////////////////////////////////////////////////////////////////
//
// Variables and tables
//
///////////////////////////////////////////////////////////////////////////////

// GTK builder
GtkBuilder *builder;

char lcTempMainString[250];

// Receive message FIFO
char gucReceiveFIFO[RECEIVE_FIFO_MSG_COUNT][RECEIVE_FIFO_MSG_LENGTH_MAX];
guint16 guiReceiveFIFOWriteIndex;
guint16 guiReceiveFIFOReadIndex;

// UNIX timestamp
guint32 gulUNIXTimestamp;
// Elapsed time since last data update
guint32 gulElapsedTimeSinceDataUpdate_sec;
// Glib date/time
GDateTime *gDateTime;
// Countdown minutes to Status timestamp
#define TIMESTAMP_DELAY_TABLE_COUNT (13)
guint16 guiStatusTimestampCountdown_minutes = 0;
guint8  guiStatusTimestampCountdownIndex    = 0;
guint8  uiStatusTimestampCountdownIndex_OLD = 0;
guint16 uiStatusTimestampCountdownTable[TIMESTAMP_DELAY_TABLE_COUNT] = 
{
    5, 6, 6, 7, 8, 10, 13, 18, 28, 39, 60, 120, 240
};
// Device initial and current timestamps
guint32 gulDeviceStartTimestamp   = 0;
guint32 gulDeviceCurrentTimestamp = 0;
guint32 gulDeviceRuntime_seconds;

// Logfile
GIOChannel *gioChannelLogfile;
char lcLogfileName[100];
gboolean lfIsLogfileEnabled;

// I/O channel for serial-to-USB port
GIOChannel *gIOChannelSerialUSB;

// Sticky error status
char gucStickyErrorStatus[200];
char gucStickyErrorStatusOLD[200];

// Sticky Status
#define STICKY_ERROR_COUNT_PERIOD_SECONDS 600
static guint16 guiStickyErrorCountdown_sec = STICKY_ERROR_COUNT_PERIOD_SECONDS;


///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Name:         is_valid_mac
// Description:  Validate a given string is a MAC address
//               Expects format xx-xx-xx-xx-xx-xx
//                           or xx:xx:xx:xx:xx:xx
//               where xx is a hexadecimal number in range [00:FF]
//               case-insensitive
// Parameters:   paucTestMAC - pointer to string to be tested
// Return:       TRUE if valid MAC; FALSE otherwise
////////////////////////////////////////////////////////////////////////////
gboolean is_valid_mac(char* paucTestMAC)
{
    // Test if the length is correct
    if (17 != strlen(paucTestMAC))
    {
        return FALSE;
    }

    // Test if delimiters are '-' or ':'
    for (int i = 2; i < 17; i += 3)
    {
        if (paucTestMAC[i] != '-' && paucTestMAC[i] != ':')
        {
            // invalid delimiter
            return FALSE;
        }
    }

    // Test if digits are hex
    for (int i = 0; i < 17; ++i)
    {
        if ( !isxdigit(paucTestMAC[i]) )
        {
            // Ignore the indices of the delimiters
            if ( i!=2 && i!=5 && i!=8 && i!=11 && i!=14)
            {
                return FALSE;
            }
        }
    }

    // Delimiters are OK
    // Digits are hex
    // Must be a properly-formatted MAC address
    return TRUE;
}
// end is_valid_mac

////////////////////////////////////////////////////////////////////////////
// Name:         trim
// Description:  Trims the given string of leading and trailing spaces
//               Uses isspace() to determine what's a "space"
//               By Johannes Schaub, 2008.12.09
//               stackoverflow.com/questions/352055/
//                 best-algorithm-to-strip-leading-and-trailing-spaces-in-c
// Parameters:   Pointer to start of string to trim
// Return:       Pointer to null-terminated trimmed string
////////////////////////////////////////////////////////////////////////////
char* trim(char* paucInputString)
{
    char* e = paucInputString + strlen(paucInputString) - 1;
    while (*paucInputString && isspace(*paucInputString)) paucInputString++;
    while (e > paucInputString && isspace(*e)) *e-- = '\0';
    return paucInputString;
}
// end trim







///////////////////////////////////////////////////////////////////////////////
//
// Callbacks
//
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Name:         main_ATCommand_clicked
// Description:  Callback routine - ATCommand button clicked
//               Send the user-entered AT command to the SARA-R5 on the 400 Cellular
// Parameters:   the contents of the AT Command text entry
// Return:       None
////////////////////////////////////////////////////////////////////////////
void main_ATCommand_clicked(void)
{
    char lcATCommand[100];
    guint16 luiATCommandLength;

    display_status_write("AT Command button pressed\r\n");

    //luiATCommandLength = gtk_entry_get_text_length(GTK_ENTRY(txtentATCommand));
    //sprintf(lcTempMainString, "AT Command length = %d chars\r\n", luiATCommandLength);
    //display_status_write(lcTempMainString);
    if (luiATCommandLength > 0)
    {
        // Get the contents of the AT Command text entry
        memset(lcATCommand, 0, sizeof(lcATCommand));
        //memcpy(lcATCommand, gtk_entry_get_text(GTK_ENTRY(txtentATCommand)), luiATCommandLength);

        // Send the formatted AT Command to the 400 Cellular
        //sprintf(lcTempMainString, "AT Command = >>%s<<\r\n", lcATCommand);
        //display_status_write(lcTempMainString);
        sprintf(lcTempMainString, "+++COMMAND:%s", lcATCommand);
        serial_write(lcTempMainString);
    }
    else
    {
        display_status_write("WARNING - AT Command entry appears blank\r\n");
    }
}
// end main_ATCommand_clicked


////////////////////////////////////////////////////////////////////////////
// Name:         main_BOARDREV_clicked
// Description:  Callback routine - BoardRev button clicked
//               Send the user-entered Board rev to the 400 Cellular
// Parameters:   the contents of the Board rev text entry
// Return:       None
////////////////////////////////////////////////////////////////////////////
void main_BOARDREV_clicked(void)
{
    char lcBoardRev[100];
    guint16 luiBoardRevLength;

    display_status_write("Board rev button pressed\r\n");

    //luiBoardRevLength = gtk_entry_get_text_length(GTK_ENTRY(txtentNewBoardRev));
    //sprintf(lcTempMainString, "luiBoardRevLength length = %d chars\r\n", luiBoardRevLength);
    //display_status_write(lcTempMainString);
    if (luiBoardRevLength > 0)
    {
        // Get the contents of the Board rev text entry
        memset(lcBoardRev, 0, sizeof(lcBoardRev));
        //memcpy(lcBoardRev, trim((char*)gtk_entry_get_text(GTK_ENTRY(txtentNewBoardRev))), 1);

        // Send the formatted Board rev Menu to the 400 Cellular
        if (isalpha(lcBoardRev[0]))
        {
            //sprintf(lcTempMainString, "New Board rev = >>%s<<\r\n", lcBoardRev);
            //display_status_write(lcTempMainString);
            sprintf(lcTempMainString, "+++MENU:P %s", lcBoardRev);
            serial_write(lcTempMainString);
        }
        else
        {
            sprintf(lcTempMainString, "WARNING - '%s' is an invalid Board rev\r\n", lcBoardRev);
            display_status_write(lcTempMainString);
        }
    }
    else
    {
        display_status_write("WARNING - Board rev appears blank\r\n");
    }
}
// end main_BOARDREV_clicked

////////////////////////////////////////////////////////////////////////////
// Name:         main_LOGENABLE_state_set
// Description:  Callback routine - logfile enable switch clicked
// Parameters:   None
// Return:       None
////////////////////////////////////////////////////////////////////////////
void main_LOGENABLE_state_set(void)
{
    lfIsLogfileEnabled = gtk_switch_get_active(GTK_SWITCH(swLogfileEnable));
    if (lfIsLogfileEnabled)
    {
        // Logfile has just been enabled, build timestamp filename and open file
        memset(lcLogfileName, 0, sizeof(lcLogfileName));
        sprintf(lcLogfileName, "%s WSG30TempDisplay.txt", g_date_time_format(gDateTime, "%Y%m%d %H%M"));
        sprintf(lcTempMainString, "Logfile %s opened\r\n", lcLogfileName);
        display_status_write(lcTempMainString);
        gtk_label_set_text(GTK_LABEL(lblLogfile), lcLogfileName);
        // (Open file for append, which creates new file if file doesn't exist yet)
        gioChannelLogfile = g_io_channel_new_file(lcLogfileName, "a", NULL);
        //g_io_channel_set_encoding(gioChannelLogfile, NULL, NULL);

        // Write intro text to logfile
        memset(lcTempMainString, 0, sizeof(lcTempMainString));
        sprintf(lcTempMainString, "---------- Sensaphone WSG30 Temperature Display logfile, opened %s local time -----------", 
                                                          g_date_time_format(gDateTime, "%Y.%m.%d %H:%M") );
        main_logfile_write(lcTempMainString);

        // Set the switch state to ON
        gtk_switch_set_state(GTK_SWITCH(swLogfileEnable), TRUE);
    }
    else
    {
        // Logfile has just been disabled, close the logfile and blank the displayed log filename
        g_io_channel_shutdown(gioChannelLogfile, TRUE, NULL);
        sprintf(lcTempMainString, "Logfile %s is now closed\r\n", lcLogfileName);
        display_status_write(lcTempMainString);
        gtk_label_set_text(GTK_LABEL(lblLogfile), "----------------------------------------------------");

        // Set the switch state to OFF
        gtk_switch_set_state(GTK_SWITCH(swLogfileEnable), FALSE);
    }
}
// end main_LOGENABLE_state_set

////////////////////////////////////////////////////////////////////////////
// Name:         main_MAC_clicked
// Description:  Callback routine - MAC button clicked
//               Send the user-entered MAC to the 400 Cellular
// Parameters:   the contents of the MAC text entry
// Return:       None
////////////////////////////////////////////////////////////////////////////
void main_MAC_clicked(void)
{
    char lcMACAddress[100];
    guint16 luiMACAddressLength;

    display_status_write("MAC button pressed\r\n");

    //luiMACAddressLength = gtk_entry_get_text_length(GTK_ENTRY(txtentNewMAC));
    //sprintf(lcTempMainString, "luiMACAddressLength length = %d chars\r\n", luiMACAddressLength);
    //display_status_write(lcTempMainString);
    if (luiMACAddressLength > 0)
    {
        // Get the contents of the MAC text entry
        memset(lcMACAddress, 0, sizeof(lcMACAddress));
        //memcpy(lcMACAddress, trim((char*)gtk_entry_get_text(GTK_ENTRY(txtentNewMAC))), luiMACAddressLength);

        // Send the formatted MAC Menu to the 400 Cellular
        if (is_valid_mac(lcMACAddress))
        {
            //sprintf(lcTempMainString, "New MAC address = >>%s<<\r\n", lcMACAddress);
            //display_status_write(lcTempMainString);
            sprintf(lcTempMainString, "+++MENU:M %s", lcMACAddress);
            serial_write(lcTempMainString);
        }
        else
        {
            sprintf(lcTempMainString, "WARNING - '%s' is an invalid MAC address\r\n", lcMACAddress);
            display_status_write(lcTempMainString);
        }
    }
    else
    {
        display_status_write("WARNING - MAC address appears blank\r\n");
    }
}
// end main_MAC_clicked

////////////////////////////////////////////////////////////////////////////
// Name:         main_MENU_clicked
// Description:  Callback routine - MENU button clicked
// Parameters:   None
// Return:       None
////////////////////////////////////////////////////////////////////////////
void main_MENU_clicked(void)
{
    gint liMenuItemSelected = gtk_combo_box_get_active(GTK_COMBO_BOX(cbtMenu));
    if (-1 ==liMenuItemSelected)
    {
        // No menu item selected
        display_status_write("MENU button pressed, but no menu item selected\r\n");
    }
    else
    {
        // A menu item has been selected
        sprintf(lcTempMainString, "MENU button pressed, menu item %d (%s, %s) selected\r\n", 
                                                    liMenuItemSelected, 
                                                    pucMenuCMD[liMenuItemSelected], 
                                                    pucMenuItems[liMenuItemSelected]);
        display_status_write(lcTempMainString);

        // Send the formatted menu command to the 400 Cellular
        sprintf(lcTempMainString, "+++MENU:%s", pucMenuCMD[liMenuItemSelected]);
        serial_write(lcTempMainString);
    }
}
// end main_MENU_clicked

////////////////////////////////////////////////////////////////////////////
// Name:         main_REBOOT_clicked
// Description:  Callback routine - REBOOT button clicked
// Parameters:   None
// Return:       None
////////////////////////////////////////////////////////////////////////////
void main_REBOOT_clicked(void)
{
    //display_status_write("REBOOT button pressed\r\n");
    // Send the formatted menu command to the 400 Cellular
    sprintf(lcTempMainString, "+++MENU:Z");
    serial_write(lcTempMainString);

    // Reset display
    //display_clear_UUT_values();
}
// end main_REBOOT_clicked

////////////////////////////////////////////////////////////////////////////
// Name:         main_RTD_clicked
// Description:  Callback routine - RTD button clicked
// Parameters:   None
// Return:       None
////////////////////////////////////////////////////////////////////////////
void main_RTD_clicked(void)
{
    //display_status_write("RESET to Defaults button pressed\r\n");
    // Send the formatted menu command to the 400 Cellular
    sprintf(lcTempMainString, "+++MENU:X");
    serial_write(lcTempMainString);

    // Reset display
    //display_clear_UUT_values();
}
// end main_RTD_clicked







///////////////////////////////////////////////////////////////////////////////
//
// Main application
//
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// Name:         main_parse_msg
// Description:  Parse a string for detectable data
// Parameters:   paucReceiveMsg - pointer to received NULL-terminated string
// Return:       None
////////////////////////////////////////////////////////////////////////////
void 
main_parse_msg(char *paucReceiveMsg)
{
    char *plcDetected;
    char *plcQuoteStart;
    char *plcQuoteEnd;
    char *plcPercentage;
    char *plcHumidity;
    char *plcDetectedParam;
    char *plcSpace;

    // For zone number, type and type description
    char lcZoneNumberBuf[4];
    uint8_t lucZoneNumber;
    char lcZoneTypeBuf[4];
    uint8_t lucZoneType;
    int liZoneValue;
    char lcZoneTypeName[20];

    // For zone alarm statuses
    char lcZoneAlarmBuf[4];
    uint8_t lucZoneAlarm;
    char lcZoneRangeBuf[4];
    uint8_t lucZoneRange;
    char lcZoneUnackBuf[4];
    uint8_t lucZoneUnack;
    char lcZoneAlarmColor[30];

    // Look for *** WARNING ***
    plcDetected = strstr((char*)paucReceiveMsg, "*** WARNING ***");
    if (plcDetected)
    {
        // Write the entire warning, including *** WARNING ***, to Status
        // (but only if there's a significant string following)
        memset (lcTempMainString, 0, sizeof(lcTempMainString));
        memcpy (lcTempMainString, plcDetected, sizeof(lcTempMainString));
        if (strlen(lcTempMainString) > 20)
        {
            display_status_write(lcTempMainString);
            display_status_write("\r\n");
        }
    }
    
    // Look for *** ERROR ***
    plcDetected = strstr((char*)paucReceiveMsg, "*** ERROR ***");
    if (plcDetected)
    {
        // Write the entire error, including *** ERROR ***, to Status
        // (but only if there's a significant string following)
        memset (lcTempMainString, 0, sizeof(lcTempMainString));
        memcpy (lcTempMainString, plcDetected, sizeof(lcTempMainString));
        if (strlen(lcTempMainString) > 20)
        {
            display_status_write(lcTempMainString);
            display_status_write("\r\n");

            // Save error message as a sticky one
            strcpy(gucStickyErrorStatus, lcTempMainString);
        }
    }
    
    // Look for "Sensaphone WSG30 Temperature Sensor Display starting..."
    plcDetected = strstr((char*)paucReceiveMsg, "Sensaphone WSG30 Temperature Sensor Display starting...");
    if (plcDetected)
    {
        // This is a new UUT or the old UUT restarting
        // Either way, reset the Diagnostic tool display
        display_clear_UUT_values();
    }
    
    // Look for "Board revision = "
    plcDetected = strstr((char*)paucReceiveMsg, "Board revision = ");
    if (plcDetected)
    {
        // Write the board rev to Status and to the board rev label
        memset (lcTempMainString, 0, sizeof(lcTempMainString));
        memcpy (lcTempMainString, plcDetected+17, strlen(plcDetected+17));
        display_status_write("Detected Board revision: ");
        display_status_write(lcTempMainString);
        display_status_write("\r\n");
        //gtk_label_set_text(GTK_LABEL(lblBoardRev), lcTempMainString);
    }
    
    // Look for "WSG30 Temperature Sensor firmware version is "
    plcDetected = strstr((char*)paucReceiveMsg, "WSG30 Temperature Sensor firmware version is ");
    if (plcDetected)
    {
        // Write the WSG30 Temperature Sensor FW version to Status
        memset (lcTempMainString, 0, sizeof(lcTempMainString));
        memcpy (lcTempMainString, plcDetected+33, strlen(plcDetected+45));
        display_status_write("Detected WSG30 Temperature Sensor firmware version: ");
        display_status_write(lcTempMainString);
        display_status_write("\r\n");
        //gtk_label_set_text(GTK_LABEL(lbl400FW), lcTempMainString);
    }
    
    // Look for "Network_Connection_StateMachine: Transitioning from "
    plcDetected = strstr((char*)paucReceiveMsg, "Network_Connection_StateMachine: Transitioning from ");
    if (plcDetected)
    {
        // Write the network online state transition to Status
        display_status_write(paucReceiveMsg);
        display_status_write("\r\n");

        // Get the new connection state
        plcDetectedParam = strstr((char*)paucReceiveMsg, " to ");
        if (plcDetectedParam)
        {
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+4, strlen(trim(plcDetectedParam+4)));
            gtk_label_set_text(GTK_LABEL(lblConnection), lcTempMainString);
            if ( strstr((char*)lcTempMainString, "CONNECTED") )
            {
                gtk_widget_set_name((lblConnection),     "ConnectionOK");     // green

                // Clear sticky error status, prep for the next one
                memset(gucStickyErrorStatus, 0x00, sizeof(gucStickyErrorStatus));
                guiStickyErrorCountdown_sec = STICKY_ERROR_COUNT_PERIOD_SECONDS;
                gtk_label_set_text(GTK_LABEL(lblStatusTitle),  "Status");
            }
            else
            {
                gtk_widget_set_name((lblConnection),     "DiagnosticValue");  // white
            }
        }
    }
    
    // Look for "+++ Start DIAGNOSTIC MODE +++"
    plcDetected = strstr((char*)paucReceiveMsg, "+++ Start DIAGNOSTIC MODE +++");
    if (plcDetected)
    {
        // Diagnostic mode is enabled on the UUT
        // Send a sacrificial dummy string to "initialize" serial_write()
        // and UUT Diagnostic receive
        //serial_write("+++");
        serial_write("+++ 1  2  3  Sensaphone WSG30 Temperature Display Diagnostic tool reporting...\r\n");
    }
    
    // Look for "InputTask: Battery reading: " then "Percentage "
    plcDetected = strstr((char*)paucReceiveMsg, "InputTask: Battery reading: ");
    if (plcDetected)
    {
        plcPercentage = strstr((char*)plcDetected, "Percentage ");
        if (plcPercentage)
        {
            // Write the battery percentage to the Battery Value label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcPercentage+11, strlen(trim(plcPercentage+11)));
            gtk_label_set_text(GTK_LABEL(lblBatteryPercentage), lcTempMainString);
        }
    }
    
    // Look for "InputTask: Battery reading: " then "Battery voltage "
    plcDetected = strstr((char*)paucReceiveMsg, "InputTask: Battery reading: ");
    if (plcDetected)
    {
        plcDetectedParam = strstr((char*)plcDetected, "Battery voltage ");
        plcSpace = strchr(plcDetectedParam+16, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the battery percentage to the Battery Value label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+16, plcSpace - (plcDetectedParam+16));
            strcat (lcTempMainString, " V");
            gtk_label_set_text(GTK_LABEL(lblBatteryVoltage), lcTempMainString);
        }
    }
    
    // Look for "Timestamp "
    plcDetected = strstr((char*)paucReceiveMsg, "Timestamp ");
    if (plcDetected)
    {
        plcSpace = strchr(plcDetected+10, 0x20); // search for 1st space character
        if (plcSpace)
        {
            // Get date and time, excluding the seconds and UTC
            memset(lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy(lcTempMainString, plcDetected+10, plcSpace-(plcDetected+10));

            // Write device time to Timestamp label
            gtk_label_set_text(GTK_LABEL(lblTimestamp),  lcTempMainString);

            // Calculate elapsed time
            gulDeviceCurrentTimestamp = (guint32)atoi(trim(lcTempMainString));
            if (0 == gulDeviceStartTimestamp || gulDeviceStartTimestamp > gulDeviceCurrentTimestamp) 
            {
                gulDeviceStartTimestamp = gulDeviceCurrentTimestamp;
            }
            gulDeviceRuntime_seconds = gulDeviceCurrentTimestamp - gulDeviceStartTimestamp;
            guint16 luiRuntime_Days     = gulDeviceRuntime_seconds / (60*60*24);
            guint8  lucRuntime_Hours    = (gulDeviceRuntime_seconds % (60*60*24))/(60*60);
            guint8  lucRuntime_Minutes  = (gulDeviceRuntime_seconds % (60*60))/60;
            guint8  lucRuntime_Seconds  = (gulDeviceRuntime_seconds % (60*60))%60;
            memset(lcTempMainString, 0, sizeof(lcTempMainString));
            sprintf(lcTempMainString, "%dd %dh %dm %ds", luiRuntime_Days, lucRuntime_Hours, lucRuntime_Minutes, lucRuntime_Seconds);
            gtk_label_set_text(GTK_LABEL(lblElapsedTime),  lcTempMainString);
        }
    }

    // Look for "STATUS >> " 
    plcDetected = strstr((char*)paucReceiveMsg, "STATUS >> ");
    if (plcDetected)
    {
        // look for "Scale:"
        plcDetectedParam = strstr((char*)plcDetected, "Scale:");
        plcSpace = strchr(plcDetectedParam+6, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the MINIMUM to the minimum label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+6, plcSpace - (plcDetectedParam+6));

            // Write "Fahrenheit" or "Celsius"
            if (strstr((char*)lcTempMainString, "F"))
            {
                gtk_label_set_text(GTK_LABEL(lblTemperatureTitle), "Fahrenheit");
            }
            else
            {
                gtk_label_set_text(GTK_LABEL(lblTemperatureTitle), "Celsius");
            }
        }

        // look for "MIN:"
        plcDetectedParam = strstr((char*)plcDetected, "MIN:");
        plcSpace = strchr(plcDetectedParam+4, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the MINIMUM to the minimum label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+4, plcSpace - (plcDetectedParam+4));
            gtk_label_set_text(GTK_LABEL(lblMinimum), lcTempMainString);
        }

        // look for "TEMP:"
        plcDetectedParam = strstr((char*)plcDetected, "TEMP:");
        plcSpace = strchr(plcDetectedParam+5, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the TEMPERATURE to the temperature label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+5, plcSpace - (plcDetectedParam+5));
            gtk_label_set_text(GTK_LABEL(lblTemperature), lcTempMainString);
        }

        // look for "MAX:"
        plcDetectedParam = strstr((char*)plcDetected, "MAX:");
        plcSpace = strchr(plcDetectedParam+4, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the MAXIMUM to the maximum label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+4, plcSpace - (plcDetectedParam+4));
            gtk_label_set_text(GTK_LABEL(lblMaximum), lcTempMainString);
        }

        // look for "AlHI:"
        plcDetectedParam = strstr((char*)plcDetected, "AlHI:");
        plcSpace = strchr(plcDetectedParam+5, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the AlarmHI to the AlarmHI label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+5, plcSpace - (plcDetectedParam+5));
            gtk_label_set_text(GTK_LABEL(lblAlarmHI), lcTempMainString);
        }

        // look for "AlLO:"
        plcDetectedParam = strstr((char*)plcDetected, "AlLO:");
        plcSpace = strchr(plcDetectedParam+5, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the AlarmLO to the AlarmLO label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+5, plcSpace - (plcDetectedParam+5));
            gtk_label_set_text(GTK_LABEL(lblAlarmLO), lcTempMainString);
        }

        // look for "Alarm:"
        plcDetectedParam = strstr((char*)plcDetected, "Alarm:");
        plcSpace = strchr(plcDetectedParam+6, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the Alarm to the Alarm label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+6, plcSpace - (plcDetectedParam+6));
            gtk_label_set_text(GTK_LABEL(lblAlarm), lcTempMainString);
        }

        // look for "SampleRateSeconds:"
        plcDetectedParam = strstr((char*)plcDetected, "SampleRateSeconds:");
        plcSpace = strchr(plcDetectedParam+18, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the sample rate to the sample rate label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+18, plcSpace - (plcDetectedParam+18));
            gtk_label_set_text(GTK_LABEL(lblSampleRate), lcTempMainString);
        }

        // look for "ACK:"
        plcDetectedParam = strstr((char*)plcDetected, "ACK:");
        plcSpace = strchr(plcDetectedParam+4, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the Alarm ACK to the label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+4, plcSpace - (plcDetectedParam+4));
            gtk_label_set_text(GTK_LABEL(lblACK), lcTempMainString);
        }

        // look for "HostCal:"
        plcDetectedParam = strstr((char*)plcDetected, "HostCal:");
        plcSpace = strchr(plcDetectedParam+8, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the Host Calibration to the label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+8, plcSpace - (plcDetectedParam+8));
            gtk_label_set_text(GTK_LABEL(lblHostCal), lcTempMainString);
        }

        // look for "Buzzer:"
        plcDetectedParam = strstr((char*)plcDetected, "Buzzer:");
        plcSpace = strchr(plcDetectedParam+7, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the Buzzer to the label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+7, plcSpace - (plcDetectedParam+7));
            gtk_label_set_text(GTK_LABEL(lblBuzzer), lcTempMainString);
        }

        // look for "Mains:"
        plcDetectedParam = strstr((char*)plcDetected, "Mains:");
        if (plcDetectedParam)
        {
            // Write the Mains to the label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+6, strlen(plcDetectedParam+6));
            gtk_label_set_text(GTK_LABEL(lblMains), lcTempMainString);
        }

    }
    
    // Look for "XBEE >> " 
    plcDetected = strstr((char*)paucReceiveMsg, "XBEE >> ");
    if (plcDetected)
    {
        // look for "SerialNumber:"
        plcDetectedParam = strstr((char*)plcDetected, "SerialNumber:");
        plcSpace = strchr(plcDetectedParam+13, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the SERIAL NUMBER to the serial number label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+13, plcSpace - (plcDetectedParam+13));
            gtk_label_set_text(GTK_LABEL(lblXBeeSN), lcTempMainString);
        }

        // look for "Device:"
        plcDetectedParam = strstr((char*)plcDetected, "Device:");
        plcSpace = strchr(plcDetectedParam+7, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the device to the device label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+7, plcSpace - (plcDetectedParam+7));
            gtk_label_set_text(GTK_LABEL(lblDevice), lcTempMainString);
        }

        // look for "PAN_ID:"
        plcDetectedParam = strstr((char*)plcDetected, "PAN_ID:");
        plcSpace = strchr(plcDetectedParam+5, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the PAN ID to the PAN ID label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+7, plcSpace - (plcDetectedParam+7));
            gtk_label_set_text(GTK_LABEL(lblPANID), lcTempMainString);
        }

        // look for "Channel:"
        plcDetectedParam = strstr((char*)plcDetected, "Channel:");
        plcSpace = strchr(plcDetectedParam+8, 0x20); // search for 1st space character
        if (plcDetectedParam && plcSpace)
        {
            // Write the channel to the channel label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+8, plcSpace - (plcDetectedParam+8));
            gtk_label_set_text(GTK_LABEL(lblChannel), lcTempMainString);
        }

        // look for "Connection:"
        plcDetectedParam = strstr((char*)plcDetected, "Connection:");
        if (plcDetectedParam)
        {
            // Write the Mains to the label
            memset (lcTempMainString, 0, sizeof(lcTempMainString));
            memcpy (lcTempMainString, plcDetectedParam+11, strlen(plcDetectedParam+11));
            gtk_label_set_text(GTK_LABEL(lblConnection), lcTempMainString);
        }

    }
 
    // Look for "Network_XBee_Modem_Status: "
    plcDetected = strstr((char*)paucReceiveMsg, "Network_XBee_Modem_Status: ");
    if (plcDetected)
    {
        // Write the Modem Status to Status and to the connection label
        memset (lcTempMainString, 0, sizeof(lcTempMainString));
        memcpy (lcTempMainString, plcDetected+27, strlen(plcDetected+27));
        display_status_write("Modem Status: ");
        display_status_write(lcTempMainString);
        display_status_write("\r\n");
        gtk_label_set_text(GTK_LABEL(lblConnection), lcTempMainString);
    }
    
    
}
// end main_parse_msg

////////////////////////////////////////////////////////////////////////////
// Name:         main_receive_msg_read
// Description:  Read a received string from the receive FIFO
// Parameters:   None
// Return:       Pointer to received string, or NULL if no more strings
//               are available from the FIFO
////////////////////////////////////////////////////////////////////////////
char * 
main_receive_msg_read(void)
{
    char *plcReturnPointer;

    if (guiReceiveFIFOReadIndex == guiReceiveFIFOWriteIndex)
    {
        // Read and write FIFO pointers are the same, no more receive strings available
        plcReturnPointer = NULL;
    }
    else
    {
        // Return the next available received string off the FIFO,
        // then point to the next received string
        plcReturnPointer = &gucReceiveFIFO[guiReceiveFIFOReadIndex][0];
        if (++guiReceiveFIFOReadIndex >= RECEIVE_FIFO_MSG_COUNT) guiReceiveFIFOReadIndex = 0;
    }
    return plcReturnPointer;
}
// end main_receive_msg_read


////////////////////////////////////////////////////////////////////////////
// Name:         main_logfile_write
// Description:  Write NULL-terminated string to logfile
//               Assumes logfile already exists
// Parameters:   paucMessage - pointer to NULL-terminated string
// Return:       Size of message written
////////////////////////////////////////////////////////////////////////////
int main_logfile_write(char * paucMessage)
{
    gssize lsizeByteWritten = 0;

    if (lfIsLogfileEnabled)
    {
        // Write message to logfile
        g_io_channel_write_chars(gioChannelLogfile, paucMessage, -1, &lsizeByteWritten, NULL);
        g_io_channel_write_chars(gioChannelLogfile, "\r\n",      -1, NULL, NULL);
        // Send it out NOW!!
        //g_io_channel_flush(gioChannelLogfile, NULL);
    }

    return (int)lsizeByteWritten;
}
// end main_logfile_write

////////////////////////////////////////////////////////////////////////////
// Name:         main_receive_msg_write
// Description:  Write a received string to the FIFO
// Parameters:   paucReceiveMsg - pointer to received NULL-terminated string
// Return:       None
////////////////////////////////////////////////////////////////////////////
void 
main_receive_msg_write(char *paucReceiveMsg)
{
    static char lcFIFOWarning[200];

    // Zeroize the FIFO entry about to get the received message string
    memset(&gucReceiveFIFO[guiReceiveFIFOWriteIndex][0], 0, RECEIVE_FIFO_MSG_LENGTH_MAX);

    // Copy the received message string into the FIFO entry,
    // then point to the next FIFO entry to receive the next received message string
    memcpy(&gucReceiveFIFO[guiReceiveFIFOWriteIndex][0], paucReceiveMsg, strlen(paucReceiveMsg));
    if (++guiReceiveFIFOWriteIndex >= RECEIVE_FIFO_MSG_COUNT) guiReceiveFIFOWriteIndex = 0;

    // Check if the FIFO is almost full
    int liFIFOCount = guiReceiveFIFOWriteIndex - guiReceiveFIFOReadIndex;
    if (liFIFOCount < 0) liFIFOCount += RECEIVE_FIFO_MSG_COUNT;
    if (liFIFOCount > RECEIVE_FIFO_MSG_COUNT - 25)
    {
        sprintf(lcFIFOWarning, "\r\nWARNING - receive FIFO is almost full\r\n");
        display_status_write(lcFIFOWarning);
        //main_logfile_write(lcFIFOWarning);
    }
    else if (liFIFOCount == RECEIVE_FIFO_MSG_COUNT/2)
    {
        sprintf(lcFIFOWarning, "\r\nWARNING - receive FIFO is half-full\r\n");
        display_status_write(lcFIFOWarning);
        //main_logfile_write(lcFIFOWarning);
    }
}
// end main_receive_msg_write


////////////////////////////////////////////////////////////////////////////
// Name:         main_periodic
// Description:  Main periodic code
// Parameters:   None
// Return:       TRUE
////////////////////////////////////////////////////////////////////////////
static gboolean
main_periodic(gpointer data)
{
    static guint32 lulElapsed_sec = 0;
    char* plcReceivedMsgAvailable;
    static int fd;
    static gint liRTCSecond;
    static gint liRTCMinute;
    static gint liRTCHour;
    static gint liOLDRTCSecond = 99;
    static gint liOLDRTCMinute = 99;
    static gint liOLDRTCHour   = 99;
    
    //////////////////////////////////////////////////////////
    //
    // Update timestamps
    //
    //////////////////////////////////////////////////////////
    if (gulUNIXTimestamp != g_get_real_time()/1000000)
    {
        //
        // Updates every second
        //
        gulUNIXTimestamp = g_get_real_time()/1000000;
        //gDateTime = g_date_time_new_now_utc();
        gDateTime = g_date_time_new_now_local();
        ++lulElapsed_sec;
        ++gulElapsedTimeSinceDataUpdate_sec;
        liRTCSecond = g_date_time_get_second(gDateTime);
        liRTCMinute = g_date_time_get_minute(gDateTime);
        liRTCHour   = g_date_time_get_hour(gDateTime);

        // Force Status window to bottom
        adjStatus = gtk_scrolled_window_get_vadjustment(scrolledwindowStatus);
        gtk_adjustment_set_value( adjStatus, gtk_adjustment_get_upper(adjStatus) );

        // Display sticky error status (if any) to Status for N minutes
        // (but first check if stick error status has changed)
        if (strcmp(gucStickyErrorStatus, gucStickyErrorStatusOLD))
        {
            // Want to display new sticky status immediately,
            // so reset sticky error count and make the new
            // sticky status the current one
            guiStickyErrorCountdown_sec = STICKY_ERROR_COUNT_PERIOD_SECONDS;
            memset(gucStickyErrorStatusOLD, 0x00, sizeof(gucStickyErrorStatusOLD));
            strcpy(gucStickyErrorStatusOLD, gucStickyErrorStatus);
        }
        if (strlen(gucStickyErrorStatus) > 0)
        {
            if (guiStickyErrorCountdown_sec > 0)
            {
                // Display sticky error status
                --guiStickyErrorCountdown_sec;
                if (lulElapsed_sec%60 == 0)
                {
                    display_status_write(gucStickyErrorStatus);
                    display_status_write("\r\n");
                }
                sprintf(lcTempMainString, "Status: %s", gucStickyErrorStatus);
                gtk_label_set_text(GTK_LABEL(lblStatusTitle),  lcTempMainString);
            }
            else
            {
                // Clear sticky error status, prep for the next one
                memset(gucStickyErrorStatus, 0x00, sizeof(gucStickyErrorStatus));
                guiStickyErrorCountdown_sec = STICKY_ERROR_COUNT_PERIOD_SECONDS;
                gtk_label_set_text(GTK_LABEL(lblStatusTitle),  "Status");
            }
        }
        else
        {
            guiStickyErrorCountdown_sec = STICKY_ERROR_COUNT_PERIOD_SECONDS;
        }

        if (lulElapsed_sec%60 == 0)
        {
            //
            // Updates every ELAPSED minute
            //
        }

        if (lulElapsed_sec%(5*60) == 0)
        {
            //
            // Updates every ELAPSED 5 minutes
            //
            // Periodically clear the Receive text buffer
            gtk_text_buffer_get_start_iter(textbufReceive, &textiterReceiveStart);
            gtk_text_buffer_get_end_iter  (textbufReceive, &textiterReceiveEnd);
            gtk_text_buffer_delete(textbufReceive, &textiterReceiveStart, &textiterReceiveEnd);
        }


        if (lulElapsed_sec%(60*60) == 0)
        {
            //
            // Updates every ELAPSED hour
            //
        }

        if (lulElapsed_sec%(60*60*24) == 0)
        {
            //
            // Updates every ELAPSED 24-hour day
            //
            // Periodically clear the Status text buffer
            gtk_text_buffer_get_start_iter(textbufStatus, &textiterStatusStart);
            gtk_text_buffer_get_end_iter  (textbufStatus, &textiterStatusEnd);
            gtk_text_buffer_delete(textbufStatus, &textiterStatusStart, &textiterStatusEnd);
        }

        //
        // Updates every minute
        //
        if (liOLDRTCMinute != liRTCMinute)
        {
            liOLDRTCMinute = liRTCMinute;

            if (--guiStatusTimestampCountdown_minutes)
            {
                // Do nothing, not yet time to display Status timestamp
            }
            else
            {
                // Save old table index
                uiStatusTimestampCountdownIndex_OLD = guiStatusTimestampCountdownIndex;

                // Print timestamp in Status
                sprintf(lcTempMainString, "%s: UNIX timestamp %d\t", __FUNCTION__, gulUNIXTimestamp);
                display_status_write(lcTempMainString);
                sprintf(lcTempMainString, "Local time %s\r\n", g_date_time_format(gDateTime, "%Y-%m-%d %H:%M:%S"));
                display_status_write(lcTempMainString);

                // Display data age
                display_update_data_age();

                // Restore table index; update if needed
                guiStatusTimestampCountdownIndex = uiStatusTimestampCountdownIndex_OLD;
                if (guiStatusTimestampCountdownIndex<(TIMESTAMP_DELAY_TABLE_COUNT-1)) ++guiStatusTimestampCountdownIndex;

                // Reset minute countdown for Status timestamp
                guiStatusTimestampCountdown_minutes = uiStatusTimestampCountdownTable[guiStatusTimestampCountdownIndex];
            }
        }

    }
    
    //////////////////////////////////////////////////////////
    //
    // USB reconnect
    //
    //////////////////////////////////////////////////////////
    if (isUSBConnectionOK)
    {
        // Code when USB connection OK
    }
    else // USB connection not OK
    {
        // Code when USB connection failed
        // Try to open the serial-to-USB port
        /* IO channel variable for file */

        // g_print("\r\nlulElapsed_sec=%d   serial_open returns %d\r\n",
        //         lulElapsed_sec, 
        //         fd = serial_open("/dev/ttyUSB0",115200));
        fd = serial_open("/dev/ttyUSB0",115200);
        //g_print("  fd = %d\r\n", fd);
        if (fd<0)
        {
            if (isFirstSerialFail)
            {
                isFirstSerialFail = FALSE;
                display_status_write("***ERROR*** problem opening ttyUSB0 - connect serial-to-USB cable to USB port\r\n");
            }
            isUSBConnectionOK = FALSE;
        }
        else
        {
            display_status_write("ttyUSB0 opened successfully!\r\n");
            isUSBConnectionOK = TRUE;
            isFirstSerialFail = TRUE;
            gIOChannelSerialUSB = g_io_channel_unix_new(fd);  // creates the correct reference for callback
            
            // Set encoding
            g_io_channel_set_encoding(gIOChannelSerialUSB, NULL, NULL);
            // g_print("\r\n%s() g_io_channel_get_encoding() returned %s\r\n", __FUNCTION__,
            //         g_io_channel_get_encoding(gIOChannelSerialUSB));

            // Specify callback routines for serial read and error
            g_io_add_watch(gIOChannelSerialUSB, 
                           G_IO_IN, 
                           serial_read, 
                           NULL);
            g_io_add_watch(gIOChannelSerialUSB, 
                           G_IO_ERR|G_IO_HUP|G_IO_NVAL, 
                           serial_error, 
                           NULL);
        }
    }
    
    
    //////////////////////////////////////////////////////////
    //
    // Display and parse received messages
    // If a log file is active, save received messages
    //
    //////////////////////////////////////////////////////////
    do
    {
        plcReceivedMsgAvailable = main_receive_msg_read();
        if (plcReceivedMsgAvailable)
        {
            // Reinitialize data age
            gulElapsedTimeSinceDataUpdate_sec = 0;

            // If log file is active, save received message
            // (save it to the logfile NOW; if something unexpected
            //  is triggering the app to crash, perhaps it'll be saved)
            main_logfile_write(plcReceivedMsgAvailable);

            // Display received message
            display_receive_write(plcReceivedMsgAvailable);
            display_receive_write("\r\n");

            // Parse received message
            main_parse_msg(plcReceivedMsgAvailable);
        }
    } while (plcReceivedMsgAvailable);
    
    
    return TRUE;
}
// end main_periodic



////////////////////////////////////////////////////////////////////////////
// Name:         main
// Description:  Main routine for 400 Cellular Diagnostic
// Parameters:   Standard main arguments, unused
// Return:       0 on conventional exit; error otherwise
////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    GError *error = NULL;

    gtk_init(&argc, &argv);
    
    //
    // Initalize any globals needed
    //
    gulElapsedTimeSinceDataUpdate_sec = 0;
    //gDateTime = g_date_time_new_now_utc();
    gDateTime = g_date_time_new_now_local();
    lfIsLogfileEnabled = FALSE;
    guiStatusTimestampCountdown_minutes = 1;
    memset (gucStickyErrorStatus, 0x00, sizeof(gucStickyErrorStatus));

    //
    // Enable CSS styling (colors, fonts, text sizes)
    //
    cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "theme.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                 GTK_STYLE_PROVIDER(cssProvider),
                                 GTK_STYLE_PROVIDER_PRIORITY_USER);

    //
    // Construct a GtkBuilder instance and load our UI description
    //
    builder = gtk_builder_new();
    if (gtk_builder_add_from_file(builder, GLADE_LAYOUT, &error) == 0)
    {
        g_printerr ("Error loading file: %s\r\n", error->message);
        g_clear_error(&error);
        return 1;
    }
    
    //
    // Connect signal handlers to the constructed widgets
    //
    // Main window
    window = GTK_WINDOW(gtk_builder_get_object(builder, "window1"));
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    

    //
    // Initialize the Main window
    //
    display_main_initialize();
   
    //
    // Enable window decoration (title bar, minimize/maximize/close, etc.)
    //
    gtk_window_set_decorated(GTK_WINDOW(window), TRUE);

    
    //
    // Start the timeout periodic function
    //
    g_timeout_add(MAIN_PERIODIC_INTERVAL_MSEC, main_periodic, NULL);

    display_status_write("=================================<=>=================================\r\n");
    display_status_write("               Sensaphone WSG30 Temp Sensor Diagnostic               \r\n");
    sprintf(lcTempMainString, "                               v%s.%s.%s \r\n", VERSION_A,VERSION_B,VERSION_C);
    display_status_write(lcTempMainString);
    sprintf(lcTempMainString, "                             %s     \r\n", VERSION_DATE);
    display_status_write(lcTempMainString);
    display_status_write("=================================<=>=================================\r\n");


    //
    // Kick off GTK main loop
    //
    gtk_main();

    
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // Should only get here when exiting/quitting the GTK application
    //
    ////////////////////////////////////////////////////////////////////
    return (0);
}
// end main


