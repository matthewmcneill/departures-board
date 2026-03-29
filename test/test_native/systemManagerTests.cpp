#include <unity.h>
#include "SystemState.hpp"
#include "appContext.hpp"
#include "systemManager.hpp"
#include "displayManager.hpp"
#include "iDisplayBoard.hpp"

class MockBoard : public iDisplayBoard {
public:
    MockBoard() : status(-1) {}
    int updateData() override { return status; }
    void render(U8G2& u8g2) override {}
    int getLastUpdateStatus() const override { return status; }
    void setStatus(int s) { status = s; }
    
    // Minimal mock for other virtuals
    const char* getBoardName() const override { return "Mock"; }
    void onActivate() override {}
    void onDeactivate() override {}
    void tick(uint32_t ms) override {}
    const char* getLastErrorMsg() override { return ""; }
    void onConfigUpdate(const struct Config& config) override {}
    void configure(const struct BoardConfig& config) override {}
    bool isComplete() const override { return true; }
    WeatherStatus& getWeatherStatus() override { static WeatherStatus ws; return ws; }

    int status;
};

// Mock DisplayManager
class MockDisplayManager : public displayManager {
public:
    MockDisplayManager(appContext& ctx) : displayManager(ctx) {
        for(int i=0; i<MAX_BOARDS; i++) boards[i] = nullptr;
    }
    void setBoard(int idx, iDisplayBoard* b) { boards[idx] = b; }
    iDisplayBoard* getDisplayBoard(int idx) override { return boards[idx]; }
    void render() override {}
private:
    iDisplayBoard* boards[MAX_BOARDS];
};

void test_system_manager_round_robin() {
    appContext ctx;
    systemManager& sm = ctx.getSystemManager();
    // In our mock appContext, getSystemManager() will return a real systemManager.
    // However, it might need to be initialized.
    sm.begin(&ctx);
    
    Config& cfg = ctx.getConfigManager().getConfig();
    
    // Setup 3 boards
    cfg.boardCount = 3;
    cfg.apiRefreshRate = 150000; // 150s
    cfg.boards[0].complete = true;
    cfg.boards[1].complete = true;
    cfg.boards[2].complete = true;

    // Trigger now
    sm.setNextRoundRobinUpdate(0); 
    sm.setWifiConnected(true);
    
    // Tick
    sm.tick();
    
    // Verify increment logic side effects if possible, or just ensure no crash.
    // Since backgroundUpdateIndex is private, we'll verify nextRoundRobinUpdate.
    TEST_ASSERT_TRUE(sm.getNextRoundRobinUpdate() > 0);
}

void runSystemManagerTests() {
    RUN_TEST(test_system_manager_round_robin);
}
