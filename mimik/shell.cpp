/*
 * mimik - Mini Shell para ESP32-CAM
 * shell.cpp - Implementación core
 */

#include "shell.h"
#include "networkConfig.h"

MiniShell shell;

MiniShell::MiniShell() {
    strcpy(currentPath, "/");
    cmdIndex = 0;
    commandCount = 0;
    memset(cmdBuffer, 0, MAX_CMD_LENGTH);
}

bool MiniShell::init() {
    Serial.println("Initializing mimik Shell...");
    
    // Inicializar SD Card en modo 1-bit
    if(!SD_MMC.begin("/sdcard", true)) {
        Serial.println("ERROR: SD card not mounted");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE) {
        Serial.println("ERROR: No SD card inserted");
        return false;
    }
    
    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC) Serial.println("MMC");
    else if(cardType == CARD_SD) Serial.println("SD");
    else if(cardType == CARD_SDHC) Serial.println("SDHC");
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("Size: %lluMB\n", cardSize);
    
    // Crear directorio root si no existe
    if(!SD_MMC.exists("/")) {
        Serial.println("Creating root directory...");
        SD_MMC.mkdir("/");
    }
    
    Serial.println("Checking for saved WiFi configuration...");
    NetworkConfigManager::autoConnect();

    // Registrar comandos del sistema de archivos
    registerCommand("ls", "List files", cmd_ls, 0, 1);
    registerCommand("cd", "Change directory", cmd_cd, 1, 1);
    registerCommand("pwd", "Print working directory", cmd_pwd, 0, 0);
    registerCommand("mkdir", "Create directory", cmd_mkdir, 1, 1);
    registerCommand("touch", "Create file", cmd_touch, 1, 1);
    registerCommand("rm", "Remove file/directory", cmd_rm, 1, 1);
    registerCommand("mv", "Move/rename file", cmd_mv, 2, 2);
    registerCommand("cp", "Copy file", cmd_cp, 2, 2);
    registerCommand("nano", "Edit file", cmd_nano, 1, 1);
    registerCommand("cat", "Display file contents", cmd_cat, 1, 1);
    registerCommand("help", "Show help", cmd_help, 0, 0);
    
    // Registrar comandos de networking
    registerCommand("ifconfig", "Network interface info", cmd_ifconfig, 0, 0);
    registerCommand("ipset", "Set static IP", cmd_ipset, 3, 3);
    registerCommand("ping", "Ping host", cmd_ping, 1, 2);
    registerCommand("wifiscan", "Scan WiFi networks", cmd_wifiscan, 0, 0);
    registerCommand("wificonnect", "Connect to WiFi", cmd_wificonnect, 2, 2);
    registerCommand("wifidisconnect", "Disconnect WiFi", cmd_wifidisconnect, 0, 0);
    registerCommand("netconfig", "Show network config", cmd_netconfig, 0, 0);
    registerCommand("netclear", "Clear network config", cmd_netclear, 0, 0);
    
    // Registrar comandos de monitoreo
    registerCommand("top", "System resources", cmd_top, 0, 0);
    
    Serial.println("mimik Shell initialized successfully");
    return true;
}

void MiniShell::registerCommand(const char* name, const char* description, 
                                CommandFunction func, int minArgs, int maxArgs) {
    if(commandCount >= MAX_COMMANDS) {
        Serial.println("ERROR: Command limit reached");
        return;
    }
    
    commands[commandCount].name = name;
    commands[commandCount].description = description;
    commands[commandCount].function = func;
    commands[commandCount].minArgs = minArgs;
    commands[commandCount].maxArgs = maxArgs;
    commandCount++;
}

void MiniShell::parseLine(char* line, CommandArgs* args) {
    args->argc = 0;
    char* token = strtok(line, " \t\n\r");
    
    while(token != NULL && args->argc < MAX_ARGS) {
        args->argv[args->argc++] = token;
        token = strtok(NULL, " \t\n\r");
    }
}

Command* MiniShell::findCommand(const char* name) {
    for(int i = 0; i < commandCount; i++) {
        if(strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

String MiniShell::resolvePath(const char* path) {
    if(isAbsolutePath(path)) {
        return String(path);
    }
    
    String fullPath = String(currentPath);
    if(fullPath[fullPath.length()-1] != '/') {
        fullPath += "/";
    }
    fullPath += path;
    
    return fullPath;
}

bool MiniShell::isAbsolutePath(const char* path) {
    return path[0] == '/';
}

void MiniShell::setCurrentPath(const char* path) {
    strncpy(currentPath, path, MAX_PATH_LENGTH - 1);
    currentPath[MAX_PATH_LENGTH - 1] = '\0';
}

void MiniShell::printPrompt() {
    Serial.print("mimik:");
    Serial.print(currentPath);
    Serial.print("$ ");
}

void MiniShell::processInput() {
    while(Serial.available()) {
        char c = Serial.read();
        
        if(c == '\n' || c == '\r') {
            if(cmdIndex > 0) {
                Serial.println();
                cmdBuffer[cmdIndex] = '\0';
                
                CommandArgs args;
                parseLine(cmdBuffer, &args);
                
                if(args.argc > 0) {
                    Command* cmd = findCommand(args.argv[0]);
                    
                    if(cmd != NULL) {
                        int argCount = args.argc - 1;
                        if(argCount < cmd->minArgs) {
                            Serial.printf("ERROR: %s requires at least %d argument(s)\n", 
                                        cmd->name, cmd->minArgs);
                        } else if(argCount > cmd->maxArgs) {
                            Serial.printf("ERROR: %s accepts maximum %d argument(s)\n", 
                                        cmd->name, cmd->maxArgs);
                        } else {
                            ShellError err = cmd->function(args);
                            if(err != SHELL_OK) {
                                Serial.printf("Error executing command (code: %d)\n", err);
                            }
                        }
                    } else {
                        Serial.printf("Command not found: %s\n", args.argv[0]);
                        Serial.println("Type 'help' to see available commands");
                    }
                }
                
                cmdIndex = 0;
                memset(cmdBuffer, 0, MAX_CMD_LENGTH);
                printPrompt();
            }
        } else if(c == 127 || c == 8) {
            if(cmdIndex > 0) {
                cmdIndex--;
                Serial.write(8);
                Serial.write(' ');
                Serial.write(8);
            }
        } else if(cmdIndex < MAX_CMD_LENGTH - 1) {
            cmdBuffer[cmdIndex++] = c;
            Serial.write(c);
        }
    }
}