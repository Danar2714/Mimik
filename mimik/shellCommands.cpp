/*
 * mimik - Mini Shell para ESP32-CAM
 * shellCommands.cpp - Implementación de comandos
 */

#include "shell.h"
#include "sshServer.h"

// Comando: pwd
ShellError MiniShell::cmd_pwd(CommandArgs args) {
    ShellOutput::println(shell.getCurrentPath());
    return SHELL_OK;
}

// Comando: ls
ShellError MiniShell::cmd_ls(CommandArgs args) {
    String path = args.argc > 1 ? shell.resolvePath(args.argv[1]) : shell.getCurrentPath();
    
    File dir = SD_MMC.open(path);
    if(!dir) {
        ShellOutput::println("ERROR: Cannot open the directory");
        return SHELL_ERR_NOT_FOUND;
    }
    
    if(!dir.isDirectory()) {
        ShellOutput::println("ERROR: Is not a directory");
        dir.close();
        return SHELL_ERR_INVALID_PATH;
    }
    
    File file = dir.openNextFile();
    while(file) {
        String fileName = String(file.name());
        
        // Ignorar carpetas del sistema de Windows
        if(fileName == "System Volume Information" || 
           fileName == "$RECYCLE.BIN" ||
           fileName == "RECYCLER" ||
           fileName.startsWith("._")) {
            file = dir.openNextFile();
            continue;
        }
        
        ShellOutput::print(file.isDirectory() ? "d " : "- ");
        ShellOutput::print(fileName);
        if(!file.isDirectory()) {
            ShellOutput::print("\t");
            ShellOutput::print((int)file.size());
            ShellOutput::print(" bytes");
        }
        ShellOutput::println();
        file = dir.openNextFile();
    }
    
    dir.close();
    return SHELL_OK;
}

// Comando: cd
ShellError MiniShell::cmd_cd(CommandArgs args) {
    String newPath;
    
    // Soporte para cd ..
    if(strcmp(args.argv[1], "..") == 0) {
        String currentPath = String(shell.getCurrentPath());
        
        // Si estamos en root, no hacer nada
        if(currentPath == "/") {
            return SHELL_OK;
        }
        
        // Encontrar el último '/' y obtener el directorio padre
        int lastSlash = currentPath.lastIndexOf('/', currentPath.length() - 2);
        if(lastSlash <= 0) {
            newPath = "/";
        } else {
            newPath = currentPath.substring(0, lastSlash);
        }
    } else {
        newPath = shell.resolvePath(args.argv[1]);
    }
    
    if(!SD_MMC.exists(newPath)) {
        ShellOutput::println("ERROR: Directory does not exist");
        return SHELL_ERR_NOT_FOUND;
    }
    
    File dir = SD_MMC.open(newPath);
    if(!dir.isDirectory()) {
        ShellOutput::println("ERROR: Is not a directory");
        dir.close();
        return SHELL_ERR_INVALID_PATH;
    }
    dir.close();
    
    shell.setCurrentPath(newPath.c_str());
    return SHELL_OK;
}

// Comando: mkdir
ShellError MiniShell::cmd_mkdir(CommandArgs args) {
    String path = shell.resolvePath(args.argv[1]);
    
    if(SD_MMC.exists(path)) {
        ShellOutput::println("ERROR: Directory already exists");
        return SHELL_ERR_FILE_EXISTS;
    }
    
    if(SD_MMC.mkdir(path)) {
        ShellOutput::println("Directory created");
        return SHELL_OK;
    }
    
    ShellOutput::println("ERROR: Cannot create the directory");
    return SHELL_ERR_PERMISSION;
}

// Comando: touch
ShellError MiniShell::cmd_touch(CommandArgs args) {
    String path = shell.resolvePath(args.argv[1]);
    
    if(SD_MMC.exists(path)) {
        ShellOutput::println("File already exists");
        return SHELL_OK;
    }
    
    File file = SD_MMC.open(path, FILE_WRITE);
    if(!file) {
        ShellOutput::println("ERROR: Cannot create the file");
        return SHELL_ERR_PERMISSION;
    }
    
    file.close();
    ShellOutput::println("File created");
    return SHELL_OK;
}

// Comando: rm
ShellError MiniShell::cmd_rm(CommandArgs args) {
    String path = shell.resolvePath(args.argv[1]);
    
    if(!SD_MMC.exists(path)) {
        ShellOutput::println("ERROR: File or directory does not exist");
        return SHELL_ERR_NOT_FOUND;
    }
    
    File file = SD_MMC.open(path);
    bool isDir = file.isDirectory();
    file.close();
    
    if(isDir) {
        if(SD_MMC.rmdir(path)) {
            ShellOutput::println("Directory deleted");
            return SHELL_OK;
        }
        ShellOutput::println("ERROR: Cannot delete directory (is it empty?)");
    } else {
        if(SD_MMC.remove(path)) {
            ShellOutput::println("File deleted");
            return SHELL_OK;
        }
        ShellOutput::println("ERROR: Cannot delete file");
    }
    
    return SHELL_ERR_PERMISSION;
}

