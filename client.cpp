// #include <bits/stdc++.h>
#include <openssl/sha.h>
#include <Lord-Rajkumar.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

// string hashStringToSHA256(const std::string& input) {
//     // Create a buffer to hold the SHA-256 hash
//     unsigned char hash[SHA256_DIGEST_LENGTH];
    
//     // Compute the SHA-256 hash
//     SHA256_CTX sha256;
//     SHA256_Init(&sha256);
//     SHA256_Update(&sha256, input.c_str(), input.size());
//     SHA256_Final(hash, &sha256);
    
//     // Convert the hash to a hex string
//     std::ostringstream hexStream;
//     for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
//         hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
//     }
    
//     return hexStream.str();
// }

// FNV-1a hash function
size_t fnv1a_hash(const std::string& str) {
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
    server_addr.sin_addr.s_addr = '127.0.0.1';

    inet_pton(AF_INET,"127.0.0.1" , &server_addr.sin_addr); // Replace with server IP address

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // recive token from the serve  
    char token[1024];
    if (recv(client_socket, token, sizeof(token), 0) == -1) {
        perror("Error receiving token from server");
        exit(EXIT_FAILURE);
    }
    // token recieved from the server
    cout << "\nToken recieved from the server\n" ;
    cout << "\nGenerating Hash of the token\n" ;

    string token_string = string(token);
    string token_hash = to_string(fnv1a_hash(token_string));

    // sending the hash to the server
    if (send(client_socket, token_hash.c_str(), token_hash.size(), 0) == -1) {
        perror("Error sending hash to server");
        exit(EXIT_FAILURE);
    }
    cout << "\nHash sent to the server\n" ;
    cout << "\nWaiting for the server to authenticate\n" ;

    // reciving the authentication status from the server
    char auth_status[1024];
    if (recv(client_socket, auth_status, sizeof(auth_status), 0) == -1) {
        perror("Error receiving authentication status from server");
        exit(EXIT_FAILURE);
    }
    
    // For debug only
    // changing the sizec of the auth_status 
    int size = sizeof(auth_status);
    auth_status[size] = '\0';
    // sleep(1);
    cout << auth_status << '\n';    
    
    // Close client socket
    close(client_socket);

    return 0;
}