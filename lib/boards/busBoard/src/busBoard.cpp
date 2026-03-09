/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/busBoard/src/busBoard.cpp
 * Description: Concrete implementation of OLED rendering for Bus departure boards.
 */

#include "../include/busBoard.hpp"
extern void blankArea(int x, int y, int w, int h);
extern void centreText(const char *msg, int yConstraint);
extern int getStringWidth(const char *str);
extern const uint8_t NatRailSmall9[];
extern const uint8_t Underground10[];

extern const char btAttribution[]; // bustimes.org attribution

extern void tflCallback();
extern stnMessages messages;
extern void drawCurrentTimeUG(bool);

// Shared constants for Screen Dimensions
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 64
#endif
#ifndef ULINE0
#define ULINE0 0
#define ULINE1 15
#define ULINE2 28
#define ULINE3 41
#define ULINE4 56
#endif



/**
 * @brief Retrieves the human-readable name of the bus stop being displayed.
 * @return The human-readable name of the bus stop as a C-string.
 */
const char* BusBoard::getLocationName() const {
    return busName;
}

/**
 * @brief Pull down the latest departures from the Bus Times HTML API!
 */
int BusBoard::updateData() {
    busDataClient* dataClient = new busDataClient();
    if (!dataClient) {
        strcpy(lastErrorMsg, "Out of Memory");
        return 6; // UPD_DATA_ERROR mapping
    }

    // Always re-clean the filter before parsing in case user changed it in web UI
    dataClient->cleanFilter(busFilter, cleanBusFilter, sizeof(busFilter));

    int status = dataClient->updateDepartures(&stationData, busAtco, cleanBusFilter, &tflCallback);
    strncpy(lastErrorMsg, dataClient->lastErrorMsg, sizeof(lastErrorMsg)-1);
    delete dataClient;
    return status;
}

/**
 * @brief Render the top header bar of the screen for the Bus board.
 *        Draws the bus stop name and active filter.
 * @param u8g2 Reference to the global U8g2 object.
 */
void BusBoard::drawHeader(U8G2& u8g2) const {
  // Clear the top line
  blankArea(0,0,256,14);

  u8g2.setFont(NatRailSmall9);
  char boardTitle[95];
  String title = String(busName) + " ";
  if (busFilter[0]) title+="\x8D" + String(busFilter);
  title.trim();
  strncpy(boardTitle,title.c_str(),sizeof(boardTitle));

  centreText(boardTitle,-1);
}

/**
 * @brief Render a single row/service onto the OLED screen for a Bus.
 *        Omits the platform column and handles bus-specific formatting.
 * @param u8g2 Reference to the global U8g2 object.
 * @param line The 0-based index of the service to draw.
 * @param yPos The pixel Y-coordinate for the baseline of this row.
 */
void BusBoard::drawService(U8G2& u8g2, int line, int yPos) const {
    if (line<stationData.numServices) {
        char clipDestination[30];
        
        u8g2.setFont(Underground10);
        blankArea(0,yPos,256,10);
        
        u8g2.drawStr(0,yPos-1,stationData.service[line].routeNumber);
        
        char etd[16];
        if (isDigit(stationData.service[line].expectedTime[0])) sprintf(etd,"%s min",stationData.service[line].expectedTime);
        else strcpy(etd,stationData.service[line].expectedTime);
        int etdWidth = getStringWidth(etd);
        u8g2.drawStr(256 - etdWidth,yPos-1,etd);
        
        // work out if we need to clip the destination
        strcpy(clipDestination,stationData.service[line].destination);
        int spaceAvailable = 256 - busDestX - etdWidth - 6;

        if (getStringWidth(clipDestination) > spaceAvailable) {
            while (getStringWidth(clipDestination) > spaceAvailable - 17) {
                clipDestination[strlen(clipDestination)-1] = '\0';
            }
            if (clipDestination[strlen(clipDestination)-1] == ' ') clipDestination[strlen(clipDestination)-1] = '\0';
            strcat(clipDestination,"...");
        }
        u8g2.drawStr(busDestX,yPos-1,clipDestination);
    } else {
        // We're showing the mandatory attribution
        u8g2.setFont(NatRailSmall9);
        centreText(btAttribution,yPos-2);
    }
}

