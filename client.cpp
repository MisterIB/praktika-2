#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

bool exitingClient(const string& command) {
    if (command == "EXIT") {
        cout << "Exit" << endl;
        return true;
    }
    return false;
}

void gettingTable(const string& command, int32_t clientSocket) {
    istringstream commandStream(command);
    string chekCommand;
    commandStream >> chekCommand;
    char buffer[1024] = {0};
    if (chekCommand == "SELECT") {
        int32_t resultingTable = read(clientSocket, buffer, sizeof(buffer));
        if (resultingTable > 0) cout << buffer << endl;
        else throw runtime_error("No response was received from the server");
    }
}

int main() {
    try {
        int32_t clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(7432);
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) throw runtime_error("Error conection");
        cout << "You have connected to the server" << endl;
        while (true) {
            string command;
            cout << "Enter the command" << "\n" << ">";
            getline(cin , command);
            send(clientSocket, command.c_str(), command.length(), 0);
            if (exitingClient(command)) break;
            char buffer[1024] = {0};
            int32_t bytes_received = read(clientSocket, buffer, sizeof(buffer));
            if (bytes_received > 0) cout << "Response from server: " << buffer << endl;    
            else throw runtime_error("No response was received from the server");
            istringstream responceStream(buffer);
            string firstWord;
            responceStream >> firstWord;
            if (firstWord != "Another") gettingTable(command, clientSocket);
        }
        close(clientSocket);
    }
    catch (exception& e) {
        cout << e.what() << endl;
    }
}
