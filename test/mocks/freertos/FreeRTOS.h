#ifndef MOCK_FREERTOS_H
#define MOCK_FREERTOS_H

#include <stdint.h>

typedef uint32_t TickType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;
typedef void* TaskHandle_t;
typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;

#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
#define pdFAIL 0

#define portMAX_DELAY (TickType_t)0xffffffffUL
#define configTICK_RATE_HZ 1000

#define pdMS_TO_TICKS( xTimeInMs ) ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000 ) )
#define tskIDLE_PRIORITY 0

#ifdef __cplusplus
extern "C" {
#endif

void* xQueueCreate(uint32_t length, uint32_t itemSize);
long xQueueSendToFront(void* queue, const void* item, TickType_t ticksToWait);
long xQueueSend(void* queue, const void* item, TickType_t ticksToWait);
long xQueueReceive(void* queue, void* buffer, TickType_t ticksToWait);
void* xSemaphoreCreateBinary();
void* xSemaphoreCreateMutex();
long xSemaphoreTake(void* semaphore, TickType_t ticksToWait);
long xSemaphoreGive(void* semaphore);
#define vSemaphoreDelete(xSemaphore) {}

typedef void (*TaskFunction_t)(void*);
long xTaskCreatePinnedToCore(TaskFunction_t pvTaskCode, const char* const pcName, const uint32_t usStackDepth, void* const pvParameters, UBaseType_t uxPriority, TaskHandle_t* const pvCreatedTask, const BaseType_t xCoreID);
void vTaskDelay(TickType_t xTicksToDelay);

#ifdef __cplusplus
}
#endif

#endif
