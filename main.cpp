#include "FileSystem.h"

//Assume the cstring size is 1 more than the size of string parameter for null terminator
void convertString(string str, char cstring[]){
    int i;
    for(i = 0; i < str.length(); i++){
        cstring[i] = str.at(i);
    }
    cstring[i] = 0;
}


int main()
{
    FileSystem fileSystem;
    string line;
    char cline[100];
    char buffer[193];
    char* command;
    char* parameter;
    char symbolic_file_name[4];
    char* inputChar;
    int count;
    int OFT_index;
    int bytesRead = 0;
    int bytesWrite = 0;
    int pos;
    int status;
    bool disk_initialized = false;

    do{
        getline(cin, line);
        strcpy(cline, line.c_str());
        command = strtok(cline, " \t\n");
        parameter = strtok(NULL, " \t\n");
        if (strcmp(command, "in") == 0) {
            fileSystem.init(parameter);
            if (parameter != nullptr) {
                cout << "disk initialized\n";
            } else
                cout << "disk restored\n";
            disk_initialized = true;
            //status = fileSystem.init(parameter);
        } else if (strcmp(command, "cr") == 0 && disk_initialized) {
            if (strlen(parameter) > 3)
                cout << "Error create! File name length is greater than 3\n";
            else if (strlen(parameter) < 1)
                cout << "Error create! File name is too short\n";
            else {
                cout << parameter << " created\n";
                convertString(parameter, symbolic_file_name);
                fileSystem.create(symbolic_file_name);
            }
        } else if (strcmp(command, "de") == 0 && disk_initialized) {
            if (strlen(parameter) > 3)
                cout << "Error delete! File name length is greater than 3\n";
            else if (strlen(parameter) < 1)
                cout << "Error delete! File name is too short\n";
            else {
                convertString(parameter, symbolic_file_name);
                status = fileSystem.destroy(symbolic_file_name);
                if(status != -1)
                    cout << parameter << " destroyed\n";
            }
        } else if (strcmp(command, "op") == 0 && disk_initialized) {
            if (strlen(parameter) > 3)
                cout << "Error open! File name length is greater than 3\n";
            else if (strlen(parameter) < 1)
                cout << "Error open! File name is too short\n";
            else {
                convertString(parameter, symbolic_file_name);
                OFT_index = fileSystem.open(symbolic_file_name);
                if(OFT_index != -1)
                    cout << parameter << " opened " << OFT_index << endl;
            }
        } else if (strcmp(command, "cl") == 0 && disk_initialized) {
            OFT_index = stoi(parameter, nullptr, 10);
            if (OFT_index < 1 || OFT_index > 4) {
                cout << "Error close! Invalid value for OFT index\n";
            } else {
                status = fileSystem.close(OFT_index);
                if(status != -1)
                    cout << OFT_index << " closed\n";
            }
        } else if (strcmp(command, "rd") == 0 && disk_initialized) {
            //parameter would be index
            OFT_index = stoi(parameter, nullptr, 10);
            count = stoi((strtok(NULL, " \t\n")), nullptr, 10);
            if (OFT_index < 1 || OFT_index > 4) {
                cout << "Error read! Invalid value for OFT index\n";
            } else {
                bytesRead = fileSystem.read(OFT_index, buffer, count);
                buffer[bytesRead] = 0;
                if(bytesRead != -1){
                    cout << "Bytes read: " << bytesRead << endl;
                    cout << buffer << endl;
                }
            }
        } else if (strcmp(command, "wr") == 0 && disk_initialized) {
            OFT_index = stoi(parameter, nullptr, 10);
            inputChar = strtok(NULL, " \t\n");
            count = stoi((strtok(NULL, " \t\n")), nullptr, 10);
            if (OFT_index < 1 || OFT_index > 4) {
                cout << "Error write! Invalid value for OFT index\n";
            } else {
                bytesWrite = fileSystem.write(OFT_index, inputChar, count);
                if(bytesWrite != -1){
                    cout << "Bytes write: " << bytesWrite << endl;
                    cout << buffer << endl;
                }
            }
        } else if (strcmp(command, "sk") == 0 && disk_initialized) {
            OFT_index = stoi(parameter, nullptr, 10);
            pos = stoi((strtok(NULL, " \t\n")), nullptr, 10);
            if (OFT_index < 1 || OFT_index > 4) {
                cout << "Error seek! Invalid value for OFT index\n";
            } else {
                status = fileSystem.lseek(OFT_index, pos);
                if(status != -1){
                    cout << "position is " << pos << endl;
                }
            }
        } else if (strcmp(command, "dr") == 0 && disk_initialized) {
            cout << fileSystem.directory() << endl;

        } else if (strcmp(command, "sv") == 0 && disk_initialized) {
            fileSystem.save(parameter);
            cout << "disk saved\n";
        } else if(!disk_initialized)
            cout << "Disk is not initialized\n";
        else if(strcmp(command, "exit") != 0){
            cout << "Invalid command! Please input a new command: \n";
        }
    }while(strcmp(command, "exit") != 0);
        return 0;
}

