/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * bustimes.org Client Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/busDataClient/busDataClient.cpp
 * Description: Implementation of the bustimes.org client.
 *
 * Exported Functions/Classes:
 * - busDataClient::stripTag: Extracts the inner text from an HTML tag string.
 * - busDataClient::replaceWord: Replaces all occurrences of a target string with a replacement string in-place.
 * - busDataClient::trim: Trims leading and trailing whitespace from a character array.
 * - busDataClient::equalsIgnoreCase: Compares two character arrays case-insensitively.
 * - busDataClient::serviceMatchesFilter: Checks if a specific bus service ID matches a filter list.
 * - busDataClient::cleanFilter: Normalizes a user-provided filter string.
 * - busDataClient::updateDepartures: Scrapes and parses the HTML departures board.
 * - busDataClient::getStopLongName: Retrieves the official long name of a bus stop.
 * - busDataClient::whitespace: JSON whitespace handler.
 * - busDataClient::startDocument: JSON handler triggered at start of document.
 * - busDataClient::key: JSON handler triggered for each object key.
 * - busDataClient::value: JSON handler triggered for each key value.
 * - busDataClient::endArray: JSON handler triggered when exiting an array.
 * - busDataClient::endObject: JSON handler triggered when exiting an object.
 * - busDataClient::endDocument: JSON handler triggered at end of document.
 * - busDataClient::startArray: JSON handler triggered when entering an array.
 * - busDataClient::startObject: JSON handler triggered when entering an object.
 */

#include <busDataClient.h>
#include <JsonListener.h>
#include <WiFiClientSecure.h>
#include <stationData.h>

busDataClient::busDataClient() {}

/**
 * @brief Extracts the inner text from an HTML tag string.
 * @param html The full HTML tag string to parse.
 * @return The inner text stripped of its surrounding tags.
 */
String busDataClient::stripTag(String html) {
    String result = "";
    int start = html.indexOf(">");
    int end = html.indexOf("</");
    if (start!=1 && end!=1 && end>start) {
        result = html.substring(start+1,end);
        result.trim();
    }
    return result;
}

/**
 * @brief Replaces all occurrences of a target string with a replacement string in-place.
 * @param input The string buffer to modify.
 * @param target The exact word or phrase to look for.
 * @param replacement The string to inject in place of the target.
 */
void busDataClient::replaceWord(char* input, const char* target, const char* replacement) {
    // Find the first occurrence of the target word
    char* pos = strstr(input, target);
    while (pos) {
        // Calculate the length of the target word
        size_t targetLen = strlen(target);
        // Calculate the length difference between target and replacement
        int diff = strlen(replacement) - targetLen;

        // Shift the remaining characters to accommodate the replacement
        memmove(pos + strlen(replacement), pos + targetLen, strlen(pos + targetLen) + 1);

        // Copy the replacement word into the position
        memcpy(pos, replacement, strlen(replacement));

        // Find the next occurrence of the target word
        pos = strstr(pos + strlen(replacement), target);
    }
}

/**
 * @brief Trims leading and trailing whitespace from a character array in-place.
 * @param start Pointer to the start of the character array.
 * @param end Pointer to the end of the character array.
 */
void busDataClient::trim(char* &start, char* &end) {
  while (start <= end && isspace(*start)) start++;
  while (end >= start && isspace(*end)) end--;
}

/**
 * @brief Compares two character arrays case-insensitively.
 * @param a The first character array.
 * @param a_len The length of the first character array.
 * @param b The second character array (null-terminated).
 * @return True if the strings match regardless of case.
 */
bool busDataClient::equalsIgnoreCase(const char* a, int a_len, const char* b) {
  for (int i = 0; i < a_len; i++) {
    if (tolower(a[i]) != tolower(b[i])) return false;
  }
  return b[a_len] == '\0';
}

/**
 * @brief Checks if a specific bus service ID matches a provided comma-separated filter list.
 * @param filter The comma-separated string of desired bus route numbers.
 * @param serviceId The currently parsed bus route number.
 * @return True if the route is found in the filter, or if the filter is empty.
 */
