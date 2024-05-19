#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>

#define DIRECTORY_PATH "/Users/rajkumar/Desktop/Snap-Sync/tmp/"

using namespace std;

// FNV-1a hash function
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

class Client {
    int client_socket;
    struct sockaddr_in server_addr;

    public:
        Client(const string& ip_address, int port) {
            create_socket();
            setup_server_address(ip_address, port);
            connect_to_server();
        }

        ~Client() {
            close(client_socket);
        }

        void run() {
            authenticate();
            send_files_in_directory(DIRECTORY_PATH);
        }

    private:
        void create_socket() {
            if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("Error creating socket");
                exit(EXIT_FAILURE);
            }
        }

        void setup_server_address(const string& ip_address, int port) {
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            if (inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr) <= 0) {
                perror("Invalid address/ Address not supported");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }

        void connect_to_server() {
            if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
                perror("Error connecting to server");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }

        void authenticate() {
            char token[1024];
            if (recv(client_socket, token, sizeof(token), 0) == -1) {
                perror("Error receiving token from server");
                exit(EXIT_FAILURE);
            }

            cout << "\nToken received from the server: " << token << endl;

            string token_string(token);
            string token_hash = to_string(fnv1a_hash(token_string));

            if (send(client_socket, token_hash.c_str(), token_hash.size() + 1, 0) == -1) { // include null terminator
                perror("Error sending hash to server");
                exit(EXIT_FAILURE);
            }

            cout << "\nHash sent to the server\n";
            cout << "\nWaiting for the server to authenticate...\n";

            char auth_status[1024] = {0};
            if (recv(client_socket, auth_status, sizeof(auth_status), 0) == -1) {
                perror("Error receiving authentication status from server");
                exit(EXIT_FAILURE);
            }

            cout << auth_status << endl;

            char signal[1024] = {0};
            if (recv(client_socket, signal, sizeof(signal), 0) == -1) {
                perror("Error receiving start signal from server");
                exit(EXIT_FAILURE);
            }

            cout << "Server signal: " << signal << endl;
        }

        bool send_file(const string& filepath) {
            ifstream file(filepath, ios::binary);
            if (!file) {
                perror("Error opening file");
                return false;
            }

            // Get the file size
            file.seekg(0, ios::end);
            size_t filesize = file.tellg();
            file.seekg(0, ios::beg);

            // Send the filename
            string filename = filepath.substr(filepath.find_last_of("/") + 1);
            if (send(client_socket, filename.c_str(), filename.size() + 1, 0) == -1) { // include null terminator
                perror("Error sending filename to server");
                file.close();
                return false;
            }

            // Send the file size
            if (send(client_socket, &filesize, sizeof(filesize), 0) == -1) {
                perror("Error sending file size to server");
                file.close();
                return false;
            }

            // Send the file data
            char buffer[1024];
            while (file.read(buffer, sizeof(buffer))) {
                if (send(client_socket, buffer, sizeof(buffer), 0) == -1) {
                    perror("Error sending file data to server");
                    file.close();
                    return false;
                }
            }
            if (file.gcount() > 0) {
                if (send(client_socket, buffer, file.gcount(), 0) == -1) {
                    perror("Error sending remaining file data to server");
                    file.close();
                    return false;
                }
            }

            file.close();
            return true;
        }

        void send_files_in_directory(const string& directory_path) {
            DIR* dir;
            struct dirent* ent;
            if ((dir = opendir(directory_path.c_str())) == NULL) {
                perror("Error opening directory");
                exit(EXIT_FAILURE);
            }

            while ((ent = readdir(dir)) != NULL) {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                    continue;
                }

                string filepath = directory_path + string(ent->d_name);
                if (!send_file(filepath)) {
                    closedir(dir);
                    exit(EXIT_FAILURE);
                }

                cout << "\nFile sent to the server: " << filepath << endl;

                // Wait for confirmation from the server before sending the next file
                char confirmation[1024];
                if (recv(client_socket, confirmation, sizeof(confirmation), 0) == -1) {
                    perror("Error receiving confirmation from server");
                    closedir(dir);
                    exit(EXIT_FAILURE);
                }
                cout << "Server confirmation: " << confirmation << endl;
            }

            // Send end signal to indicate all files have been sent
            string end_signal = "end_of_files";
            if (send(client_socket, end_signal.c_str(), end_signal.size() + 1, 0) == -1) { // include null terminator
                perror("Error sending end signal to server");
                exit(EXIT_FAILURE);
            }
            cout << "\nAll files sent to the server\n";

            closedir(dir);
        }
};

int main() {
    Client client("127.0.0.1", 6969);
    client.run();
    return 0;
}
