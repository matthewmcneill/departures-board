/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/nationalRailBoard/src/nationalRailBoard.cpp
 * Description: Concrete implementation of OLED rendering for National Rail boards.
 */

#include "../include/nationalRailBoard.hpp"
extern void blankArea(int x, int y, int w, int h);
extern void centreText(const char *msg, int yConstraint);
extern int getStringWidth(const char *str);
extern const uint8_t NatRailSmall9[];
extern const uint8_t NatRailTall12[];



// Removed encapsulated variables: callingStation, platformFilter, nrTimeOffset, crsCode, nrToken, callingCrsCode, cleanPlatformFilter
extern bool dateEnabled;
extern int dateWidth;
extern int dateDay;
extern struct tm timeinfo;
extern char displayedTime[29];
extern bool hidePlatform;
extern const char* myUrl;
extern bool isShowingVia;
extern stnMessages messages;
#include "../../busBoard/include/busBoard.hpp"
#include <weatherClient.h>
extern const char* wsdlHost;
extern const char* wsdlAPI;

extern void raildataCallback(int stage, int nServices);

// Shared constants for Screen Dimensions
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 64
#endif
#ifndef LINE0
#define LINE0 0
#define LINE1 13
#define LINE2 28
#define LINE3 41
#define LINE4 55
#endif

extern const char nrAttributionn[];
// Coach class availability
static const char firstClassSeating[] PROGMEM = " First class seating only.";
static const char standardClassSeating[] PROGMEM = " Standard class seating only.";
static const char dualClassSeating[] PROGMEM = " First and Standard class seating available.";
extern bool noScrolling;

/**
 * @brief Render the top header bar of the screen for the National Rail board.
 *        Draws the station name, calling stops/platforms, and current time.
 * @param u8g2 Reference to the global U8g2 object.
 */
void NationalRailBoard::drawHeader(U8G2& u8g2) const {
  // Clear the top line
  blankArea(0,LINE0,256,LINE1-1);

  u8g2.setFont(NatRailSmall9);
  char boardTitle[95];
  String title = String(stationData.location) + " ";
  if (nrTimeOffset) {
    title+="\x8F";
    if (nrTimeOffset>0) title+="+";
    title+=String(nrTimeOffset) + "m ";
  }
  if (platformFilter[0]) title+="\x8D" + String(platformFilter) + " ";
  if (callingStation[0]) title+="(\x81" + String(callingStation) + ")";
  title.trim();
  strncpy(boardTitle,title.c_str(),sizeof(boardTitle));

  int boardTitleWidth = getStringWidth(boardTitle);

  if (dateEnabled) {
    int const dateY=55;
    // Get the date
    char sysTime[29];
    getLocalTime(&timeinfo);
    strftime(sysTime,29,"%a %d %b",&timeinfo);
    dateWidth = getStringWidth(sysTime);
    dateDay = timeinfo.tm_mday;
    if (callingStation[0] || boardTitleWidth+dateWidth+10>=SCREEN_WIDTH) {
      blankArea(SCREEN_WIDTH-70,dateY,70,SCREEN_HEIGHT-dateY);
      u8g2.drawStr(SCREEN_WIDTH-dateWidth,dateY-1,sysTime); // Date bottom right
      centreText(boardTitle,LINE0-1);
    } else {
      u8g2.drawStr(SCREEN_WIDTH-dateWidth,LINE0-1,sysTime); // right-aligned date top
      if ((SCREEN_WIDTH-boardTitleWidth)/2 < dateWidth+8) {
        // station name left aligned
        u8g2.drawStr(0,LINE0-1,boardTitle);
      } else {
        centreText(boardTitle,LINE0-1);
      }
    }
  } else {
    centreText(boardTitle,LINE0-1);
  }
}

/**
 * @brief Pull down the latest departures from the National Rail SOAP API!
 */
int NationalRailBoard::updateData() {
    raildataXmlClient* dataClient = new raildataXmlClient();
    if (!dataClient) {
        strcpy(lastErrorMsg, "Out of Memory");
        return UPD_DATA_ERROR;
    }

    // Ensure the inner `raildataXmlClient` is initialized with the SOAP host variables
    static bool isInitialized = false;
    if (!isInitialized) {
        int res = dataClient->init(wsdlHost, wsdlAPI, &raildataCallback);
        if (res == UPD_SUCCESS) {
            isInitialized = true;
            dataClient->cleanFilter(platformFilter, cleanPlatformFilter, sizeof(platformFilter));
        } else {
            strncpy(lastErrorMsg, dataClient->getLastError(), sizeof(lastErrorMsg)-1);
            delete dataClient;
            return res;
        }
    } else {
        // Run filter clean anyway for subsequent calls if filter changes
        dataClient->cleanFilter(platformFilter, cleanPlatformFilter, sizeof(platformFilter));
    }
    
    // Trigger the actual web request!
    int status = dataClient->updateDepartures(
        &stationData, 
        &messages, 
        crsCode, 
        nrToken, 
        NR_MAX_SERVICES, 
        busBoard->getEnableBus(), 
        callingCrsCode, 
        cleanPlatformFilter, 
        nrTimeOffset
    );
    
    strncpy(lastErrorMsg, dataClient->getLastError(), sizeof(lastErrorMsg)-1);
    delete dataClient;
    return status;
}

