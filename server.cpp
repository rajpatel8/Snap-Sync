//  header file for the server 

#include <Lord-Rajkumar.h>          // Local - env
// #include <bits/stdc++.h>         // Global - env

// beta - still unstable
// #include <server.h>
#include <openssl/evp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For inet_ntoa
#include <netdb.h> // For getaddrinfo()
#include <unistd.h> // For close()
#include <random>

using namespace std;

struct HostInfo {
    std::string hostname;
    std::string ip_addresses;
};

HostInfo getHostAndIP() {
    HostInfo info;
    char hostbuffer[256];
    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
        perror("\nError getting hostname\n");
        exit(EXIT_FAILURE);
    }
    info.hostname = hostbuffer;

    struct hostent *host_entry = gethostbyname(hostbuffer);
    if (host_entry == nullptr) {
        herror("\nError getting host by name\n");
        exit(EXIT_FAILURE);
    }

    struct in_addr **addr_list = (struct in_addr **)host_entry->h_addr_list;
    for (int i = 0; addr_list[i] != NULL; i++) {
        info.ip_addresses += inet_ntoa(*addr_list[i]);
        if (addr_list[i+1] != NULL) {
            info.ip_addresses += " ";
        }
    }
    return info;
}

bool authenticateClient(const std::string& receivedHash, const std::string& expectedHash) {
    return receivedHash == expectedHash;
}

// string hashStringToSHA256(const std::string& input) {
//     EVP_MD_CTX* context = EVP_MD_CTX_new();
//     const EVP_MD* md = EVP_sha256();
//     unsigned char hash[EVP_MAX_MD_SIZE];
//     unsigned int lengthOfHash = 0;

//     EVP_DigestInit_ex(context, md, nullptr);
//     EVP_DigestUpdate(context, input.c_str(), input.size());
//     EVP_DigestFinal_ex(context, hash, &lengthOfHash);
//     EVP_MD_CTX_free(context);

//     std::ostringstream hexStream;
//     for (unsigned int i = 0; i < lengthOfHash; ++i) {
//         hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
//     }
//     return hexStream.str();
// }

string generateRandomString(size_t length) {
    const string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:',.<>?/~`";
    random_device rd;  // Non-deterministic random device
    mt19937 generator(rd());  // Mersenne Twister RNG
    uniform_int_distribution<> distribution(0, characters.size() - 1);

    string randomString;
    for (size_t i = 0; i < length; ++i) {
        randomString += characters[distribution(generator)];
    }

    return randomString;
}

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

int main()
{
    /* Only for debug/testing
     * HostInfo info =getHostAndIP() ;      
     * cout << "\nServer Name: " << info.hostname << '\n';
     * cout << "\nSerfver IP : " << info.ip_addresses << '\n';
    */

    // creating socket

    int server_socket = socket(AF_INET,SOCK_STREAM,0) ;
    if(server_socket == -1)
    { 
        perror("\nError Creating Socket\n") ;
        exit(EXIT_FAILURE) ;
    }

    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET ;
    server_addr.sin_addr.s_addr = INADDR_ANY ;
    server_addr.sin_port = htons(21) ;

    // binding socket to ip and port
    if (bind(server_socket,(struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("\nError in binding\n") ;
        exit(EXIT_FAILURE);
    }
    // cout << "\n Socket binding successful\n" ; // only for debug

    // Listening on port 21
    if(listen(server_socket,3) == -1){ // Maximum 3 pending Connection
        perror("\nError listning on port 21\n") ;
        exit(EXIT_FAILURE);
    }
    // cout << "\n listning on port 21 \n" ;   // only for debug

    while (true){
        int client_socket = accept(server_socket,NULL, NULL) ;
        if(client_socket == -1){
            perror("\nClient Socket setup error\n") ;
            continue;
        }
         
         cout << "\nConnection accepted from client\n" ;

        // send random string to client
        string Token = generateRandomString(256);
        cout << "\nToken : " << Token << '\n';
        if (send(client_socket, Token.c_str(), Token.size(), 0) == -1) {
            perror("\nError sending token to client\n");
            close(client_socket);
            continue;
        }
        cout << "\nToken sent to client\n";
        cout << "\nWaiting for client to send hash\n";
        // hash the token and wait for the client to send the hash
        string expectedHash = to_string(fnv1a_hash(Token));
        char receivedHash[1024];
        if (recv(client_socket, receivedHash, sizeof(receivedHash), 0) == -1) {
            perror("\nError receiving hash from client\n");
            close(client_socket);
            continue;
        }
        cout << "\nHash received from client\n";
        string receivedHashString = string(receivedHash);
        // check if the received hash matches the expected hash
        cout << "\nChecking hash...\n";
        if (authenticateClient(receivedHashString, expectedHash)) {
            cout << "\nHash verified!\n";
            // send success message to client
            string successMessage = "Authentication successful";
            if (send(client_socket, successMessage.c_str(), successMessage.size(), 0) == -1) {
                perror("\nError sending success message to client\n");
                close(client_socket);
                continue;
            }
            
            // close(client_socket);
        }
        else {
            cout << "\nHash verification failed!\n";
            close(client_socket);
            continue;
        }
        
    }

    // Close the server socket
    close(server_socket);

    return 0 ;

}
