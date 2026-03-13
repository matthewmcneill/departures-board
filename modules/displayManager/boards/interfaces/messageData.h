#ifndef MESSAGE_DATA_H
#define MESSAGE_DATA_H

#define MAXBOARDMESSAGES 4 // Maximum number of station-wide messages to store
#define MAXMESSAGESIZE 400 // Maximum string length for each individual message

/**
 * @brief Structure for holding station-wide messages (e.g., delays, status updates).
 *        Used by National Rail and TfL boards.
 */
struct stnMessages {
    int numMessages;
    char messages[MAXBOARDMESSAGES][MAXMESSAGESIZE];
};

#endif // MESSAGE_DATA_H
