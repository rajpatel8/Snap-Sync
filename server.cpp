// #include <Lord-Rajkumar.h>
//  header file for the server 
#include <bits/stdc++.h>
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
    gethostname(hostbuffer, sizeof(hostbuffer));
    info.hostname = hostbuffer;

    struct hostent *host_entry = gethostbyname(hostbuffer);
    if (host_entry) {
        struct in_addr **addr_list = (struct in_addr **)host_entry->h_addr_list;
        for (int i = 0; addr_list[i] != NULL; i++) {
            info.ip_addresses += inet_ntoa(*addr_list[i]);
            if (addr_list[i+1] != NULL) {
                info.ip_addresses += " ";
            }
        }
    }
    return info;
}

int main()
{
    HostInfo info =getHostAndIP() ;   
    std :: cout << " HostName : "<< info.hostname << '\n';
    std :: cout << " Host IP  : "<< info.ip_addresses << '\n';
    return 0 ;
}
