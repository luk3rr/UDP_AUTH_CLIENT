#include <arpa/inet.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>

const uint32_t PORT     = 51001;
const uint16_t BUF_SIZE = 1024;

// Estrutura para a requisição de token individual (tipo 1)
struct IndividualTokenRequest
{
        uint16_t type;   // Tipo da mensagem (1)
        char     id[12]; // ID do aluno (NetID) com 12 bytes
        uint32_t nonce;  // Nonce de 4 bytes
} __attribute__((packed));

// Estrutura para a resposta de token individual (tipo 2)
struct IndividualTokenResponse
{
        uint16_t type;      // Tipo da mensagem (2)
        char     id[12];    // ID do aluno (NetID) com 12 bytes
        uint32_t nonce;     // Nonce de 4 bytes
        char     token[64]; // Token de 64 bytes (em hexadecimal)
} __attribute__((packed));

// Função para converter os números para a ordem de byte da rede
uint16_t to_network_short(uint16_t hostshort)
{
    return htons(hostshort);
}

uint32_t to_network_long(uint32_t hostlong)
{
    return htonl(hostlong);
}

uint16_t from_network_short(uint16_t netshort)
{
    return ntohs(netshort);
}

uint32_t from_network_long(uint32_t netlong)
{
    return ntohl(netlong);
}

void send_individual_token_request(const char* host,
                                   uint16_t    port,
                                   const char* id,
                                   uint32_t    nonce)
{
    int                           sockfd;
    struct sockaddr_in            server_addr;
    struct IndividualTokenRequest request;

    // Criar o socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configuração do endereço do servidor
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = to_network_short(port);
    server_addr.sin_addr.s_addr = inet_addr(host); // Suporta IPv4

    // Preencher a requisição
    request.type = to_network_short(1);     // Tipo 1: Requisição de token individual
    strncpy(request.id, id, 12);            // ID do aluno
    request.nonce = to_network_long(nonce); // Nonce

    // Enviar a requisição
    ssize_t sent_len = sendto(sockfd,
                              &request,
                              sizeof(request),
                              0,
                              (struct sockaddr*)&server_addr,
                              sizeof(server_addr));
    if (sent_len < 0)
    {
        perror("Erro ao enviar mensagem");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Requisição enviada para o servidor!" << std::endl;

    // Receber a resposta do servidor
    char      buffer[BUF_SIZE];
    socklen_t server_len = sizeof(server_addr);
    ssize_t   recv_len   = recvfrom(sockfd,
                                buffer,
                                BUF_SIZE,
                                0,
                                (struct sockaddr*)&server_addr,
                                &server_len);
    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    struct IndividualTokenResponse* response = (struct IndividualTokenResponse*)buffer;
    if (from_network_short(response->type) == 2)
    {
        std::cout << "Resposta do servidor: " << std::endl;
        std::cout << "ID: " << response->id << std::endl;
        std::cout << "Nonce: " << from_network_long(response->nonce) << std::endl;
        std::cout << "Token: " << response->token << std::endl;
    }

    close(sockfd);
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cerr << "Uso: ./client <host> <port> <command>" << std::endl;
        exit(EXIT_FAILURE);
    }

    const char* host    = argv[1];
    uint16_t    port    = atoi(argv[2]);
    const char* command = argv[3];

    if (strcmp(command, "itr") == 0)
    { // Requisição de token individual
        if (argc != 5)
        {
            std::cerr << "Uso para itr: ./client <host> <port> itr <id> <nonce>"
                      << std::endl;
            exit(EXIT_FAILURE);
        }
        const char* id    = argv[4];
        uint32_t    nonce = 1; // Exemplo de nonce, substitua conforme necessário
        send_individual_token_request(host, port, id, nonce);
    }

    return 0;
}
