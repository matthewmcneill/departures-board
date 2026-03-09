/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/tflBoard/src/tflBoard.cpp
 * Description: Concrete implementation of OLED rendering for TfL Underground boards.
 */

#include "../include/tflBoard.hpp"
extern void blankArea(int x, int y, int w, int h);
extern void centreText(const char *msg, int yConstraint);
extern int getStringWidth(const char *str);
extern const uint8_t Underground10[];
extern const uint8_t NatRailSmall9[];
extern const char tflAttribution[];
extern stnMessages messages;
extern void tflCallback();
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
 * @brief Retrieves the human-readable name of the tube station being displayed.
 * @return The human-readable name of the tube station as a C-string.
 */
const char* TfLBoard::getLocationName() const {
    return tubeName;
}

/**
 * @brief Pull down the latest departures from the TfL JSON API!
 */
int TfLBoard::updateData() {
    TfLdataClient* dataClient = new TfLdataClient();
    if (!dataClient) {
        strcpy(lastErrorMsg, "Out of Memory");
        return 6; // UPD_DATA_ERROR
    }

    int status = dataClient->updateArrivals(&stationData, &messages, tubeId, String(tflAppkey), &tflCallback);
    strncpy(lastErrorMsg, dataClient->lastErrorMsg, sizeof(lastErrorMsg)-1);
    delete dataClient;
    return status;
}

/**
 * @brief Render the top header bar of the screen for the TfL Underground board.
 *        Draws the station name centered on the top line.
 * @param u8g2 Reference to the global U8g2 object.
 */
void TfLBoard::drawHeader(U8G2& u8g2) const {
  // Clear the top line
  blankArea(0,0,256,14);

  u8g2.setFont(NatRailSmall9);
  char boardTitle[95];
  String title = String(tubeName) + " ";
  title.trim();
  strncpy(boardTitle,title.c_str(),sizeof(boardTitle));

  centreText(boardTitle,-1);
}

/**
 * @brief Render a single row/service onto the OLED screen for a Tube train.
 *        Handles line names as via data and draws the time to station.
 * @param u8g2 Reference to the global U8g2 object.
 * @param line The 0-based index of the service to draw.
 * @param yPos The pixel Y-coordinate for the baseline of this row.
 */
void TfLBoard::drawService(U8G2& u8g2, int line, int yPos) const {
    if (line<stationData.numServices) {
        char clipDestination[30];
        
        u8g2.setFont(Underground10);
        blankArea(0,yPos,256,10);
        
        int destPos = u8g2.drawStr(0,yPos-1,stationData.service[line].lineName) + 5;
        
        char etd[16];
        int mins = stationData.service[line].timeToStation / 60;
        if (mins == 0) strcpy(etd,"Due");
        else sprintf(etd,"%d min%s",mins, mins>1?"s":"");
        int etdWidth = getStringWidth(etd);
        u8g2.drawStr(256 - etdWidth,yPos-1,etd);
        
        // work out if we need to clip the destination
        strcpy(clipDestination,stationData.service[line].destination);
        int spaceAvailable = 256 - destPos - etdWidth - 6;

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
        u8g2.setFont(NatRailSmall9);
        centreText(tflAttribution,yPos-2);
    }
}

void TfLBoard::render(U8G2& u8g2) {
  numMessages = messages.numMessages;
  if (line3Service==0) line3Service=1;
  attributionScrolled=false;
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
    } else {
      u8g2.setFont(Underground10);
      centreText("There are no scheduled arrivals at this station.",ULINE1-1);
    }
  }
  for (int i=0;i<messages.numMessages;i++) {
    strcpy(line2[i],messages.messages[i]);
  }
  // Add attribution msg
  strcpy(line2[messages.numMessages],tflAttribution);
  messages.numMessages++;

  u8g2.sendBuffer();

}