// Comando: mv
ShellError MiniShell::cmd_mv(CommandArgs args) {
    String srcPath = shell.resolvePath(args.argv[1]);
    String dstPath = shell.resolvePath(args.argv[2]);
    
    if(!SD_MMC.exists(srcPath)) {
        ShellOutput::println("ERROR: Source does not exist");
        return SHELL_ERR_NOT_FOUND;
    }
    
    if(SD_MMC.exists(dstPath)) {
        ShellOutput::println("ERROR: Destination already exists");
        return SHELL_ERR_FILE_EXISTS;
    }
    
    if(SD_MMC.rename(srcPath, dstPath)) {
        ShellOutput::println("Moved/renamed successfully");
        return SHELL_OK;
    }
    
    ShellOutput::println("ERROR: Cannot move/rename");
    return SHELL_ERR_PERMISSION;
}

// Comando: cp
ShellError MiniShell::cmd_cp(CommandArgs args) {
    String srcPath = shell.resolvePath(args.argv[1]);
    String dstPath = shell.resolvePath(args.argv[2]);
    
    if(!SD_MMC.exists(srcPath)) {
        ShellOutput::println("ERROR: Source does not exist");
        return SHELL_ERR_NOT_FOUND;
    }
    
    File srcFile = SD_MMC.open(srcPath, FILE_READ);
    if(!srcFile) {
        ShellOutput::println("ERROR: Cannot open source");
        return SHELL_ERR_PERMISSION;
    }
    
    if(srcFile.isDirectory()) {
        srcFile.close();
        ShellOutput::println("ERROR: Cannot copy directories");
        return SHELL_ERR_INVALID_ARGS;
    }
    
    File dstFile = SD_MMC.open(dstPath, FILE_WRITE);
    if(!dstFile) {
        srcFile.close();
        ShellOutput::println("ERROR: Cannot create destination");
        return SHELL_ERR_PERMISSION;
    }
    
    uint8_t buffer[512];
    while(srcFile.available()) {
        size_t len = srcFile.read(buffer, sizeof(buffer));
        dstFile.write(buffer, len);
    }
    
    srcFile.close();
    dstFile.close();
    
    ShellOutput::println("File copied");
    return SHELL_OK;
}

// Comando: cat
ShellError MiniShell::cmd_cat(CommandArgs args) {
    String path = shell.resolvePath(args.argv[1]);
    
    File file = SD_MMC.open(path, FILE_READ);
    if(!file) {
        ShellOutput::println("ERROR: Cannot open file");
        return SHELL_ERR_NOT_FOUND;
    }
    
    if(file.isDirectory()) {
        file.close();
        ShellOutput::println("ERROR: Is a directory");
        return SHELL_ERR_INVALID_PATH;
    }
    
    while(file.available()) {
        ShellOutput::write(file.read());
    }
    ShellOutput::println();
    
    file.close();
    return SHELL_OK;
}

// Comando: nano (editor simple)
ShellError MiniShell::cmd_nano(CommandArgs args) {
    String path = shell.resolvePath(args.argv[1]);
    
    ShellOutput::println("=== Simple Nano Editor ===");
    ShellOutput::println("Write content. End with 'EOF' on a new line");
    ShellOutput::println();
    
    File file = SD_MMC.open(path, FILE_WRITE);
    if(!file) {
        ShellOutput::println("ERROR: Cannot open file for writing");
        return SHELL_ERR_PERMISSION;
    }
    
    String line = "";
    unsigned long lastActivity = millis();
    
    while(true) {
        while(Serial.available()) {
            char c = Serial.read();
            lastActivity = millis();
            
            if(c == '\n' || c == '\r') {
                ShellOutput::println();
                if(line == "EOF") {
                    file.close();
                    ShellOutput::println("\nFile saved");
                    return SHELL_OK;
                }
                file.println(line);
                line = "";
            } else if(c == 127 || c == 8) {
                if(line.length() > 0) {
                    line.remove(line.length() - 1);
                    ShellOutput::write(8);
                    ShellOutput::write(' ');
                    ShellOutput::write(8);
                }
            } else {
                line += c;
                ShellOutput::write(c);
            }
        }
        
        // Timeout de 60 segundos sin actividad
        if(millis() - lastActivity > 60000) {
            file.close();
            ShellOutput::println("\nTimeout - file saved");
            return SHELL_OK;
        }
        
        delay(10);
    }
}

// Comando: help
ShellError MiniShell::cmd_help(CommandArgs args) {
    ShellOutput::println("\n=== mimik - Available Commands ===");
    for(int i = 0; i < shell.commandCount; i++) {
        ShellOutput::print(shell.commands[i].name);
        ShellOutput::print("\t- ");
        ShellOutput::println(shell.commands[i].description);
    }
    ShellOutput::println();
    return SHELL_OK;
}