/*
 * mimik - Mini Shell para ESP32-CAM
 * ShellTasks.cpp - Gestión de tareas con FreeRTOS
 */

#include "shell.h"
#include "sshServer.h"
#include <esp_task_wdt.h>

// Handles de las tareas
TaskHandle_t shellTaskHandle = NULL;
TaskHandle_t sdMonitorTaskHandle = NULL;
TaskHandle_t telnetTaskHandle = NULL;

// Tarea principal del shell
void shellTask(void* parameter) {
    while(true) {
        shell.processInput();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// Tarea de monitoreo de SD (opcional)
void sdMonitorTask(void* parameter) {
    while(true) {
        // Obtener información de la tarjeta SD
        uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
        uint64_t usedSize = SD_MMC.usedBytes() / (1024 * 1024);
        uint64_t totalSize = SD_MMC.totalBytes() / (1024 * 1024);
        
        // Esta información se puede usar para estadísticas
        
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

// Tarea del servidor Telnet
void telnetTask(void* parameter) {
    // Esperar a que WiFi esté conectado
    //Serial.println("Telnet task: Waiting for WiFi connection...");
    while(WiFi.status() != WL_CONNECTED) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    //Serial.println("Telnet task: WiFi connected, starting server...");
    
    // Iniciar servidor Telnet
    if(sshServer.begin()) {
        //Serial.println("Telnet server started successfully");
    } else {
        Serial.println("Failed to start Telnet server");
        vTaskDelete(NULL);
        return;
    }
    
    // Loop del servidor Telnet
    while(true) {
        sshServer.loop();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// Función de inicialización de tareas
bool initShellTasks() {
    // NO inicializar watchdog - ya está inicializado por el sistema
    // Si necesitas configurarlo, hazlo en setup() ANTES de crear tareas
    
    // Crear tarea del shell
    BaseType_t result = xTaskCreatePinnedToCore(
        shellTask,
        "ShellTask",
        4096,
        NULL,
        1,
        &shellTaskHandle,
        1
    );
    
    if(result != pdPASS) {
        Serial.println("ERROR: Cannot create shell task");
        return false;
    }
    
    // Crear tarea de monitoreo
    result = xTaskCreatePinnedToCore(
        sdMonitorTask,
        "SDMonitor",
        2048,
        NULL,
        0,
        &sdMonitorTaskHandle,
        0
    );
    
    if(result != pdPASS) {
        Serial.println("WARNING: Cannot create monitor task");
    }
    
    // Crear tarea Telnet
    result = xTaskCreatePinnedToCore(
        telnetTask,
        "TelnetTask",
        8192,  // 8KB de stack es suficiente para Telnet
        NULL,
        1,
        &telnetTaskHandle,
        0  // Core 0
    );
    
    if(result != pdPASS) {
        Serial.println("WARNING: Cannot create Telnet task");
    } else {
        Serial.println("Telnet task created successfully");
    }
    
    Serial.println("mimik tasks initialized successfully");
    return true;
}