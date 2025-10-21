/*
 * mimik - Mini Shell para ESP32-CAM
 * monitorCommands.cpp - Comandos de monitoreo
 */

#include "shell.h"
#include "sshServer.h"

// Comando: top - Mostrar uso de recursos del sistema
ShellError MiniShell::cmd_top(CommandArgs args) {
    ShellOutput::println("\n=== System Resources Monitor ===\n");
    
    // Información de CPU
    ShellOutput::println("CPU:");
    ShellOutput::println("-------------------------------");
    ShellOutput::print("  Cores:      ");
    ShellOutput::println(2); // ESP32 tiene 2 cores
    ShellOutput::print("  Frequency:  ");
    ShellOutput::print((int)ESP.getCpuFreqMHz());
    ShellOutput::println(" MHz");
    ShellOutput::print("  Uptime:     ");
    
    unsigned long uptimeMs = millis();
    unsigned long seconds = uptimeMs / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    if(days > 0) {
        ShellOutput::printf("%lu days, ", days);
    }
    ShellOutput::printf("%02lu:%02lu:%02lu\n", hours % 24, minutes % 60, seconds % 60);
    
    ShellOutput::println();
    
    // Información de RAM
    ShellOutput::println("Memory (RAM):");
    ShellOutput::println("-------------------------------");
    
    uint32_t heapSize = ESP.getHeapSize();
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t usedHeap = heapSize - freeHeap;
    uint32_t heapPercent = (usedHeap * 100) / heapSize;
    
    ShellOutput::printf("  Total:      %u KB\n", heapSize / 1024);
    ShellOutput::printf("  Used:       %u KB\n", usedHeap / 1024);
    ShellOutput::printf("  Free:       %u KB\n", freeHeap / 1024);
    ShellOutput::printf("  Usage:      %u%%\n", heapPercent);
    ShellOutput::print("  Bar:        [");
    
    // Barra de progreso
    int bars = heapPercent / 5; // 20 caracteres máximo
    for(int i = 0; i < 20; i++) {
        if(i < bars) ShellOutput::print("#");
        else ShellOutput::print("-");
    }
    ShellOutput::println("]");
    
    ShellOutput::println();
    
    // Información de PSRAM (si está disponible)
    if(psramFound()) {
        ShellOutput::println("PSRAM:");
        ShellOutput::println("-------------------------------");
        
        uint32_t psramSize = ESP.getPsramSize();
        uint32_t freePsram = ESP.getFreePsram();
        uint32_t usedPsram = psramSize - freePsram;
        uint32_t psramPercent = (usedPsram * 100) / psramSize;
        
        ShellOutput::printf("  Total:      %u KB\n", psramSize / 1024);
        ShellOutput::printf("  Used:       %u KB\n", usedPsram / 1024);
        ShellOutput::printf("  Free:       %u KB\n", freePsram / 1024);
        ShellOutput::printf("  Usage:      %u%%\n", psramPercent);
        
        ShellOutput::println();
    }
    
    // Información de Flash (programa)
    ShellOutput::println("Flash (Program):");
    ShellOutput::println("-------------------------------");
    
    uint32_t flashSize = ESP.getFlashChipSize();
    uint32_t sketchSize = ESP.getSketchSize();
    uint32_t freeFlash = ESP.getFreeSketchSpace();
    uint32_t flashPercent = (sketchSize * 100) / flashSize;
    
    ShellOutput::printf("  Total:      %u KB\n", flashSize / 1024);
    ShellOutput::printf("  Program:    %u KB\n", sketchSize / 1024);
    ShellOutput::printf("  Free:       %u KB\n", freeFlash / 1024);
    ShellOutput::printf("  Usage:      %u%%\n", flashPercent);
    
    ShellOutput::println();
    
    // Información de SD Card
    ShellOutput::println("Storage (SD Card):");
    ShellOutput::println("-------------------------------");
    
    if(SD_MMC.cardType() != CARD_NONE) {
        uint64_t cardSize = SD_MMC.cardSize();
        uint64_t totalBytes = SD_MMC.totalBytes();
        uint64_t usedBytes = SD_MMC.usedBytes();
        uint64_t freeBytes = totalBytes - usedBytes;
        uint32_t sdPercent = (usedBytes * 100) / totalBytes;
        
        ShellOutput::printf("  Total:      %llu MB\n", cardSize / (1024 * 1024));
        ShellOutput::printf("  Used:       %llu MB\n", usedBytes / (1024 * 1024));
        ShellOutput::printf("  Free:       %llu MB\n", freeBytes / (1024 * 1024));
        ShellOutput::printf("  Usage:      %u%%\n", sdPercent);
        ShellOutput::print("  Bar:        [");
        
        // Barra de progreso
        int bars = sdPercent / 5;
        for(int i = 0; i < 20; i++) {
            if(i < bars) ShellOutput::print("#");
            else ShellOutput::print("-");
        }
        ShellOutput::println("]");
    } else {
        ShellOutput::println("  Status:     Not available");
    }
    
    ShellOutput::println();
    
    // Información de tareas (opcional)
    ShellOutput::println("Tasks:");
    ShellOutput::println("-------------------------------");
    ShellOutput::printf("  Active:     %u\n", uxTaskGetNumberOfTasks());
    ShellOutput::printf("  Stack Free: %u bytes (shell task)\n", 
                  uxTaskGetStackHighWaterMark(NULL));
    
    ShellOutput::println();
    
    return SHELL_OK;
}