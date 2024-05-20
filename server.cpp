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

public:
    Server(int port = 6969) : PORT(port) {
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
        flush_socket_buffer(client_socket);

        string token = generate_random_string(TOKEN_LENGTH);
        cout << "Token: " << token << '\n';

        if (send(client_socket, token.c_str(), token.size() + 1, 0) == -1) { // include null terminator
            perror("[ERROR] : Sending token to client failed");
            close(client_socket);
            return;
        }
        cout << "[LOG] : Token sent to client. Waiting for hash...\n";

        flush_socket_buffer(client_socket);

        string expected_hash = to_string(fnv1a_hash(token));
        char received_hash[1024] = {0};
        if (recv(client_socket, received_hash, sizeof(received_hash), 0) == -1) {
            perror("[ERROR] : Receiving hash from client failed");
            close(client_socket);
            return;
        }
        cout << "[LOG] : Hash received from client. Checking hash...\n";

        if (authenticate_client(string(received_hash), expected_hash)) {
            cout << "[LOG] : Hash verified!\n";
            send_message("Authentication successful\n");

            string folder_name = create_timestamped_folder();
            if (!folder_name.empty()) {
                cout << "[LOG] : Folder created successfully.\n";
                send_message("start");

                receive_files(folder_name);
            } else {
                cout << "[ERROR] : Folder creation failed.\n";
            }
        } else {
            cout << "[LOG] : Hash verification failed!\n";
        }

        close(client_socket);
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

    string generate_random_string(size_t length) {
        const string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:',.<>?/~`";
        random_device rd;
        mt19937 generator(rd());
        uniform_int_distribution<> distribution(0, characters.size() - 1);

        string random_string;
        for (size_t i = 0; i < length; ++i) {
            random_string += characters[distribution(generator)];
        }
        return random_string;
    }

    size_t fnv1a_hash(const string& str) {
        const size_t FNV_offset_basis = 14695981039346656037ULL;
        const size_t FNV_prime = 1099511628211ULL;

        size_t hash = FNV_offset_basis;
        for (char c : str) {
            hash ^= static_cast<size_t>(c);
            hash *= FNV_prime;
        }
        return hash;
    }

    bool authenticate_client(const string& received_hash, const string& expected_hash) {
        return received_hash == expected_hash;
    }

    bool create_folder(const string& folder_name) {
        if (mkdir(folder_name.c_str(), 0755) == -1) {
            perror("[ERROR] : Creating folder failed");
            return false;
        }
        return true;
    }

    string create_timestamped_folder() {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        string folder_name = to_string(1900 + ltm->tm_year) + "-" +
                             to_string(1 + ltm->tm_mon) + "-" +
                             to_string(ltm->tm_mday) + "_" +
                             to_string(ltm->tm_hour) + "-" +
                             to_string(ltm->tm_min) + "-" +
                             to_string(ltm->tm_sec);
        return create_folder(folder_name) ? folder_name : "";
    }

    void receive_files(const string& folder_name) {
        while (true) {
            flush_socket_buffer(client_socket);

            char filename[1024] = {0};
            if (recv(client_socket, filename, sizeof(filename), 0) == -1) {
                perror("[ERROR] : Receiving filename from client failed");
                return;
            }

            // Check if filename is empty
            if (strlen(filename) == 0) {
                cerr << "[ERROR] : Received empty filename, skipping.\n";
                continue;
            }

            if (strcmp(filename, "end_of_files") == 0) {
                break;
            }
            cout << "[LOG] : Receiving file: " << filename << endl;

            size_t filesize;
            if (recv(client_socket, &filesize, sizeof(filesize), 0) == -1) {
                perror("[ERROR] : Receiving file size from client failed");
                return;
            }
            cout << "[LOG] : File size: " << filesize << " bytes\n";

            string filepath = folder_name + "/" + string(filename);
            ofstream file(filepath, ios::binary);
            if (!file) {
                perror("[ERROR] : Opening file for writing failed");
                return;
            }

            char buffer[1024];
            size_t bytes_received = 0;
            while (bytes_received < filesize) {
                ssize_t n = recv(client_socket, buffer, min(sizeof(buffer), filesize - bytes_received), 0);
                if (n == -1) {
                    perror("[ERROR] : Receiving file data from client failed");
                    return;
                }
                file.write(buffer, n);
                bytes_received += n;
            }

            cout << "[LOG] : File received and saved to " << filepath << endl;
            send_message("File received: " + string(filename));
        }
        cout << "[LOG] : All files received.\n";
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
};

int main() {
    Server server(6969);
    server.run();
    return 0;
}
