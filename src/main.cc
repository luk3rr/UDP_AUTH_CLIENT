#include "tokens.h"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <unistd.h>

void sendIndividualTokenRequest(const char* host,
                                uint16_t    port,
                                const char* id,
                                uint32_t    nonce)
{
    UdpSocket socket(host, port);

    IndividualTokenRequest request(id, nonce);
    if (socket.send(&request, sizeof(request)) < 0)
    {
        perror("Erro ao enviar mensagem");
        exit(EXIT_FAILURE);
    }

    std::cout << "Requisição enviada para o servidor!" << std::endl;

    char    buffer[BUF_SIZE];
    std::memset(buffer, CLEAN_CHAR, sizeof(buffer));
    ssize_t recv_len = socket.receive(buffer, sizeof(IndividualTokenResponse));

    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        exit(EXIT_FAILURE);
    }

    IndividualTokenResponse response;
    std::memcpy(&response, buffer, sizeof(IndividualTokenResponse));

    if (fromNetworkShort(response.type) == 2)
    {
        std::cout << "Resposta do servidor:" << std::endl;
        std::cout << response << std::endl;
    }
    else
    {
        ErrorResponse* error_response = reinterpret_cast<ErrorResponse*>(buffer);

        if (fromNetworkShort(error_response->type) == 256)
        {
            std::cout << *error_response << std::endl;
        }
    }
}

void sendIndividualTokenValidation(const char* host, uint16_t port, const char* sas)
{
    UdpSocket socket(host, port);

    IndividualTokenValidation validation =
        parseIndividualTokenValidationFromString(sas);

    char serializedValidation[sizeof(validation)];
    validation.serialize(serializedValidation);

    if (socket.send(serializedValidation, sizeof(validation)) < 0)
    {
        perror("Erro ao enviar mensagem");
        exit(EXIT_FAILURE);
    }

    std::cout << "Validação enviada para o servidor!" << std::endl;

    char    buffer[BUF_SIZE];
    std::memset(buffer, CLEAN_CHAR, sizeof(IndividualTokenStatus));
    ssize_t recv_len = socket.receive(buffer, sizeof(IndividualTokenStatus));

    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        exit(EXIT_FAILURE);
    }

    IndividualTokenStatus status;
    std::memcpy(&status, buffer, sizeof(IndividualTokenStatus));

    if (fromNetworkShort(status.type) == 4)
    {
        std::cout << "Resposta do servidor:" << std::endl;
        std::cout << status << std::endl;
    }
    else
    {
        ErrorResponse* error_response = reinterpret_cast<ErrorResponse*>(buffer);

        if (fromNetworkShort(error_response->type) == 256)
        {
            std::cout << *error_response << std::endl;
        }
    }
}

void sendGroupTokenRequest(const char* host, uint16_t port, std::vector<SAS>& sas)
{
    UdpSocket socket(host, port);

    GroupTokenRequest request(sas);
    char serializedRequest[request.packetSize()];
    request.serialize(serializedRequest);

    if (socket.send(serializedRequest, request.packetSize()) < 0)
    {
        perror("Erro ao enviar mensagem");
        exit(EXIT_FAILURE);
    }

    std::cout << "Requisição enviada para o servidor!" << std::endl;

    char    buffer[BUF_SIZE];
    std::memset(buffer, CLEAN_CHAR, sizeof(buffer));
    ssize_t recv_len = socket.receive(buffer, BUF_SIZE);

    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        exit(EXIT_FAILURE);
    }

    GroupTokenResponse response = parseGroupTokenResponse(
        buffer, recv_len, fromNetworkShort(request.n));

    if (fromNetworkShort(response.type) == 6)
    {
        std::cout << "Resposta do servidor:" << std::endl;
        std::cout << response << std::endl;
    }
    else
    {
        ErrorResponse* error_response = reinterpret_cast<ErrorResponse*>(buffer);

        if (fromNetworkShort(error_response->type) == 256)
        {
            std::cout << *error_response << std::endl;
        }
    }
}

void sendGroupTokenValidation(const char* host, uint16_t port, const char* sas)
{
    UdpSocket socket(host, port);

    GroupTokenValidation validation = parseGroupTokenValidationFromString(sas);

    std::cout << validation << std::endl;

    char* serializedValidation = new char[validation.packetSize()];
    std::memset(serializedValidation, CLEAN_CHAR, validation.packetSize());

    validation.serialize(serializedValidation);

    if (socket.send(serializedValidation, validation.packetSize()) < 0)
    {
        perror("Erro ao enviar mensagem");
        exit(EXIT_FAILURE);
    }

    std::cout << "Validação enviada para o servidor!" << std::endl;

    char    buffer[BUF_SIZE];
    std::memset(buffer, CLEAN_CHAR, sizeof(buffer));
    ssize_t recv_len = socket.receive(buffer, BUF_SIZE);

    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        exit(EXIT_FAILURE);
    }

    GroupTokenStatus status = parseGroupTokenStatus(
        buffer, recv_len, fromNetworkShort(validation.n));

    if (fromNetworkShort(status.type) == 8)
    {
        std::cout << "Resposta do servidor:" << std::endl;
        std::cout << status << std::endl;
    }
    else
    {
        ErrorResponse* error_response = reinterpret_cast<ErrorResponse*>(buffer);

        if (fromNetworkShort(error_response->type) == 256)
        {
            std::cout << *error_response << std::endl;
        }
    }
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
    {
        if (argc != 6)
        {
            std::cerr << "Uso para itr: ./client <host> <port> itr <id> <nonce>"
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        const char* id    = argv[4];
        uint32_t    nonce = atoi(argv[5]);
        sendIndividualTokenRequest(host, port, id, nonce);
    }
    else if (strcmp(command, "itv") == 0)
    {
        if (argc != 5)
        {
            std::cerr << "Uso para itv: ./client <host> <port> itv <SAS>" << std::endl;
            exit(EXIT_FAILURE);
        }

        const char* sas = argv[4];
        sendIndividualTokenValidation(host, port, sas);
    }
    else if (strcmp(command, "gtr") == 0)
    {
        if (argc < 6)
        {
            std::cerr << "Uso para gtr: ./client <host> <port> gtr N <SAS-1> <SAS-2> "
                         "... <SAS-N>"
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        uint16_t n = atoi(argv[4]);

        if (argc != 5 + n)
        {
            std::cerr << "Uso para gtr: ./client <host> <port> gtr N <SAS-1> <SAS-2> "
                         "... <SAS-N>"
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        std::vector<SAS> gas;
        gas.reserve(n);

        for (int i = 0; i < n; ++i)
        {
            gas.emplace_back(argv[5 + i]);
        }

        sendGroupTokenRequest(host, port, gas);
    }
    else if (strcmp(command, "gtv") == 0)
    {
        if (argc != 5)
        {
            std::cerr << "Uso para gtv: ./client <host> <port> gtv <GAS>" << std::endl;
            exit(EXIT_FAILURE);
        }

        const char* sas = argv[4];
        sendGroupTokenValidation(host, port, sas);
    }
    else
    {
        std::cerr << "Comando inválido" << std::endl;
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
