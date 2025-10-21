/*
 * mimik - Mini Shell para ESP32-CAM
 * networkConfig.h - Gestión de configuración de red persistente
 */

#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <WiFi.h>

#define CONFIG_FILE "/networkConfig.cfg"

// Estructura de configuración de red
struct NetworkConfig {
    char ssid[64];
    char password[64];
    bool useStaticIP;
    char staticIP[16];
    char netmask[16];
    char gateway[16];
};

class NetworkConfigManager {
public:
    // Cargar configuración desde SD
    static bool loadConfig(NetworkConfig* config);
    
    // Guardar configuración a SD
    static bool saveConfig(const NetworkConfig* config);
    
    // Conectar WiFi usando configuración guardada
    static bool autoConnect();
    
    // Guardar credenciales WiFi
    static bool saveWiFiCredentials(const char* ssid, const char* password);
    
    // Guardar IP estática
    static bool saveStaticIP(const char* ip, const char* netmask, const char* gateway);
    
    // Limpiar configuración
    static bool clearConfig();
};

#endif