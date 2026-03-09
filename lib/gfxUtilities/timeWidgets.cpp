/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/timeWidgets.cpp
 * Description: Implementations for specific header rendering widgets.
 */

#include "timeWidgets.hpp"

/**
 * @brief Draw the National Rail header row including station name and clock
 * @param stopName Primary station name
 * @param callingStopName Filter/via station name
 * @param platFilter Active platform filter
 * @param timeOffset Time offset applied to board
 */
void drawCurrentTime(const char* stopName, const char* callingStopName, const char* platFilter, int timeOffset) {
  char szTime[10];

  getLocalTime(&timeinfo);
  byte myHour = timeinfo.tm_hour;
  byte myMin = timeinfo.tm_min;

  if (clockEnabled) snprintf(szTime,sizeof(szTime),"%02d:%02d",myHour,myMin); else strcpy(szTime,"");

  u8g2.setDrawColor(0);
  u8g2.drawBox(0,0,256,9);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_5x8_tf);
  
  if (strcmp(callingStopName,"")!=0) {
    if (strcmp(platFilter,"")!=0) {
      char header[100];
      snprintf(header,sizeof(header),"%s to %s (Plat %s)",stopName,callingStopName,platFilter);
      u8g2.drawStr(0, 7, header);
    } else {
      char header[100];
      snprintf(header,sizeof(header),"%s to %s",stopName,callingStopName);
      u8g2.drawStr(0, 7, header);
    }
  } else {
    if (strcmp(platFilter,"")!=0) {
      char header[100];
      snprintf(header,sizeof(header),"%s (Plat %s)",stopName,platFilter);
      u8g2.drawStr(0, 7, header);
    } else u8g2.drawStr(0, 7, stopName);
  }

  if (timeOffset!=0) {
    char tOffset[40];
    snprintf(tOffset,sizeof(tOffset),"(+%d mins)",timeOffset);
    u8g2.drawStr(u8g2.getDisplayWidth() - getStringWidth(tOffset) - getStringWidth(szTime) - 4, 7, tOffset);        
  } else {
    if (dateEnabled) {
      char date[30];
      byte wDay = timeinfo.tm_wday;
      byte mDay = timeinfo.tm_mday;
      snprintf(date,sizeof(date),"%s %02d",weekdays[wDay],mDay);
      u8g2.drawStr(u8g2.getDisplayWidth() - getStringWidth(date) - getStringWidth(szTime) - 4, 7, date);      
    }
  }
  
  u8g2.drawStr(u8g2.getDisplayWidth() - getStringWidth(szTime), 7, szTime);
}

/**
 * @brief Render the current time in the top right corner for TfL/Bus boards
 * @param update If true, visually refresh the display area immediately
 */
void drawCurrentTimeUG(bool update) {
  char szTime[10];
  
  getLocalTime(&timeinfo);
  byte myHour = timeinfo.tm_hour;
  byte myMin = timeinfo.tm_min;

  if (clockEnabled) snprintf(szTime,sizeof(szTime),"%02d:%02d",myHour,myMin); else strcpy(szTime,"");

  if (update) {
    u8g2.setDrawColor(0);
    u8g2.drawBox(u8g2.getDisplayWidth() - getStringWidth(szTime) - 2,0,u8g2.getDisplayWidth(),9);
    u8g2.setDrawColor(1);    
  }
  
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(u8g2.getDisplayWidth() - getStringWidth(szTime), 7, szTime);

  if (update) {
    u8g2.updateDisplayArea(31,0,1,1);
  }
}
