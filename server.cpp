#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <unistd.h>       // For close() and gethostname()
#include <arpa/inet.h>    // For inet_ntoa
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>        // For getaddrinfo()
#include <random>
#include <sys/stat.h>     // For mkdir
#include <openssl/evp.h>

using namespace std;

struct HostInfo {
    string hostname;
    string ip_addresses;
};

HostInfo getHostAndIP() {
    HostInfo info;
    char hostbuffer[256];
    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
        perror("Error getting hostname");
        exit(EXIT_FAILURE);
    }
    info.hostname = hostbuffer;

    struct hostent *host_entry = gethostbyname(hostbuffer);
    if (host_entry == nullptr) {
        herror("Error getting host by name");
        exit(EXIT_FAILURE);
    }

    struct in_addr **addr_list = (struct in_addr **)host_entry->h_addr_list;
    for (int i = 0; addr_list[i] != NULL; i++) {
        info.ip_addresses += inet_ntoa(*addr_list[i]);
        if (addr_list[i + 1] != NULL) {
            info.ip_addresses += " ";
        }
    }
    return info;
}

bool authenticateClient(const string& receivedHash, const string& expectedHash) {
    return receivedHash == expectedHash;
}

string generateRandomString(size_t length) {
    const string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:',.<>?/~`";
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> distribution(0, characters.size() - 1);

    string randomString;
    for (size_t i = 0; i < length; ++i) {
        randomString += characters[distribution(generator)];
    }
    return randomString;
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

bool createFolder(const string& folderName) {
    if (mkdir(folderName.c_str(), 0755) == -1) {
        perror("Error creating folder");
        return false;
    }
    return true;
}

void handleClient(int client_socket) {
    // Send random string to client
    string Token = generateRandomString(256);
    cout << "Token: " << Token << '\n';
    if (send(client_socket, Token.c_str(), Token.size(), 0) == -1) {
        perror("Error sending token to client");
        close(client_socket);
        return;
    }
    cout << "Token sent to client\nWaiting for client to send hash\n";

    // Hash the token and wait for the client to send the hash
    string expectedHash = to_string(fnv1a_hash(Token));
    char receivedHash[1024] = {0};
    if (recv(client_socket, receivedHash, sizeof(receivedHash), 0) == -1) {
        perror("Error receiving hash from client");
        close(client_socket);
        return;
    }
    cout << "Hash received from client\nChecking hash...\n";

    string receivedHashString = string(receivedHash);
    if (authenticateClient(receivedHashString, expectedHash)) {
        cout << "Hash verified!\n";
        string successMessage = "Authentication successful\n";
        if (send(client_socket, successMessage.c_str(), successMessage.size(), 0) == -1) {
            perror("Error sending success message to client");
            close(client_socket);
            return;
        }

        // Create folder with today's date
        time_t now = time(0);
        tm *ltm = localtime(&now);
        string folderName = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday);
        if (createFolder(folderName)) {
            cout << "Folder created successfully\n";
        } else {
            cout << "Error creating folder\n";
            close(client_socket);
            return;
        }

        string signal = "start";
        if (send(client_socket, signal.c_str(), signal.size(), 0) == -1) {
            perror("Error sending signal to client");
        }
    } else {
        cout << "Hash verification failed!\n";
    }

    close(client_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(21);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) == -1) {
        perror("Error listening on port 21");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    cout << "Listening on port 21\n";

    while (true) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("Client socket setup error");
            continue;
        }

        cout << "Connection accepted from client\n";
        handleClient(client_socket);
    }

    close(server_socket);
    return 0;
}