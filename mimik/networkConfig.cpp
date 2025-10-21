/*
 * mimik - Mini Shell para ESP32-CAM
 * networkConfig.cpp - Implementación de configuración persistente
 */

#include "networkConfig.h"

bool NetworkConfigManager::loadConfig(NetworkConfig* config) {
    Serial.print("Looking for config at: ");
    Serial.println(CONFIG_FILE);
    Serial.print("File exists: ");
    Serial.println(SD_MMC.exists(CONFIG_FILE) ? "YES" : "NO");
    if(!SD_MMC.exists(CONFIG_FILE)) {
        Serial.println("Network config file not found");
        return false;
    }
    
    File file = SD_MMC.open(CONFIG_FILE, FILE_READ);
    if(!file) {
        Serial.println("ERROR: Cannot open network config file");
        return false;
    }
    
    // Leer línea por línea
    memset(config, 0, sizeof(NetworkConfig));
    
    while(file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        
        // Ignorar líneas vacías o comentarios
        if(line.length() == 0 || line.startsWith("#")) {
            continue;
        }
        
        // Parsear key=value
        int separatorIndex = line.indexOf('=');
        if(separatorIndex > 0) {
            String key = line.substring(0, separatorIndex);
            String value = line.substring(separatorIndex + 1);
            
            key.trim();
            value.trim();
            
            if(key == "SSID") {
                strncpy(config->ssid, value.c_str(), sizeof(config->ssid) - 1);
            } else if(key == "PASSWORD") {
                strncpy(config->password, value.c_str(), sizeof(config->password) - 1);
            } else if(key == "USE_STATIC_IP") {
                config->useStaticIP = (value == "true" || value == "1");
            } else if(key == "STATIC_IP") {
                strncpy(config->staticIP, value.c_str(), sizeof(config->staticIP) - 1);
            } else if(key == "NETMASK") {
                strncpy(config->netmask, value.c_str(), sizeof(config->netmask) - 1);
            } else if(key == "GATEWAY") {
                strncpy(config->gateway, value.c_str(), sizeof(config->gateway) - 1);
            }
        }
    }
    
    file.close();
    return true;
}

bool NetworkConfigManager::saveConfig(const NetworkConfig* config) {
    File file = SD_MMC.open(CONFIG_FILE, FILE_WRITE);
    if(!file) {
        Serial.println("ERROR: Cannot write network config file");
        return false;
    }
    
    // Escribir configuración
    file.println("# mimik Network Configuration");
    file.println("# Auto-generated - DO NOT EDIT MANUALLY");
    file.println();
    
    file.print("SSID=");
    file.println(config->ssid);
    
    file.print("PASSWORD=");
    file.println(config->password);
    
    file.print("USE_STATIC_IP=");
    file.println(config->useStaticIP ? "true" : "false");
    
    if(config->useStaticIP) {
        file.print("STATIC_IP=");
        file.println(config->staticIP);
        
        file.print("NETMASK=");
        file.println(config->netmask);
        
        file.print("GATEWAY=");
        file.println(config->gateway);
    }
    
    file.close();
    Serial.println("Network configuration saved to SD card");
    return true;
}

bool NetworkConfigManager::autoConnect() {
    NetworkConfig config;
    
    if(!loadConfig(&config)) {
        Serial.println("No saved network configuration found");
        return false;
    }
    
    if(strlen(config.ssid) == 0) {
        Serial.println("ERROR: SSID is empty in config");
        return false;
    }
    
    Serial.print("Auto-connecting to WiFi: ");
    Serial.println(config.ssid);
    
    // Configurar IP estática si está habilitada
    if(config.useStaticIP && strlen(config.staticIP) > 0) {
        IPAddress ip, netmask, gateway;
        
        if(ip.fromString(config.staticIP) && 
           netmask.fromString(config.netmask) && 
           gateway.fromString(config.gateway)) {
            
            Serial.println("Configuring static IP...");
            WiFi.config(ip, gateway, netmask);
        } else {
            Serial.println("WARNING: Invalid static IP configuration, using DHCP");
        }
    }
    
    // Conectar a WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);
    
    int attempts = 0;
    while(WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Gateway:    ");
        Serial.println(WiFi.gatewayIP());
        return true;
    } else {
        Serial.println("ERROR: WiFi connection failed");
        return false;
    }
}

bool NetworkConfigManager::saveWiFiCredentials(const char* ssid, const char* password) {
    NetworkConfig config;
    
    // Cargar configuración existente (para mantener IP estática si existe)
    loadConfig(&config);
    
    // Actualizar credenciales
    strncpy(config.ssid, ssid, sizeof(config.ssid) - 1);
    strncpy(config.password, password, sizeof(config.password) - 1);
    
    return saveConfig(&config);
}

bool NetworkConfigManager::saveStaticIP(const char* ip, const char* netmask, const char* gateway) {
    NetworkConfig config;
    
    // Cargar configuración existente
    if(!loadConfig(&config)) {
        // Si no existe, crear nueva con valores vacíos
        memset(&config, 0, sizeof(NetworkConfig));
    }
    
    // Actualizar IP estática
    config.useStaticIP = true;
    strncpy(config.staticIP, ip, sizeof(config.staticIP) - 1);
    strncpy(config.netmask, netmask, sizeof(config.netmask) - 1);
    strncpy(config.gateway, gateway, sizeof(config.gateway) - 1);
    
    return saveConfig(&config);
}

bool NetworkConfigManager::clearConfig() {
    if(SD_MMC.exists(CONFIG_FILE)) {
        return SD_MMC.remove(CONFIG_FILE);
    }
    return true;
}