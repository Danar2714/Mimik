/*
 * mimik - Mini Shell para ESP32-CAM
 * networkCommands.cpp - Comandos de networking
 */

#include "shell.h"
#include "sshServer.h"
#include "networkConfig.h"


// Comando: netconfig - Mostrar configuración guardada
ShellError MiniShell::cmd_netconfig(CommandArgs args) {
    NetworkConfig config;
    
    if(!NetworkConfigManager::loadConfig(&config)) {
        ShellOutput::println("No saved network configuration");
        return SHELL_OK;
    }
    
    ShellOutput::println("\n=== Saved Network Configuration ===\n");
    ShellOutput::print("SSID:       ");
    ShellOutput::println(config.ssid);
    ShellOutput::print("Password:   ");
    ShellOutput::println("********");  // No mostrar password
    ShellOutput::print("Static IP:  ");
    ShellOutput::println(config.useStaticIP ? "Enabled" : "Disabled");
    
    if(config.useStaticIP) {
        ShellOutput::print("IP:         ");
        ShellOutput::println(config.staticIP);
        ShellOutput::print("Netmask:    ");
        ShellOutput::println(config.netmask);
        ShellOutput::print("Gateway:    ");
        ShellOutput::println(config.gateway);
    }
    
    ShellOutput::println();
    return SHELL_OK;
}

// Comando: netclear - Limpiar configuración
ShellError MiniShell::cmd_netclear(CommandArgs args) {
    if(NetworkConfigManager::clearConfig()) {
        ShellOutput::println("Network configuration cleared");
        ShellOutput::println("WiFi will NOT auto-connect on next boot");
        return SHELL_OK;
    } else {
        ShellOutput::println("ERROR: Cannot clear configuration");
        return SHELL_ERR_PERMISSION;
    }
}
// Comando: ifconfig - Mostrar información de interfaces de red
ShellError MiniShell::cmd_ifconfig(CommandArgs args) {
    ShellOutput::println("\n=== Network Interfaces ===\n");
    
    // Interfaz WiFi
    ShellOutput::println("wlan0: WiFi Interface");
    ShellOutput::println("-------------------------------");
    
    if(WiFi.status() == WL_CONNECTED) {
        ShellOutput::print("  Status:     CONNECTED\n");
        ShellOutput::print("  SSID:       ");
        ShellOutput::println(WiFi.SSID());
        ShellOutput::print("  IP Address: ");
        ShellOutput::println(WiFi.localIP().toString());
        ShellOutput::print("  Netmask:    ");
        ShellOutput::println(WiFi.subnetMask().toString());
        ShellOutput::print("  Gateway:    ");
        ShellOutput::println(WiFi.gatewayIP().toString());
        ShellOutput::print("  DNS:        ");
        ShellOutput::println(WiFi.dnsIP().toString());
        ShellOutput::print("  MAC:        ");
        ShellOutput::println(WiFi.macAddress());
        ShellOutput::print("  RSSI:       ");
        ShellOutput::print((int)WiFi.RSSI());
        ShellOutput::println(" dBm");
        ShellOutput::print("  Channel:    ");
        ShellOutput::println(WiFi.channel());
    } else {
        ShellOutput::println("  Status:     DISCONNECTED");
        ShellOutput::print("  MAC:        ");
        ShellOutput::println(WiFi.macAddress());
    }
    
    ShellOutput::println();
    
    return SHELL_OK;
}

// Comando: ipset - Configurar IP estática
// Uso: ipset <ip> <netmask> <gateway>
// Ejemplo: ipset 192.168.1.100 255.255.255.0 192.168.1.1
ShellError MiniShell::cmd_ipset(CommandArgs args) {
    IPAddress ip, netmask, gateway;
    
    if(!ip.fromString(args.argv[1])) {
        ShellOutput::println("ERROR: Invalid IP address");
        return SHELL_ERR_INVALID_ARGS;
    }
    
    if(!netmask.fromString(args.argv[2])) {
        ShellOutput::println("ERROR: Invalid netmask");
        return SHELL_ERR_INVALID_ARGS;
    }
    
    if(!gateway.fromString(args.argv[3])) {
        ShellOutput::println("ERROR: Invalid gateway");
        return SHELL_ERR_INVALID_ARGS;
    }
    
    if(!WiFi.config(ip, gateway, netmask)) {
        ShellOutput::println("ERROR: Cannot set static IP");
        return SHELL_ERR_PERMISSION;
    }
    
    ShellOutput::println("Static IP configured successfully");
    ShellOutput::print("IP:      ");
    ShellOutput::println(ip.toString());
    ShellOutput::print("Netmask: ");
    ShellOutput::println(netmask.toString());
    ShellOutput::print("Gateway: ");
    ShellOutput::println(gateway.toString());
    
    // NUEVO: Guardar IP estática en SD
    if(NetworkConfigManager::saveStaticIP(args.argv[1], args.argv[2], args.argv[3])) {
        ShellOutput::println("Static IP configuration saved to SD card");
        ShellOutput::println("Will be applied on next boot");
    } else {
        ShellOutput::println("WARNING: Could not save static IP config");
    }
    
    return SHELL_OK;
}