/**
 * @brief Render a single row/service onto the OLED screen for a National Rail train.
 *        Complex logic to render primary services large and secondary services smaller.
 * @param u8g2 Reference to the global U8g2 object.
 * @param line The 0-based index of the service to draw.
 * @param yPos The pixel Y-coordinate for the baseline of this row.
 */
void NationalRailBoard::drawService(U8G2& u8g2, int line, int yPos) const {
    if (line == 0) {
        // ---- Draw Primary Service (Line 1, top heavy) ----
        int destPos;
        char clipDestination[NR_MAX_LOCATION];
        char etd[16];

        u8g2.setFont(NatRailTall12);
        blankArea(0,LINE1,256,LINE2-LINE1);
        destPos = u8g2.drawStr(0,LINE1-1,stationData.service[0].sTime) + 6;
        if (stationData.service[0].platform[0] && strlen(stationData.service[0].platform)<3 && stationData.service[0].serviceType == NR_SERVICE_TRAIN && !hidePlatform) {
            destPos += u8g2.drawStr(destPos,LINE1-1,stationData.service[0].platform) + 6;
        } else if (stationData.service[0].serviceType == NR_SERVICE_BUS) {
            destPos += u8g2.drawStr(destPos,LINE1-1,"~") + 6; // Bus icon
        }
        if (isDigit(stationData.service[0].etd[0])) sprintf(etd,"Exp %s",stationData.service[0].etd);
        else strcpy(etd,stationData.service[0].etd);
        int etdWidth = getStringWidth(etd);
        u8g2.drawStr(SCREEN_WIDTH - etdWidth,LINE1-1,etd);
        // Space available for destination name
        int spaceAvailable = SCREEN_WIDTH - destPos - etdWidth - 6;
        if (isShowingVia) strcpy(clipDestination,stationData.service[0].via);
        else strcpy(clipDestination,stationData.service[0].destination);
        if (getStringWidth(clipDestination) > spaceAvailable) {
            while (getStringWidth(clipDestination) > (spaceAvailable - 8)) {
                clipDestination[strlen(clipDestination)-1] = '\0';
            }
            // check if there's a trailing space left
            if (clipDestination[strlen(clipDestination)-1] == ' ') clipDestination[strlen(clipDestination)-1] = '\0';
            strcat(clipDestination,"...");
        }
        u8g2.drawStr(destPos,LINE1-1,clipDestination);
        // Set font back to standard
        u8g2.setFont(NatRailSmall9);
    } else {
        // ---- Draw Secondary Service lines ----
        char clipDestination[30];
        char ordinal[5];

        switch (line) {
            case 1: strcpy(ordinal,"2nd "); break;
            case 2: strcpy(ordinal,"3rd "); break;
            default: sprintf(ordinal,"%dth ",line+1); break;
        }

        u8g2.setFont(NatRailSmall9);
        blankArea(0,yPos,256,9);

        if (line<stationData.numServices) {
            u8g2.drawStr(0,yPos-1,ordinal);
            int destPos = u8g2.drawStr(23,yPos-1,stationData.service[line].sTime) + 27;
            char plat[3];
            if (stationData.platformAvailable && !hidePlatform) {
                if (stationData.service[line].platform[0] && strlen(stationData.service[line].platform)<3 && stationData.service[line].serviceType == NR_SERVICE_TRAIN) {
                    strncpy(plat,stationData.service[line].platform,3);
                    plat[2]='\0';
                } else {
                    if (stationData.service[line].serviceType == NR_SERVICE_BUS) strcpy(plat,"\x86");  // Bus icon
                    else strcpy(plat,"\x96\x96");
                }
                u8g2.drawStr(destPos+11-getStringWidth(plat),yPos-1,plat);
                destPos+=16;
            }
            char etd[16];
            if (isDigit(stationData.service[line].etd[0])) sprintf(etd,"Exp %s",stationData.service[line].etd);
            else strcpy(etd,stationData.service[line].etd);
            int etdWidth = getStringWidth(etd);
            u8g2.drawStr(SCREEN_WIDTH - etdWidth,yPos-1,etd);
            // work out if we need to clip the destination
            strcpy(clipDestination,stationData.service[line].destination);

            int spaceAvailable = SCREEN_WIDTH - destPos - etdWidth - 6;

            if (getStringWidth(clipDestination) > spaceAvailable) {
                while (getStringWidth(clipDestination) > spaceAvailable - 17) {
                    clipDestination[strlen(clipDestination)-1] = '\0';
                }
                if (clipDestination[strlen(clipDestination)-1] == ' ') clipDestination[strlen(clipDestination)-1] = '\0';
                strcat(clipDestination,"...");
            }
            u8g2.drawStr(destPos,yPos-1,clipDestination);
        } else {
            // We're showing the mandatory attribution
            centreText(nrAttributionn,yPos-1);
        }
    }
}

