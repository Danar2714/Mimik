/*
 * mimik - Mini Shell para ESP32-CAM
 * shell.h - Definiciones principales
 */

#ifndef SHELL_H
#define SHELL_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPping.h>

// Configuración de pines para AI-Thinker ESP32-CAM
#define SD_MMC_CMD  15
#define SD_MMC_CLK  14
#define SD_MMC_D0   2

// Tamaños de buffer
#define MAX_CMD_LENGTH 256
#define MAX_PATH_LENGTH 128
#define MAX_ARGS 10

// Códigos de error
enum ShellError {
    SHELL_OK = 0,
    SHELL_ERR_NOT_FOUND,
    SHELL_ERR_INVALID_PATH,
    SHELL_ERR_FILE_EXISTS,
    SHELL_ERR_PERMISSION,
    SHELL_ERR_NO_SPACE,
    SHELL_ERR_INVALID_ARGS
};

// Estructura para argumentos de comandos
struct CommandArgs {
    int argc;
    char* argv[MAX_ARGS];
};

// Tipo de función para comandos
typedef ShellError (*CommandFunction)(CommandArgs args);

// Estructura de comando
struct Command {
    const char* name;
    const char* description;
    CommandFunction function;
    int minArgs;
    int maxArgs;
};

// Forward declaration de ShellOutput
class ShellOutput;

// Clase principal del Shell
class MiniShell {
private:
    char currentPath[MAX_PATH_LENGTH];
    char cmdBuffer[MAX_CMD_LENGTH];
    int cmdIndex;
    
    // Comandos registrados
    static const int MAX_COMMANDS = 30;
    Command commands[MAX_COMMANDS];
    int commandCount;
    
    // Métodos privados
    void parseLine(char* line, CommandArgs* args);
    String resolvePath(const char* path);
    bool isAbsolutePath(const char* path);
    
public:
    MiniShell();
    bool init();
    void registerCommand(const char* name, const char* description, 
                        CommandFunction func, int minArgs, int maxArgs);
    void processInput();
    void printPrompt();
    const char* getCurrentPath() { return currentPath; }
    void setCurrentPath(const char* path);
    
    // Métodos públicos para SSH
    Command* findCommand(const char* name);
    
    // Comandos integrados - Sistema de archivos
    static ShellError cmd_ls(CommandArgs args);
    static ShellError cmd_cd(CommandArgs args);
    static ShellError cmd_pwd(CommandArgs args);
    static ShellError cmd_mkdir(CommandArgs args);
    static ShellError cmd_touch(CommandArgs args);
    static ShellError cmd_rm(CommandArgs args);
    static ShellError cmd_mv(CommandArgs args);
    static ShellError cmd_cp(CommandArgs args);
    static ShellError cmd_nano(CommandArgs args);
    static ShellError cmd_cat(CommandArgs args);
    static ShellError cmd_help(CommandArgs args);
    
    // Comandos de networking
    static ShellError cmd_ifconfig(CommandArgs args);
    static ShellError cmd_ipset(CommandArgs args);
    static ShellError cmd_ping(CommandArgs args);
    static ShellError cmd_wifiscan(CommandArgs args);
    static ShellError cmd_wificonnect(CommandArgs args);
    static ShellError cmd_wifidisconnect(CommandArgs args);
    static ShellError cmd_netconfig(CommandArgs args);
    static ShellError cmd_netclear(CommandArgs args);
    
    // Comandos de monitoreo
    static ShellError cmd_top(CommandArgs args);
    
    // Permitir acceso desde funciones globales y SSH
    friend bool initShellTasks();
    friend class SSHServer;
    friend class ShellOutput;
};

// Instancia global
extern MiniShell shell;

#endif