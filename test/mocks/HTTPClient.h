#ifndef MOCK_HTTPCLIENT_H
#define MOCK_HTTPCLIENT_H

#include <WString.h>
#include "WiFiClient.h"

#define HTTPC_STRICT_FOLLOW_REDIRECTS 1
#define HTTP_CODE_OK 200

class HTTPClient {
public:
    HTTPClient() {}
    ~HTTPClient() {}
    bool begin(const String& url) { return true; }
    bool begin(WiFiClient& client, const String& url) { return true; }
    void addHeader(const String& name, const String& value) {}
    int GET() { return 200; }
    String getString() { return "{}"; }
    void end() {}
    void setFollowRedirects(int mode) {}
    void setTimeout(int timeout) {}
    int getSize() { return 0; }
    WiFiClient* getStreamPtr() { return nullptr; }
};

#endif // MOCK_HTTPCLIENT_H
