#include <Lord-Rajkumar.h>
//  header file for the server 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For inet_ntoa
#include <netdb.h> // For getaddrinfo()
#include <unistd.h> // For close()

using namespace std;

void check_hostname(int hostname){
    if(hostname==-1){
        perror("gethostname") ;
        exit(1) ;
    }

}

void check_host_entry(struct hostent *host_entry){
    if(host_entry==NULL){
        perror("Check_host_entry") ;
        exit(1) ;
    }
}

void check_IP_buffer(char *IP_buffer){
    if(IP_buffer==NULL){
        perror("Check_IP_buffer") ;
        exit(1) ;
    }
}



int main()
{
    int Lord_Rajkumar = socket(AF_INET, SOCK_STREAM,0);
    if(Lord_Rajkumar == -1)
    {
        cout << "Socket creation failed" << endl;
        return 1;
    }

    char hostname_buffer[256];
    struct hostent *host_entry;
    char * IP_buffer ;

    int hostname = gethostname(hostname_buffer, sizeof(hostname_buffer));
    check_hostname(hostname);
    
    host_entry = gethostbyname(hostname_buffer) ;
    check_host_entry(host_entry) ;

    IP_buffer = inet_ntoa(*((struct in_addr*) host_entry ->h_addr_list[0])) ;
    check_IP_buffer(IP_buffer) ;

    cout << hostname_buffer << '\n' ;
    cout << IP_buffer << '\n' ;

    return 0 ;
}
