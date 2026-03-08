/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/stationData/stationData.h
 * Description: Common station data structures shared by multiple data clients.
 *
 * Exported Functions/Classes:
 * - struct stnMessages: Stores messages to be displayed.
 * - struct rdService: Stores details for a single departure vehicle (train, bus, tube).
 * - struct rdStation: Stores all departure data for a given station.
 */
#pragma once
#include <Arduino.h>

#define MAXBOARDMESSAGES 4
#define MAXMESSAGESIZE 400
#define MAXCALLINGSIZE 450
#define MAXBOARDSERVICES 9
#define MAXLOCATIONSIZE 45

#define OTHER 0
#define TRAIN 1
#define BUS 2

#define UPD_SUCCESS 0
#define UPD_INCOMPLETE 1
#define UPD_UNAUTHORISED 2
#define UPD_HTTP_ERROR 3
#define UPD_TIMEOUT 4
#define UPD_NO_RESPONSE 5
#define UPD_DATA_ERROR 6
#define UPD_NO_CHANGE 7

/**
 * @brief Structure for holding station-wide messages
 */
struct stnMessages {
    int numMessages;
    char messages[MAXBOARDMESSAGES][MAXMESSAGESIZE];
};

/**
 * @brief Structure for holding details about a single vehicle service
 */
struct rdService {
    char sTime[6];
    char destination[MAXLOCATIONSIZE];
    char via[MAXLOCATIONSIZE];  // also used for line name for TfL
    char etd[11];
    char platform[4];
    bool isCancelled;
    bool isDelayed;
    int trainLength;
    byte classesAvailable;
    char opco[50];

    int serviceType;
    int timeToStation;  // Only for TfL
  };

  /**
   * @brief Structure for holding all information relative to a specific station
   */
  struct rdStation {
    char location[MAXLOCATIONSIZE];
    bool platformAvailable;
    int numServices;
    bool boardChanged;  // Only for TfL
    char calling[MAXCALLINGSIZE];   // Only store the calling stops for the first service returned
    char origin[MAXLOCATIONSIZE]; // Only store the origin for the first service returned
    char serviceMessage[MAXMESSAGESIZE];  // Only store the service message for the first service returned
    rdService service[MAXBOARDSERVICES];
  };
