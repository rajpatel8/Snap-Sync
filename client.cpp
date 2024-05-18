// #include <bits/stdc++.h>
// #include <openssl/sha.h>
#include <Lord-Rajkumar.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

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

int main() {
    // Create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(21);

    // Convert and set server IP address
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) { // Replace with server IP address
        perror("Invalid address/ Address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Receive token from the server
    char token[1024];
    if (recv(client_socket, token, sizeof(token), 0) == -1) {
        perror("Error receiving token from server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    cout << "\nToken received from the server: " << token << endl;

    // Generate hash of the token
    string token_string = string(token);
    string token_hash = to_string(fnv1a_hash(token_string));

    // Send the hash to the server
    if (send(client_socket, token_hash.c_str(), token_hash.size(), 0) == -1) {
        perror("Error sending hash to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    cout << "\nHash sent to the server\n";
    cout << "\nWaiting for the server to authenticate...\n";

    // Receive authentication status from the server
    char auth_status[1024] = {0};
    if (recv(client_socket, auth_status, sizeof(auth_status), 0) == -1) {
        perror("Error receiving authentication status from server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    cout << auth_status << endl;

    // Wait for server to send start signal
    char signal[1024] = {0};
    if (recv(client_socket, signal, sizeof(signal), 0) == -1) {
        perror("Error receiving start signal from server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    cout << "Server signal: " << signal << endl;

    // Logic to send files goes here
    // ...

    // Close client socket
    close(client_socket);

    return 0;
}
