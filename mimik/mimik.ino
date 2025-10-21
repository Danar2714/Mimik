/*
 * ███╗   ███╗██╗███╗   ███╗██╗██╗  ██╗
 * ████╗ ████║██║████╗ ████║██║██║ ██╔╝
 * ██╔████╔██║██║██╔████╔██║██║█████╔╝ 
 * ██║╚██╔╝██║██║██║╚██╔╝██║██║██╔═██╗ 
 * ██║ ╚═╝ ██║██║██║ ╚═╝ ██║██║██║  ██╗
 * ╚═╝     ╚═╝╚═╝╚═╝     ╚═╝╚═╝╚═╝  ╚═╝
 * 
 * mimik - Mini Shell para ESP32-CAM AI-Thinker
 * 
 * Librerías requeridas:
 * - LibSSH-ESP32 de Ewan Parker (para fase 2)
 * - FreeRTOS (incluido en ESP32 core)
 * 
 * Pines SD Card en AI-Thinker ESP32-CAM:
 * CMD: GPIO 15
 * CLK: GPIO 14
 * D0:  GPIO 2
 * 
 * Autor: Veras
 * Versión: 1.0 - Fase 1
 */

#include "shell.h"

// Declaraciones externas
extern bool initShellTasks();

void setup() {
    // Inicializar Serial
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("====================================");
    Serial.println("  ███╗   ███╗██╗███╗   ███╗██╗██╗  ██╗");
    Serial.println("  ████╗ ████║██║████╗ ████║██║██║ ██╔╝");
    Serial.println("  ██╔████╔██║██║██╔████╔██║██║█████╔╝ ");
    Serial.println("  ██║╚██╔╝██║██║██║╚██╔╝██║██║██╔═██╗ ");
    Serial.println("  ██║ ╚═╝ ██║██║██║ ╚═╝ ██║██║██║  ██╗");
    Serial.println("  ╚═╝     ╚═╝╚═╝╚═╝     ╚═╝╚═╝╚═╝  ╚═╝");
    Serial.println();
    Serial.println("  Mini Shell ESP32-CAM v1.0");
    Serial.println("====================================");
    Serial.println();
    
    // Inicializar el shell
    if(!shell.init()) {
        Serial.println("ERROR FATAL: No init");
        Serial.println("Check SD card");
        while(1) {
            delay(1000);
        }
    }
    
    // Crear tareas del shell (FreeRTOS ya está iniciado automáticamente)
    if(!initShellTasks()) {
        Serial.println("ERROR: No task creation");
        while(1) {
            delay(1000);
        }
    }
    
    Serial.println();
    Serial.println("╔═══════════════════════════════════════╗");
    Serial.println("║  System  ready!                       ║");
    Serial.println("║  'help' for command info              ║");
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.println();
    
    // Mostrar prompt inicial
    shell.printPrompt();
}

void loop() {
    // El loop principal puede estar vacío ya que FreeRTOS maneja las tareas
    // Aquí puedes añadir funcionalidades adicionales si es necesario
    delay(100);
}