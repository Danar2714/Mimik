/*
 * mimik - Mini Shell para ESP32-CAM
 * sshServer.h - Servidor Telnet (renombrado de SSH por compatibilidad)
 */

#ifndef SSH_SERVER_H
#define SSH_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include "shell.h"

// Puerto Telnet (23 es estándar, pero puedes usar 22 también)
#define TELNET_PORT 23
#define MAX_CLIENTS 1

// Forward declaration
class ShellOutput;

class SSHServer {  // Mantenemos el nombre para compatibilidad con código existente
private:
    WiFiServer* server;
    WiFiClient client;
    bool clientConnected;
    
    char cmdBuffer[MAX_CMD_LENGTH];
    int cmdIndex;
    
    // Métodos privados
    void handleClient();
    void processCommand(const char* cmdLine);
    void sendPrompt();
    void sendString(const char* str);
    
public:
    SSHServer();
    ~SSHServer();
    
    bool begin();
    void loop();
    void stop();
    
    bool isConnected() { return clientConnected; }
    
    friend class ShellOutput;
};

// Clase para abstraer la salida (Serial o Telnet)
class ShellOutput {
public:
    enum OutputMode {
        MODE_SERIAL,
        MODE_SSH  // Mantenemos MODE_SSH por compatibilidad, pero es Telnet
    };
    
private:
    static OutputMode currentMode;
    static SSHServer* sshServer;
    
public:
    static void setMode(OutputMode mode, SSHServer* server = nullptr);
    static OutputMode getMode() { return currentMode; }
    
    static void print(const char* str);
    static void print(const String& str);
    static void print(int num);
    static void print(unsigned long num);
    static void print(char c);
    
    static void println(const char* str);
    static void println(const String& str);
    static void println(int num);
    static void println();
    
    static void printf(const char* format, ...);
    static void write(uint8_t c);
};

extern SSHServer sshServer;

#endif