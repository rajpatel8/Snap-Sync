//  header file for the server 

#include <Lord-Rajkumar.h>          // Local - env
// #include <bits/stdc++.h>         // Global - env

// beta - still unstable
// #include <server.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For inet_ntoa
#include <netdb.h> // For getaddrinfo()
#include <unistd.h> // For close()

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

        // setting up buffer to recive message
        char buffer[8] ; // 7 letter data + null terminator
        int bytes_recevied = recv(client_socket ,buffer , sizeof(buffer), 0) ;
        if(bytes_recevied==-1){
            perror("Error in reciving message from client") ;
            close(client_socket);
            continue;
        }

        buffer[bytes_recevied] = '\0' ; 

        // cout << "\nRecived the Message : " << buffer << '\n' ;

        string received_password(buffer);
        
        if (received_password == "THALA07") {
            cout << "Password authentication successful" << endl;
            close(client_socket) ; // closing it temporaily
            // TODO : open another socket for file transfer

        } else {
            cout << "Password authentication failed" << endl;
            close(client_socket) ;
            continue;
        }

    }

    // Close the server socket
    close(server_socket);

    return 0 ;

}