bool busDataClient::serviceMatchesFilter(const char* filter, const char* serviceId) {
  if (filter == nullptr || filter[0] == '\0') return true; // empty filter = match all

  const char* start = filter;
  const char* ptr = filter;

  while (true) {
    if (*ptr == ',' || *ptr == '\0') {
      const char* end = ptr - 1;
      char* trimStart = const_cast<char*>(start);
      char* trimEnd   = const_cast<char*>(end);
      trim(trimStart, trimEnd);
      int len = trimEnd - trimStart + 1;
      if (len > 0 && equalsIgnoreCase(trimStart, len, serviceId)) {
        return true;
      }
      if (*ptr == '\0') break;
      ptr++;
      start = ptr;
    } else {
      ptr++;
    }
  }

  return false;
}

/**
 * @brief Normalizes a user-provided filter string by making it lowercase and removing spaces.
 * @param rawFilter The original filter input.
 * @param cleanedFilter The output buffer for the normalized filter.
 * @param maxLen The maximum length of the output buffer.
 */
void busDataClient::cleanFilter(const char* rawFilter, char* cleanedFilter, size_t maxLen) {
    if (!rawFilter || rawFilter[0] == '\0') {
        if (maxLen > 0) cleanedFilter[0] = '\0';
        return;
    }

    size_t j = 0;
    const char* ptr = rawFilter;

    while (*ptr != '\0' && j < maxLen - 1) {
        if (*ptr == ',') {
            cleanedFilter[j++] = ',';
            ptr++;
            continue;
        }
        if (!isspace(*ptr)) {
            cleanedFilter[j++] = tolower(*ptr);
        }
        ptr++;
    }

    cleanedFilter[j] = '\0';
    return;
}

/**
 * @brief Connects to bustimes.org, scrapes the HTML departures board, and parses the vehicle times.
 * @param station Pointer to the global rdStation structure to populate with arrival times.
 * @param locationId The bustimes.org Stop ID.
 * @param filter A comma-separated list of bus routes to filter by (or empty for all).
 * @param Xcb Function callback to execute UI ticks while blocking and waiting for the API.
 * @return A connection status constant.
 */