void TfLBoard::tick(uint32_t currentMillis) {
    extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2;
    extern bool noScrolling;
    bool fullRefresh = false;
    // Scrolling the additional services
  if (currentMillis>serviceTimer && !isScrollingService) {
    if (stationData.numServices<=2 && messages.numMessages==1 && attributionScrolled) {
      // There are no additional services to scroll in so static attribution.
      serviceTimer = currentMillis + 30000;
    } else {
      // Need to change to the next service or message if there is one
      attributionScrolled = true;
      prevService = line3Service;
      line3Service++;
      scrollServiceYpos=11;
      scrollStopsXpos=0;
      isScrollingService = true;
      if (line3Service>=stationData.numServices) {
        // Showing the messages
        prevMessage = currentMessage;
        prevScrollStopsLength = scrollStopsLength;  // Save the length of the previous message
        currentMessage++;
        if (currentMessage>=messages.numMessages) {
          if (stationData.numServices>2) {
            line3Service=2;
            currentMessage=-1; // Rollover back to services
          } else {
            line3Service = stationData.numServices;
            currentMessage=0;
          }
        }
        scrollStopsLength = getStringWidth(line2[currentMessage]);
      } else {
        scrollStopsLength=SCREEN_WIDTH;
      }
    }
  }

  if (isScrollingService && currentMillis>serviceTimer) {
    blankArea(0,ULINE3,256,10);
    if (scrollServiceYpos) {
      // we're scrolling up the message initially
      u8g2.setClipWindow(0,ULINE3,256,ULINE3+10);
      // Was the previous display a service?
      if (prevService<stationData.numServices) {
        drawService(u8g2, prevService, scrollServiceYpos+ULINE3-12);
      } else {
        // if the previous message didn't scroll then we need to scroll it up off the screen
        if (prevScrollStopsLength && prevScrollStopsLength<256) centreText(line2[prevMessage],scrollServiceYpos+ULINE3-13);
      }
      // Is this entry a service?
      if (line3Service<stationData.numServices) {
        drawService(u8g2, line3Service, scrollServiceYpos+ULINE3);
      } else {
        if (scrollStopsLength<256) centreText(line2[currentMessage],scrollServiceYpos+ULINE3-2); // Centre text if it fits
        else u8g2.drawStr(0,scrollServiceYpos+ULINE3-2,line2[currentMessage]);
      }
      u8g2.setMaxClipWindow();
      scrollServiceYpos--;
      if (scrollServiceYpos==0) {
        if (line3Service<stationData.numServices) {
          serviceTimer=currentMillis+3500;
          isScrollingService=false;
        } else {
          serviceTimer=currentMillis+500;
        }
      }
    } else {
      // we're scrolling left
      if (scrollStopsLength<256) centreText(line2[currentMessage],ULINE3-1); // Centre text if it fits
      else u8g2.drawStr(scrollStopsXpos,ULINE3-1,line2[currentMessage]);
      if (scrollStopsLength < 256) {
        // we don't need to scroll this message, it fits so just set a longer timer
        serviceTimer=currentMillis+3000;
        isScrollingService=false;
      } else {
        scrollStopsXpos--;
        if (scrollStopsXpos < -scrollStopsLength) {
          isScrollingService=false;
          serviceTimer=currentMillis+500;  // pause before next message
        }
      }
    }
  }

  if (isScrollingPrimary) {
    blankArea(0,ULINE1,256,ULINE3-ULINE1);
    fullRefresh = true;
    // we're scrolling the primary service(s) into view
    u8g2.setClipWindow(0,ULINE1,256,ULINE1+10);
    if (stationData.numServices) drawService(u8g2, 0, scrollPrimaryYpos+ULINE1);
    else centreText("There are no scheduled arrivals at this station.",scrollPrimaryYpos+ULINE1-1);
    if (stationData.numServices>1) {
      u8g2.setClipWindow(0,ULINE2,256,ULINE2+10);
      drawService(u8g2, 1, scrollPrimaryYpos+ULINE2);
    }
    u8g2.setMaxClipWindow();
    scrollPrimaryYpos--;
    if (scrollPrimaryYpos==0) {
      isScrollingPrimary=false;
    }
  }

  // Check if the clock should be updated
  if (currentMillis>nextClockUpdate) {
    nextClockUpdate = currentMillis+250;
    drawCurrentTimeUG(true);
  }

  long delayMs = 18 - (currentMillis-refreshTimer);
  if (delayMs>0) delay(delayMs);
  if (fullRefresh) u8g2.updateDisplayArea(0,1,32,6); else u8g2.updateDisplayArea(0,5,32,2);
  refreshTimer=currentMillis;

}
