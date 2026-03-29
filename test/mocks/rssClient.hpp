#ifndef RSS_CLIENT_MOCK_HPP
#define RSS_CLIENT_MOCK_HPP

#include <Arduino.h>

class rssClient {
public:
    rssClient() : rssEnabled(true), rssAddedtoMsgs(false), numRssTitles(0) {}
    bool getRssEnabled() const { return rssEnabled; }
    void setRssAddedtoMsgs(bool a) { rssAddedtoMsgs = a; }
    bool getRssAddedtoMsgs() const { return rssAddedtoMsgs; }
    const char* getRssName() const { return "MockRSS"; }
    
    bool rssEnabled;
    bool rssAddedtoMsgs;
    int numRssTitles;
    char rssTitle[5][100];
};

#endif