int busDataClient::updateDepartures(rdStation *station, const char *locationId, const char *filter, busClientCallback Xcb) {

    unsigned long perfTimer=millis();
    long dataReceived = 0;
    bool bChunked = false;
    lastErrorMsg = "";

    WiFiClientSecure httpsClient;
    httpsClient.setInsecure();
    httpsClient.setTimeout(5000);
    httpsClient.setConnectionTimeout(5000);
    station->boardChanged=false;

    int retryCounter=0;
    while (!httpsClient.connect(apiHost,443) && (retryCounter++ < 10)){
        delay(200);
    }
    if (retryCounter>=10) {
        lastErrorMsg = F("Connection timeout");
        return UPD_NO_RESPONSE;
    }
    String request = "GET /stops/" + String(locationId) + F("/departures HTTP/1.0\r\nHost: ") + String(apiHost) + F("\r\nConnection: close\r\n\r\n");
    httpsClient.print(request);
    Xcb();
    unsigned long ticker = millis()+800;
    retryCounter=0;
    while(!httpsClient.available() && retryCounter++ < 40) {
        delay(200);
    }

    if (!httpsClient.available()) {
        // no response within 8 seconds so exit
        httpsClient.stop();
        lastErrorMsg = F("Response timeout");
        return UPD_TIMEOUT;
    }

    // Parse status code
    String statusLine = httpsClient.readStringUntil('\n');
    if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
        httpsClient.stop();

        if (statusLine.indexOf(F("401")) > 0 || statusLine.indexOf(F("429")) > 0) {
            lastErrorMsg = F("Not Authorized");
            return UPD_UNAUTHORISED;
        } else if (statusLine.indexOf(F("500")) > 0) {
            lastErrorMsg = statusLine;
            return UPD_DATA_ERROR;
        } else {
            lastErrorMsg = statusLine;
            return UPD_HTTP_ERROR;
        }
    }

    // Skip the remaining headers
    while (httpsClient.connected() || httpsClient.available()) {
        String line = httpsClient.readStringUntil('\n');
        if (line == F("\r")) break;
        if (line.startsWith(F("Transfer-Encoding:")) && line.indexOf(F("chunked")) >= 0) bChunked=true;
    }

    // Start scraping the data
    unsigned long dataSendTimeout = millis() + 10000UL;
    id=0;
    bool maxServicesRead = false;
    xBusStop.numServices = 0;
    for (int i=0;i<MAXBOARDSERVICES;i++) {
        strcpy(xBusStop.service[i].destinationName,"Check front of bus");
        strcpy(xBusStop.service[i].scheduled,"");
        strcpy(xBusStop.service[i].expected,"");
    }
    int parseStep = PBT_START; // looking for the start of data
    int dataColumns = 0;
    bool serviceData;
    String serviceId;
    String destination;

    while((httpsClient.available() || httpsClient.connected()) && (millis() < dataSendTimeout) && (!maxServicesRead)) {
        while(httpsClient.available() && !maxServicesRead) {
            String line = httpsClient.readStringUntil('\n');
            dataReceived+=line.length()+1;
            line.trim();
            if (line.length()) {
                if (line.indexOf("</body>")>=0) {
                    // end of page
                    maxServicesRead = true;
                } else {
                    switch (parseStep) {
                        case PBT_START:
                            if (line.indexOf("<tr>")>=0) parseStep = PBT_HEADER;
                            break;

                        case PBT_HEADER:
                            if (line.indexOf("</tr>")>=0) {
                                parseStep = PBT_SERVICE;
                                serviceData = false;
                            }
                            else if (line.substring(0,1)=="<") dataColumns++;
                            break;

                        case PBT_SERVICE:
                            if (line.indexOf("</table>")>=0) {
                                // Assume another day of data with headers
                                dataColumns=0;
                                parseStep = PBT_START;
                            }
                            else if (line.indexOf("</td>")>=0) parseStep = PBT_DESTINATION;
                            else if (line.substring(0,3)=="<td") serviceData = true;
                            else if (line.substring(0,7)=="<a href" && serviceData) {
                                // Get the service name from within the hyperlink
                                serviceId = stripTag(line);
                                strncpy(xBusStop.service[id].lineName,serviceId.c_str(),MAXBUSLINESIZE-1);
                                xBusStop.service[id].lineName[MAXBUSLINESIZE-1] = '\0';
                            } else {
                                // must be a service Id without hyperlink
                                serviceId = line;
                                strncpy(xBusStop.service[id].lineName,serviceId.c_str(),MAXBUSLINESIZE-1);
                                xBusStop.service[id].lineName[MAXBUSLINESIZE-1] = '\0';
                            }
                            break;

                        case PBT_DESTINATION:
                            if (line.indexOf("</td>")>=0) parseStep = PBT_SCHEDULED;
                            else if (line.substring(0,1)!="<") {
                                strncpy(xBusStop.service[id].destinationName,line.c_str(),MAXLOCATIONSIZE-1);
                                xBusStop.service[id].destinationName[MAXLOCATIONSIZE-1] = '\0';
                            } else if (line.indexOf("class=\"vehicle\"")>=0) {
                                // Get the vehicle details
                                String vehicle = stripTag(line);
                                // Strip off ticket m/c if it's included
                                int tikregsep = vehicle.indexOf(" - ");
                                if (tikregsep>0) {
                                    vehicle = vehicle.substring(tikregsep+3);
                                    vehicle.trim();
                                }
                                if ((strlen(xBusStop.service[id].destinationName) + vehicle.length() + 3) < sizeof(xBusStop.service[id].destinationName)) {
                                    sprintf(xBusStop.service[id].destinationName,"%s (%s)",xBusStop.service[id].destinationName,vehicle.c_str());
                                }
                            }
                            break;

                        case PBT_SCHEDULED:
                            if (line.indexOf("</td>")>=0) {
                                if (dataColumns == 4) parseStep = PBT_EXPECTED; else {
                                    strcpy(xBusStop.service[id].expected,"");
                                    parseStep = PBT_HEADER;
                                    if (serviceMatchesFilter(filter,xBusStop.service[id].lineName)) id++;
                                    if (id>=MAXBOARDSERVICES) maxServicesRead=true;
                                }
                            } else if (line.substring(0,1)!="<") {
                                strncpy(xBusStop.service[id].scheduled,line.c_str(),sizeof(xBusStop.service[id].scheduled));
                                xBusStop.service[id].scheduled[sizeof(xBusStop.service[id].scheduled)-1] = '\0';
                            }
                            break;

                        case PBT_EXPECTED:
                            if (line.indexOf("</td>")>=0) {
                                parseStep = PBT_HEADER;
                                if (serviceMatchesFilter(filter,xBusStop.service[id].lineName)) id++;
                                if (id>=MAXBOARDSERVICES) maxServicesRead=true;
                            }
                            else if (line.substring(0,1)!="<") {
                                strncpy(xBusStop.service[id].expected,line.c_str(),sizeof(xBusStop.service[id].expected));
                                xBusStop.service[id].expected[sizeof(xBusStop.service[id].expected)-1] = '\0';
                            }
                            break;
                    }
                }
            }
            if (millis()>ticker) {
                Xcb();
                ticker = millis()+800;
            }
        }
    }

    httpsClient.stop();
    if (millis() >= dataSendTimeout) {
        lastErrorMsg = F("Timed out during msgs data receive operation");
        return UPD_TIMEOUT;
    }

    xBusStop.numServices = id;

    // Remove &amp; from destination name
    for (int i=0;i<xBusStop.numServices;i++) replaceWord(xBusStop.service[i].destinationName,"&amp;","&");

    // Check if any of the services have changed
    if (xBusStop.numServices != station->numServices) station->boardChanged=true;
    else {
        for (int i=0;i<xBusStop.numServices;i++) {
            if (i>1) break; // Only check first two services
            if (strcmp(xBusStop.service[i].destinationName,station->service[i].destination) || strcmp(xBusStop.service[i].lineName,station->service[i].via)) {
                station->boardChanged=true;
                break;
            }
        }
    }

    // Update the callers data with the new data
    station->numServices = xBusStop.numServices;
    for (int i=0;i<xBusStop.numServices;i++) {
        strcpy(station->service[i].destination,xBusStop.service[i].destinationName);
        strcpy(station->service[i].via,xBusStop.service[i].lineName);
        strcpy(station->service[i].sTime,xBusStop.service[i].scheduled);
        strcpy(station->service[i].etd,xBusStop.service[i].expected);
    }

    if (bChunked) lastErrorMsg = F("WARNING: Chunked response! ");
    if (station->boardChanged) {
        lastErrorMsg += F("SUCCESS [Primary Service Changed] Update took: ");
        lastErrorMsg += String(millis() - perfTimer) + F("ms [") + String(dataReceived) + F("]");
        return UPD_SUCCESS;
    } else {
        lastErrorMsg += F("SUCCESS Update took: ");
        lastErrorMsg += String(millis() - perfTimer) + F("ms [") + String(dataReceived) + F("]");
        return UPD_NO_CHANGE;
    }
}

