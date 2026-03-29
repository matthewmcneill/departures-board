#ifndef MESSAGE_POOL_MOCK_HPP
#define MESSAGE_POOL_MOCK_HPP

#include <Arduino.h>
#include <string>
#include <vector>

class MessagePool {
public:
    MessagePool() {}
    void addMessage(const char* msg) { messages.push_back(msg); }
    void removeLastMessage() { if (!messages.empty()) messages.pop_back(); }
    int getCount() const { return (int)messages.size(); }
    
    std::vector<std::string> messages;
};

#endif