// Comando: ping - Hacer ping ICMP a un host
// Uso: ping <host> [count]
// Ejemplo: ping 8.8.8.8 o ping 192.168.1.100 5
ShellError MiniShell::cmd_ping(CommandArgs args) {
    String host = args.argv[1];
    int count = args.argc > 2 ? atoi(args.argv[2]) : 4;
    
    if(count < 1 || count > 20) {
        ShellOutput::println("ERROR: Count must be between 1 and 20");
        return SHELL_ERR_INVALID_ARGS;
    }
    
    if(WiFi.status() != WL_CONNECTED) {
        ShellOutput::println("ERROR: WiFi not connected");
        return SHELL_ERR_PERMISSION;
    }
    
    IPAddress targetIP;
    
    // Resolver hostname a IP
    if(!WiFi.hostByName(host.c_str(), targetIP)) {
        // Si no puede resolver, intentar parsear como IP directa
        if(!targetIP.fromString(host)) {
            ShellOutput::println("ERROR: Cannot resolve hostname");
            return SHELL_ERR_NOT_FOUND;
        }
    }
    
    ShellOutput::printf("PING %s (%s) - %d packets\n", 
                  host.c_str(), targetIP.toString().c_str(), count);
    ShellOutput::println("-------------------------------");
    
    int success = 0;
    int failed = 0;
    unsigned long totalTime = 0;
    unsigned long minTime = 999999;
    unsigned long maxTime = 0;
    
    for(int i = 0; i < count; i++) {
        // Usar ping de la librería ESP32
        int result = Ping.ping(targetIP, 1);
        
        if(result > 0) {
            unsigned long avgTime = Ping.averageTime();
            totalTime += avgTime;
            success++;
            
            if(avgTime < minTime) minTime = avgTime;
            if(avgTime > maxTime) maxTime = avgTime;
            
            ShellOutput::printf("Reply from %s: time=%lums TTL=64\n", 
                         targetIP.toString().c_str(), avgTime);
        } else {
            failed++;
            ShellOutput::printf("Request timeout for %s\n", targetIP.toString().c_str());
        }
        
        if(i < count - 1) delay(1000);
    }
    
    ShellOutput::println();
    ShellOutput::printf("--- %s ping statistics ---\n", targetIP.toString().c_str());
    ShellOutput::printf("%d packets transmitted, %d received, %d%% packet loss\n", 
                  count, success, (failed * 100) / count);
    
    if(success > 0) {
        ShellOutput::printf("rtt min/avg/max = %lu/%lu/%lu ms\n", 
                     minTime, totalTime / success, maxTime);
    }
    
    return SHELL_OK;
}

// Comando: wifiscan - Escanear redes WiFi disponibles
ShellError MiniShell::cmd_wifiscan(CommandArgs args) {
    ShellOutput::println("Scanning WiFi networks...");
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    int n = WiFi.scanNetworks();
    
    if(n == 0) {
        ShellOutput::println("No networks found");
        return SHELL_OK;
    }
    
    ShellOutput::println("\n=== Available WiFi Networks ===\n");
    ShellOutput::println("SSID                             | RSSI | Channel | Encryption");
    ShellOutput::println("----------------------------------------------------------------");
    
    for(int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        if(ssid.length() > 32) ssid = ssid.substring(0, 29) + "...";
        
        ShellOutput::printf("%-32s | %4d | %7d | ", 
                     ssid.c_str(), 
                     WiFi.RSSI(i), 
                     WiFi.channel(i));
        
        switch(WiFi.encryptionType(i)) {
            case WIFI_AUTH_OPEN:
                ShellOutput::println("Open");
                break;
            case WIFI_AUTH_WEP:
                ShellOutput::println("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                ShellOutput::println("WPA-PSK");
                break;
            case WIFI_AUTH_WPA2_PSK:
                ShellOutput::println("WPA2-PSK");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                ShellOutput::println("WPA/WPA2-PSK");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                ShellOutput::println("WPA2-Enterprise");
                break;
            default:
                ShellOutput::println("Unknown");
        }
    }
    
    ShellOutput::printf("\nTotal: %d networks found\n", n);
    WiFi.scanDelete();
    
    return SHELL_OK;
}

// Comando: wificonnect - Conectar a red WiFi
// Uso: wificonnect <ssid> <password>
// Ejemplo: wificonnect "MyWiFi" "mypassword123"
ShellError MiniShell::cmd_wificonnect(CommandArgs args) {
    String ssid = args.argv[1];
    String password = args.argv[2];
    
    ShellOutput::printf("Connecting to '%s'...\n", ssid.c_str());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    while(WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        ShellOutput::print(".");
        attempts++;
    }
    ShellOutput::println();
    
    if(WiFi.status() == WL_CONNECTED) {
        ShellOutput::println("Connected successfully!");
        ShellOutput::print("IP Address: ");
        ShellOutput::println(WiFi.localIP().toString());
        ShellOutput::print("Gateway:    ");
        ShellOutput::println(WiFi.gatewayIP().toString());
        ShellOutput::print("RSSI:       ");
        ShellOutput::print((int)WiFi.RSSI());
        ShellOutput::println(" dBm");
        
        // NUEVO: Guardar credenciales en SD
        if(NetworkConfigManager::saveWiFiCredentials(ssid.c_str(), password.c_str())) {
            ShellOutput::println("WiFi credentials saved to SD card");
        } else {
            ShellOutput::println("WARNING: Could not save credentials");
        }
        
        return SHELL_OK;
    } else {
        ShellOutput::println("ERROR: Connection failed");
        ShellOutput::println("Check SSID and password");
        return SHELL_ERR_PERMISSION;
    }
}

// Comando: wifidisconnect - Desconectar de red WiFi
ShellError MiniShell::cmd_wifidisconnect(CommandArgs args) {
    if(WiFi.status() != WL_CONNECTED) {
        ShellOutput::println("WiFi already disconnected");
        return SHELL_OK;
    }
    
    ShellOutput::println("Disconnecting from WiFi...");
    WiFi.disconnect(true);
    delay(100);
    
    ShellOutput::println("WiFi disconnected");
    return SHELL_OK;
}