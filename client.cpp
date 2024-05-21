#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 6969
#define BUFFER_SIZE 1024

using namespace std;

class Client {
private:
    int client_socket;
    struct sockaddr_in server_address;

public:
    Client() {
        create_socket();
        setup_server_address();
        connect_to_server();
    }

    ~Client() {
        close(client_socket);
    }

    void communicate() {
        char buffer[BUFFER_SIZE] = {0};
        while (true) {
            cout << "Enter command (type 'exit' to quit): ";
            string command;
            getline(cin, command);

            if (command == "exit") {
                send_message(command);
                break;
            }

            send_message(command);

            usleep(100000); 
            
            receive_message(buffer);

            cout << "Server response: " << buffer << endl;
            memset(buffer, 0, BUFFER_SIZE);
        }
    }

private:
    void create_socket() {
        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("[ERROR] : Socket creation failed");
            exit(EXIT_FAILURE);
        }
        cout << "[LOG] : Socket created successfully.\n";
    }

    void setup_server_address() {
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);
        if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
            perror("[ERROR] : Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }
    }

    void connect_to_server() {
        if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            perror("[ERROR] : Connection failed");
            exit(EXIT_FAILURE);
        }
        cout << "[LOG] : Connected to the server.\n";
    }

    void send_message(const string& message) {
        if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
            perror("[ERROR] : Sending message failed");
            exit(EXIT_FAILURE);
        }
    }

    void receive_message(char* buffer) {
        if (recv(client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("[ERROR] : Receiving message failed");
            exit(EXIT_FAILURE);
        }
    }
};

int main() {
    Client client;
    client.communicate();
    return 0;
}
