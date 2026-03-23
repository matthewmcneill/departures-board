#ifndef MOCK_TIME_MANAGER_HPP
#define MOCK_TIME_MANAGER_HPP

#include <time.h>
#include <stdint.h>

class TimeManager {
private:
    struct tm currentTime;

public:
    TimeManager() {
        updateCurrentTime();
    }

    void updateCurrentTime() {
        time_t now = time(NULL);
        struct tm *local = localtime(&now);
        if (local) {
            currentTime = *local;
        }
    }

    struct tm& getCurrentTime() {
        return currentTime;
    }
};

#endif // MOCK_TIME_MANAGER_HPP