// (Definitions moved to file scope)

void NationalRailBoard::render(U8G2& u8g2) {
  numMessages=0;
  // Clear the top two lines
  blankArea(0,LINE0,256,LINE2-1);
  drawHeader(u8g2);

  // Draw the primary service line
  isShowingVia=false;
  viaTimer=millis()+300000;  // effectively don't check for via
  if (stationData.numServices) {
    drawService(u8g2, 0, LINE1);
    if (stationData.service[0].via[0]) viaTimer=millis()+4000;
    if (stationData.service[0].isCancelled) {
      // This train is cancelled
      if (stationData.service[0].serviceMessage[0]) {
        strcpy(line2[0],stationData.service[0].serviceMessage);
        numMessages=1;
      }
    } else {
      // The train is not cancelled
      if (stationData.service[0].isDelayed && stationData.service[0].serviceMessage[0]) {
        // The train is delayed and there's a reason
        strcpy(line2[0],stationData.service[0].serviceMessage);
        numMessages++;
      }
      if (stationData.service[0].calling[0]) {
        // Add the calling stops message
        sprintf(line2[numMessages],"Calling at: %s",stationData.service[0].calling);
        numMessages++;
      }
      if (strcmp(stationData.service[0].origin, stationData.location)==0) {
        // Service originates at this station
        if (stationData.service[0].opco[0]) {
          sprintf(line2[numMessages],"This %s service starts here.",stationData.service[0].opco);
        } else {
          strcpy(line2[numMessages],"This service starts here.");
        }
        // Add the seating if available
        switch (stationData.service[0].classesAvailable) {
          case 1:
            strcat(line2[numMessages],firstClassSeating);
            break;
          case 2:
            strcat(line2[numMessages],standardClassSeating);
            break;
          case 3:
            strcat(line2[numMessages],dualClassSeating);
            break;
        }
        numMessages++;
      } else {
        // Service originates elsewhere
        strcpy(line2[numMessages],"");
        if (stationData.service[0].opco[0]) {
          if (stationData.service[0].origin[0]) {
            sprintf(line2[numMessages],"This is the %s service from %s.",stationData.service[0].opco,stationData.service[0].origin);
          } else {
            sprintf(line2[numMessages],"This is the %s service.",stationData.service[0].opco);
          }
        } else {
          if (stationData.service[0].origin[0]) {
            sprintf(line2[numMessages],"This service originated at %s.",stationData.service[0].origin);
          }
        }
        // Add the seating if available
        switch (stationData.service[0].classesAvailable) {
          case 1:
            strcat(line2[numMessages],firstClassSeating);
            break;
          case 2:
            strcat(line2[numMessages],standardClassSeating);
            break;
          case 3:
            strcat(line2[numMessages],dualClassSeating);
            break;
        }
        if (line2[numMessages][0]) numMessages++;
      }
      if (stationData.service[0].trainLength) {
        // Add the number of carriages message
        sprintf(line2[numMessages],"This train is formed of %d coaches.",stationData.service[0].trainLength);
        numMessages++;
      }
    }
    // Add any nrcc messages
    for (int i=0;i<messages.numMessages;i++) {
      strcpy(line2[numMessages],messages.messages[i]);
      numMessages++;
    }
    // Setup for the first message to rollover to
    isScrollingStops=false;
    currentMessage=numMessages-1;
    if (noScrolling && stationData.numServices>1) {
      drawService(u8g2, 1, LINE2);
    }
  } else {
    blankArea(0,LINE2,256,LINE4-LINE2);
    u8g2.setFont(NatRailTall12);
    centreText("There are no scheduled services at this station.",LINE1-1);
    numMessages = messages.numMessages;
    for (int i=0;i<messages.numMessages;i++) {
      strcpy(line2[i],messages.messages[i]);
    }
    // Setup for the first message to rollover to
    isScrollingStops=false;
    currentMessage=numMessages-1;
  }
  u8g2.setFont(NatRailSmall9);
  u8g2.sendBuffer();
}

