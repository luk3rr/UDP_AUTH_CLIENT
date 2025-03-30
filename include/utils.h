#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdint.h>
#include <unistd.h>
#include <iomanip>

const uint16_t DEFAULT_TIMEOUT = 3; // seconds

class UdpSocket
{
    public:
        UdpSocket(const std::string& host,
                  uint16_t           port,
                  uint16_t           timeout = DEFAULT_TIMEOUT)
        {
            struct addrinfo hints{}, *res;
            hints.ai_family   = AF_UNSPEC;
            hints.ai_socktype = SOCK_DGRAM;

            std::string port_str = std::to_string(port);
            if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0)
            {
                perror("Erro ao resolver host");
                exit(EXIT_FAILURE);
            }

            sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if (sockfd < 0)
            {
                perror("Erro ao criar socket");
                freeaddrinfo(res);
                exit(EXIT_FAILURE);
            }

            memcpy(&server_addr, res->ai_addr, res->ai_addrlen);
            server_addr_len = res->ai_addrlen;
            freeaddrinfo(res);

            struct timeval timeout_val;
            timeout_val.tv_sec  = timeout; // Tempo de espera em segundos
            timeout_val.tv_usec = 0;       // Sem microssegundos
            if (setsockopt(sockfd,
                           SOL_SOCKET,
                           SO_RCVTIMEO,
                           &timeout_val,
                           sizeof(timeout_val)) < 0)
            {
                perror("Erro ao configurar timeout");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }

        ~UdpSocket()
        {
            close(sockfd);
        }

        ssize_t send(const void* data, size_t size)
        {
            return sendto(sockfd,
                          data,
                          size,
                          0,
                          (struct sockaddr*)&server_addr,
                          server_addr_len);
        }

        ssize_t receive(void* buffer, size_t size)
        {
            return recvfrom(sockfd,
                            buffer,
                            size,
                            0,
                            (struct sockaddr*)&server_addr,
                            &server_addr_len);
        }

    private:
        int                     sockfd;
        struct sockaddr_storage server_addr;
        socklen_t               server_addr_len;
};

uint16_t toNetworkShort(uint16_t hostshort)
{
    return htons(hostshort);
}

uint32_t toNetworkLong(uint32_t hostlong)
{
    return htonl(hostlong);
}

uint16_t fromNetworkShort(uint16_t netshort)
{
    return ntohs(netshort);
}

uint32_t fromNetworkLong(uint32_t netlong)
{
    return ntohl(netlong);
}

enum ErrorCode : uint16_t
{
    INVALID_MESSAGE_CODE     = 1,
    INCORRECT_MESSAGE_LENGTH = 2,
    INVALID_PARAMETER        = 3,
    INVALID_SINGLE_TOKEN     = 4,
    ASCII_DECODE_ERROR       = 5
};

std::string getErrorDescription(uint16_t error_code)
{
    switch (error_code)
    {
        case INVALID_MESSAGE_CODE:
            return "Erro " + std::to_string(error_code) +
                   ": Código de mensagem inválido. O cliente enviou uma solicitação "
                   "com um tipo desconhecido.";
        case INCORRECT_MESSAGE_LENGTH:
            return "Erro " + std::to_string(error_code) +
                   ": Tamanho de mensagem incorreto. O cliente enviou uma solicitação "
                   "cujo tamanho é incompatível com o tipo de solicitação.";
        case INVALID_PARAMETER:
            return "Erro " + std::to_string(error_code) +
                   ": Parâmetro inválido. O servidor detectou um erro em um dos campos "
                   "da solicitação.";
        case INVALID_SINGLE_TOKEN:
            return "Erro " + std::to_string(error_code) +
                   ": Token único inválido. Um SAS em um GAS é inválido.";
        case ASCII_DECODE_ERROR:
            return "Erro " + std::to_string(error_code) +
                   ": Erro de decodificação ASCII. A mensagem contém um caractere "
                   "não-ASCII.";
        default:
            return "Erro " + std::to_string(error_code) + ": Erro desconhecido.";
    }
}

bool isValidAscii(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), [](unsigned char c) {
        return c <= 127;
    });
}

std::string titleOutput(std::string title)
{

    std::string output = "\n";
    output += std::string(title.size() + 4, '#') + "\n";
    output += "# " + title + " #\n";
    output += std::string(title.size() + 4, '#');

    return output;
}

// Função para imprimir o buffer recebido em bytes
void printBufferHex(const char* buffer, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (static_cast<unsigned int>(static_cast<unsigned char>(buffer[i]))) << " ";

        // Quebra de linha a cada 16 bytes para facilitar leitura
        if ((i + 1) % 16 == 0)
            std::cout << std::endl;
    }
    
    std::cout << std::dec << std::endl; // Retorna para decimal após impressão
}