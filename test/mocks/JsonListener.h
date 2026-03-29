#ifndef JSON_LISTENER_H
#define JSON_LISTENER_H

class JsonListener {
public:
    virtual ~JsonListener() {}
    virtual void key(const char* key) {}
    virtual void value(const char* value) {}
    virtual void startObject() {}
    virtual void endObject() {}
    virtual void startArray() {}
    virtual void endArray() {}
};

#endif
