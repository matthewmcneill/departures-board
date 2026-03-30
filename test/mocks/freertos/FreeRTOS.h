#pragma once
#include <stdint.h>
#include <stddef.h>

typedef void* TaskHandle_t;
typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;
typedef uint32_t TickType_t;

#define portMAX_DELAY (TickType_t)0xFFFFFFFF
#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
#define pdFAIL 0
#define pdMS_TO_TICKS(x) (x)
#define tskIDLE_PRIORITY 0

inline TaskHandle_t xTaskCreatePinnedToCore(void (*task)(void*), const char* name, int stack, void* param, int prio, TaskHandle_t* handle, int core) { return (TaskHandle_t)1; }
inline QueueHandle_t xQueueCreate(int len, int size) { return (QueueHandle_t)1; }
inline int xQueueSend(QueueHandle_t q, const void* item, TickType_t wait) { return pdTRUE; }
inline int xQueueSendToFront(QueueHandle_t q, const void* item, TickType_t wait) { return pdTRUE; }
inline int xQueueReceive(QueueHandle_t q, void* item, TickType_t wait) { return pdTRUE; }
inline SemaphoreHandle_t xSemaphoreCreateMutex() { return (SemaphoreHandle_t)1; }
inline SemaphoreHandle_t xSemaphoreCreateBinary() { return (SemaphoreHandle_t)1; }
inline int xSemaphoreTake(SemaphoreHandle_t s, TickType_t wait) { return pdTRUE; }
inline int xSemaphoreGive(SemaphoreHandle_t s) { return pdTRUE; }
inline void vTaskDelay(TickType_t ticks) {}
inline void vTaskDelete(TaskHandle_t t) {}
inline const char* pcTaskGetName(TaskHandle_t t) { return "mock_task"; }

#define xSemaphoreTakeRecursive(s, w) xSemaphoreTake(s, w)
#define xSemaphoreGiveRecursive(s) xSemaphoreGive(s)
#define xSemaphoreCreateRecursiveMutex() xSemaphoreCreateMutex()