/**
 * @brief Connects to bustimes.org API to retrieve the official long name of a bus stop.
 * @param locationId The stop ID.
 * @param locationName Output buffer to store the retrieved name.
 * @return A connection status constant (e.g. UPD_SUCCESS, UPD_TIMEOUT).
 */
int busDataClient::getStopLongName(const char *locationId, char *locationName) {

    unsigned long perfTimer=millis();
    long dataReceived = 0;
    bool bChunked = false;
    lastErrorMsg = "";

    JsonStreamingParser parser;
    parser.setListener(this);
    WiFiClientSecure httpsClient;
    httpsClient.setInsecure();
    httpsClient.setTimeout(5000);

    int retryCounter=0;
    while (!httpsClient.connect(apiHost,443) && (retryCounter++ < 15)){
        delay(200);
    }
    if (retryCounter>=15) {
        lastErrorMsg = F("Connection timeout");
        return UPD_NO_RESPONSE;
    }
    String request = "GET /api/stops/" + String(locationId) + F(" HTTP/1.0\r\nHost: ") + String(apiHost) + F("\r\nConnection: close\r\n\r\n");
    httpsClient.print(request);
    retryCounter=0;
    while(!httpsClient.available() && retryCounter++ < 40) {
        delay(200);
    }

    if (!httpsClient.available()) {
        // no response within 8 seconds so exit
        httpsClient.stop();
        lastErrorMsg = F("Response timeout");
        return UPD_TIMEOUT;
    }

    // Parse status code
    String statusLine = httpsClient.readStringUntil('\n');
    if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
        httpsClient.stop();

        if (statusLine.indexOf(F("401")) > 0 || statusLine.indexOf(F("429")) > 0) {
            lastErrorMsg = F("Not Authorized");
            return UPD_UNAUTHORISED;
        } else if (statusLine.indexOf(F("500")) || statusLine.indexOf(F("404")) > 0) {
            lastErrorMsg = statusLine;
            return UPD_DATA_ERROR;
        } else {
            lastErrorMsg = statusLine;
            return UPD_HTTP_ERROR;
        }
    }

    // Skip the remaining headers
    while (httpsClient.connected() || httpsClient.available()) {
        String line = httpsClient.readStringUntil('\n');
        if (line == F("\r")) break;
        if (line.startsWith(F("Transfer-Encoding:")) && line.indexOf(F("chunked")) >= 0) bChunked=true;
    }

    bool isBody = false;
    char c;
    longName = "";
    unsigned long dataSendTimeout = millis() + 10000UL;
    while((httpsClient.available() || httpsClient.connected()) && (millis() < dataSendTimeout)) {
        while(httpsClient.available()) {
            c = httpsClient.read();
            dataReceived++;
            if (c == '{' || c == '[') isBody = true;
            if (isBody) parser.parse(c);
        }
        delay(25);
    }
    httpsClient.stop();
    if (millis() >= dataSendTimeout) {
        lastErrorMsg = F("Timed out during data receive operation - ");
        lastErrorMsg += String(dataReceived) + F(" bytes received");
        return UPD_TIMEOUT;
    }

    strncpy(locationName,longName.c_str(),sizeof(locationName)-1);
    locationName[sizeof(locationName)-1] = '\0';

    if (bChunked) lastErrorMsg = F("WARNING: Chunked response! ");
    lastErrorMsg += F("SUCCESS Update took: ");
    lastErrorMsg += String(millis() - perfTimer) + F("ms [") + String(dataReceived) + F("]");
    return UPD_SUCCESS;
}

/**
 * @brief JSON whitespace handler.
 * @param c The whitespace char.
 */
void busDataClient::whitespace(char c) {}

/**
 * @brief JSON handler triggered at start of document.
 */
void busDataClient::startDocument() {}

/**
 * @brief JSON handler triggered for each object key.
 * @param key The string name of the parsed key.
 */
void busDataClient::key(String key) {
    currentKey = key;
}

/**
 * @brief JSON handler triggered for each key value.
 * @param value The scalar string value.
 */
void busDataClient::value(String value) {
    if (currentKey == F("long_name")) longName = value;
}

/**
 * @brief JSON handler triggered when exiting an array.
 */
void busDataClient::endArray() {}

/**
 * @brief JSON handler triggered when exiting an object.
 */
void busDataClient::endObject() {}

/**
 * @brief JSON handler triggered at end of document.
 */
void busDataClient::endDocument() {}

/**
 * @brief JSON handler triggered when entering an array.
 */
void busDataClient::startArray() {}

/**
 * @brief JSON handler triggered when entering an object.
 */
void busDataClient::startObject() {}
