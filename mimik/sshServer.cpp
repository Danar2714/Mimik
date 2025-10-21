/*
 * mimik - Mini Shell para ESP32-CAM
 * sshServer.cpp - Servidor Telnet
 */

#include "sshServer.h"
#include <stdarg.h>

SSHServer sshServer;

// Inicializar variables estáticas de ShellOutput
ShellOutput::OutputMode ShellOutput::currentMode = ShellOutput::MODE_SERIAL;
SSHServer* ShellOutput::sshServer = nullptr;

SSHServer::SSHServer() {
    server = nullptr;
    clientConnected = false;
    cmdIndex = 0;
    memset(cmdBuffer, 0, MAX_CMD_LENGTH);
}

SSHServer::~SSHServer() {
    stop();
}

bool SSHServer::begin() {
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("ERROR: WiFi not connected. Cannot start Telnet server.");
        return false;
    }
    
    //Serial.println("Starting Telnet server...");
    
    // Crear servidor TCP
    server = new WiFiServer(TELNET_PORT);
    if(!server) {
        Serial.println("ERROR: Cannot create WiFi server");
        return false;
    }
    
    server->begin();
    server->setNoDelay(true);  // Desactivar algoritmo Nagle para respuesta rápida
    
    Serial.printf("Telnet server listening on %s:%d\n", 
                  WiFi.localIP().toString().c_str(), TELNET_PORT);
    //Serial.println("Connect with: telnet " + WiFi.localIP().toString());
    
    return true;
}

void SSHServer::loop() {
    if(!server) return;
    
    // Aceptar nuevos clientes
    if(!clientConnected) {
        if(server->hasClient()) {
            // Desconectar cliente anterior si existe
            if(client && client.connected()) {
                client.stop();
            }
            
            client = server->available();
            if(client && client.connected()) {
                Serial.println("Telnet: Client connected from " + client.remoteIP().toString());
                clientConnected = true;
                cmdIndex = 0;
                memset(cmdBuffer, 0, MAX_CMD_LENGTH);
                
                // Cambiar modo de salida
                ShellOutput::setMode(ShellOutput::MODE_SSH, this);
                
                // Enviar banner de bienvenida
                sendString("\r\n");
                sendString("===========================================\r\n");
                sendString("  Welcome to mimik Telnet Shell\r\n");
                sendString("  ESP32-CAM Remote Access\r\n");
                sendString("===========================================\r\n");
                sendString("\r\n");
                sendPrompt();
            }
        }
        return;
    }
    
    // Manejar cliente conectado
    if(client && client.connected()) {
        handleClient();
    } else {
        // Cliente desconectado
        Serial.println("Telnet: Client disconnected");
        stop();
    }
}

void SSHServer::handleClient() {
    while(client.available()) {
        char c = client.read();
        
        // Filtrar caracteres de control Telnet (IAC = 255)
        if((uint8_t)c == 255) {
            // Ignorar secuencias de negociación Telnet
            if(client.available()) client.read();
            if(client.available()) client.read();
            continue;
        }
        
        // Procesar entrada
        if(c == '\r' || c == '\n') {
            if(cmdIndex > 0) {
                sendString("\r\n");
                cmdBuffer[cmdIndex] = '\0';
                processCommand(cmdBuffer);
                cmdIndex = 0;
                memset(cmdBuffer, 0, MAX_CMD_LENGTH);
                sendPrompt();
            }
        } else if(c == 127 || c == 8) { // Backspace
            if(cmdIndex > 0) {
                cmdIndex--;
                sendString("\b \b");
            }
        } else if(c == 3) { // Ctrl+C
            sendString("^C\r\n");
            cmdIndex = 0;
            memset(cmdBuffer, 0, MAX_CMD_LENGTH);
            sendPrompt();
        } else if(c == 4) { // Ctrl+D (EOF)
            sendString("\r\nLogout\r\n");
            stop();
            return;
        } else if(c >= 32 && c < 127 && cmdIndex < MAX_CMD_LENGTH - 1) {
            // Solo caracteres imprimibles
            cmdBuffer[cmdIndex++] = c;
            // NO hacer echo - el cliente ya lo muestra
        }
    }
}

void SSHServer::processCommand(const char* cmdLine) {
    CommandArgs args;
    char lineCopy[MAX_CMD_LENGTH];
    strncpy(lineCopy, cmdLine, MAX_CMD_LENGTH - 1);
    lineCopy[MAX_CMD_LENGTH - 1] = '\0';
    
    // Parsear comando
    args.argc = 0;
    char* token = strtok(lineCopy, " \t\n\r");
    
    while(token != NULL && args.argc < MAX_ARGS) {
        args.argv[args.argc++] = token;
        token = strtok(NULL, " \t\n\r");
    }
    
    if(args.argc > 0) {
        // Buscar y ejecutar comando
        Command* cmd = shell.findCommand(args.argv[0]);
        
        if(cmd != NULL) {
            int argCount = args.argc - 1;
            if(argCount < cmd->minArgs) {
                ShellOutput::printf("ERROR: %s requires at least %d argument(s)\r\n", 
                                   cmd->name, cmd->minArgs);
            } else if(argCount > cmd->maxArgs) {
                ShellOutput::printf("ERROR: %s accepts maximum %d argument(s)\r\n", 
                                   cmd->name, cmd->maxArgs);
            } else {
                ShellError err = cmd->function(args);
                if(err != SHELL_OK) {
                    ShellOutput::printf("Error executing command (code: %d)\r\n", err);
                }
            }
        } else {
            ShellOutput::printf("Command not found: %s\r\n", args.argv[0]);
            ShellOutput::println("Type 'help' to see available commands");
        }
    }
}

void SSHServer::sendPrompt() {
    char prompt[MAX_PATH_LENGTH + 20];
    snprintf(prompt, sizeof(prompt), "mimik:%s$ ", shell.getCurrentPath());
    sendString(prompt);
}

void SSHServer::sendString(const char* str) {
    if(client && clientConnected && client.connected()) {
        client.print(str);
    }
}

void SSHServer::stop() {
    ShellOutput::setMode(ShellOutput::MODE_SERIAL);
    
    if(client && client.connected()) {
        client.stop();
    }
    
    clientConnected = false;
    cmdIndex = 0;
}

// ============================================
// Implementación de ShellOutput
// ============================================

void ShellOutput::setMode(OutputMode mode, SSHServer* server) {
    currentMode = mode;
    sshServer = server;
}

void ShellOutput::print(const char* str) {
    if(currentMode == MODE_SERIAL || !sshServer) {
        Serial.print(str);
    } else {
        sshServer->sendString(str);
    }
}

void ShellOutput::print(const String& str) {
    print(str.c_str());
}

void ShellOutput::print(int num) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", num);
    print(buf);
}

void ShellOutput::print(unsigned long num) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lu", num);
    print(buf);
}

void ShellOutput::print(char c) {
    char buf[2] = {c, '\0'};
    print(buf);
}

void ShellOutput::println(const char* str) {
    print(str);
    if(currentMode == MODE_SSH) {
        print("\r\n");
    } else {
        Serial.println();
    }
}

void ShellOutput::println(const String& str) {
    println(str.c_str());
}

void ShellOutput::println(int num) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", num);
    println(buf);
}

void ShellOutput::println() {
    if(currentMode == MODE_SSH) {
        print("\r\n");
    } else {
        Serial.println();
    }
}

void ShellOutput::printf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    print(buf);
}

void ShellOutput::write(uint8_t c) {
    print((char)c);
}