void NationalRailBoard::tick(uint32_t currentMillis) {
    extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2;
    extern bool noScrolling;

  if (currentMillis>timer && numMessages && !isScrollingStops && !noScrolling) {
    // Need to start a new scrolling line 2
    prevMessage = currentMessage;
    prevScrollStopsLength = scrollStopsLength;
    currentMessage++;
    if (currentMessage>=numMessages) currentMessage=0;
    scrollStopsXpos=0;
    scrollStopsYpos=10;
    scrollStopsLength = getStringWidth(line2[currentMessage]);
    isScrollingStops=true;
  }

  // Check if there's a via destination
  if (currentMillis>viaTimer) {
    if (stationData.numServices && stationData.service[0].via[0]) {
      isShowingVia = !isShowingVia;
      drawService(u8g2, 0, LINE1);
      u8g2.updateDisplayArea(0,1,32,3);
      if (isShowingVia) viaTimer = currentMillis+3000; else viaTimer = currentMillis+4000;
    }
  }

  if (currentMillis>serviceTimer && !isScrollingService) {
    // Need to change to the next service if there is one
    if (stationData.numServices <= 1 && !currentWeather->getWeatherMsg()[0]) {
      // There's no other services and no weather so just so static attribution.
      // drawService(u8g2, 1, LINE3); //TODO? No need to redraw statically every tick.
      serviceTimer = currentMillis + 30000;
      isScrollingService = false;
    } else {
      prevService = line3Service;
      line3Service++;
      if (stationData.numServices) {
        if ((line3Service>stationData.numServices && !currentWeather->getWeatherMsg()[0]) || (line3Service>stationData.numServices+1 && currentWeather->getWeatherMsg()[0])) line3Service=(noScrolling && stationData.numServices>1) ? 2:1;  // First 'other' service
      } else {
        if (currentWeather->getWeatherMsg()[0] && line3Service>1) line3Service=0;
      }
      scrollServiceYpos=10;
      isScrollingService = true;
    }
  }

  if (isScrollingStops && currentMillis>timer && !noScrolling) {
    blankArea(0,LINE2,256,9);
    if (scrollStopsYpos) {
      // we're scrolling up the message initially
      u8g2.setClipWindow(0,LINE2,256,LINE2+9);
      // if the previous message didn't scroll then we need to scroll it up off the screen
      if (prevScrollStopsLength && prevScrollStopsLength<256 && strncmp("Calling",line2[prevMessage],7)) centreText(line2[prevMessage],scrollStopsYpos+LINE2-12);
      if (scrollStopsLength<256 && strncmp("Calling",line2[currentMessage],7)) centreText(line2[currentMessage],scrollStopsYpos+LINE2-2); // Centre text if it fits
      else u8g2.drawStr(0,scrollStopsYpos+LINE2-2,line2[currentMessage]);
      u8g2.setMaxClipWindow();
      scrollStopsYpos--;
      if (scrollStopsYpos==0) timer=currentMillis+1500;
    } else {
      // we're scrolling left
      if (scrollStopsLength<256 && strncmp("Calling",line2[currentMessage],7)) centreText(line2[currentMessage],LINE2-1); // Centre text if it fits
      else u8g2.drawStr(scrollStopsXpos,LINE2-1,line2[currentMessage]);
      if (scrollStopsLength < 256) {
        // we don't need to scroll this message, it fits so just set a longer timer
        timer=currentMillis+6000;
        isScrollingStops=false;
      } else {
        scrollStopsXpos--;
        if (scrollStopsXpos < -scrollStopsLength) {
          isScrollingStops=false;
          timer=currentMillis+500;  // pause before next message
        }
      }
    }
  }

  if (isScrollingService && currentMillis>serviceTimer) {
    blankArea(0,LINE3,256,9);
    if (scrollServiceYpos) {
      // we're scrolling the service into view
      u8g2.setClipWindow(0,LINE3,256,LINE3+9);
      // if the prev service is showing, we need to scroll it up off
      if (prevService>0) drawService(u8g2, prevService, scrollServiceYpos+LINE3-12);
      drawService(u8g2, line3Service, scrollServiceYpos+LINE3);
      u8g2.setMaxClipWindow();
      scrollServiceYpos--;
      if (scrollServiceYpos==0) {
        serviceTimer=currentMillis+5000;
        isScrollingService=false;
      }
    }
  }

  // To ensure visual scrolling frames are sent during tick():
  u8g2.updateDisplayArea(0,3,32,4);
}