void BusBoard::render(U8G2& u8g2) {
  numMessages = messages.numMessages;
  if (line3Service==0) line3Service=2;
  blankArea(0,ULINE0,256,ULINE1-1);
  drawHeader(u8g2);

  if (stationData.boardChanged) {
    // prepare to scroll up primary services
    scrollPrimaryYpos = 11;
    isScrollingPrimary = true;
    // reset line3
    line3Service = 99;
    prevScrollStopsLength = 0;
    currentMessage=99;
    blankArea(0,ULINE3,256,11);
    serviceTimer=0;
  } else {
    // Draw the primary service line(s)
    if (stationData.numServices) {
      drawService(u8g2, 0, ULINE1);
      if (stationData.numServices>1) drawService(u8g2, 1, ULINE2);
      if (stationData.numServices>2) drawService(u8g2, 2, ULINE3);
    } else {
      u8g2.setFont(Underground10);
      centreText("There are no scheduled services at this stop.",ULINE1-1);
    }
  }
  for (int i=0;i<messages.numMessages;i++) {
    strcpy(line2[i],messages.messages[i]);
  }
  u8g2.sendBuffer();
}

void BusBoard::tick(uint32_t currentMillis) {
    extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2;
    extern bool noScrolling;
    bool fullRefresh = false;
    
    // Scrolling the additional services
    if (currentMillis>serviceTimer && !isScrollingService) {
      // Need to change to the next service if there is one
      if (stationData.numServices<=2 && messages.numMessages==1) {
      // There are no additional services or weather to scroll in so static attribution.
      serviceTimer = currentMillis + 10000;
      line3Service=stationData.numServices;
    } else {
      // Need to change to the next service or message
      prevService = line3Service;
      line3Service++;
      scrollServiceYpos=11;
      isScrollingService = true;
      if (line3Service>=stationData.numServices) {
        // Showing the messages
        prevMessage = currentMessage;
        currentMessage++;
        if (currentMessage>=messages.numMessages) {
          if (stationData.numServices>2) {
            line3Service = 2;
            currentMessage=-1; // Rollover back to services
          } else {
            line3Service = stationData.numServices;
            currentMessage=0;
          }
        }
      }
    }
  }

  if (isScrollingService && currentMillis>serviceTimer) {
    if (scrollServiceYpos) {
      blankArea(0,ULINE3,256,10);
      // we're scrolling up the message
      u8g2.setClipWindow(0,ULINE3,256,ULINE3+10);
      // Was the previous display a service?
      if (prevService<stationData.numServices) {
        drawService(u8g2, prevService, scrollServiceYpos+ULINE3-12);
      } else {
        // Scrolling up the previous message
        centreText(line2[prevMessage],scrollServiceYpos+ULINE3-13);
      }
      // Is this entry a service?
      if (line3Service<stationData.numServices) {
        drawService(u8g2, line3Service, scrollServiceYpos+ULINE3);
      } else {
        centreText(line2[currentMessage],scrollServiceYpos+ULINE3-2);
      }
      u8g2.setMaxClipWindow();
      scrollServiceYpos--;
      if (scrollServiceYpos==0) {
        serviceTimer = currentMillis+2800;
        if (stationData.numServices<=2) serviceTimer+=3000;
      }
    } else isScrollingService=false;
  }

  if (isScrollingPrimary) {
    blankArea(0,ULINE1,256,ULINE3-ULINE1+10);
    fullRefresh = true;
    // we're scrolling the primary service(s) into view
    u8g2.setClipWindow(0,ULINE1,256,ULINE1+10);
    if (stationData.numServices) drawService(u8g2, 0, scrollPrimaryYpos+ULINE1);
    else centreText("There are no scheduled services at this stop.",scrollPrimaryYpos+ULINE1-1);
    if (stationData.numServices>1) {
      u8g2.setClipWindow(0,ULINE2,256,ULINE2+10);
      drawService(u8g2, 1, scrollPrimaryYpos+ULINE2);
    }
    if (stationData.numServices>2) {
      u8g2.setClipWindow(0,ULINE3,256,ULINE3+10);
      drawService(u8g2, 2, scrollPrimaryYpos+ULINE3);
    } else if (stationData.numServices<3 && messages.numMessages==1) {
      // scroll up the attribution once...
      u8g2.setClipWindow(0,ULINE3,256,ULINE3+10);
      centreText(btAttribution,scrollPrimaryYpos+ULINE3-1);
    }
    u8g2.setMaxClipWindow();
    scrollPrimaryYpos--;
    if (scrollPrimaryYpos==0) {
      isScrollingPrimary=false;
      serviceTimer = currentMillis+2800;
    }
  }

  // Check if the clock should be updated
  if (currentMillis>nextClockUpdate) {
    nextClockUpdate = currentMillis+250;
    drawCurrentTimeUG(true);    // just use the Tube clock for bus mode
    u8g2.setFont(NatRailSmall9);
  }

  long delayMs = 40 - (currentMillis-refreshTimer);
  if (delayMs>0) delay(delayMs);
  if (fullRefresh) u8g2.updateDisplayArea(0,1,32,6); else u8g2.updateDisplayArea(0,5,32,2);
  refreshTimer=currentMillis;
}
