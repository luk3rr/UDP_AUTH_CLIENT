#include "tokens.h"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <unistd.h>

bool isPacketError(char* buffer, ssize_t packetSize)
{
    if (packetSize == sizeof(ErrorResponse))
    {
        ErrorResponse* error_response = reinterpret_cast<ErrorResponse*>(buffer);

        if (fromNetworkShort(error_response->type) == 256)
        {
            std::cout << *error_response << std::endl;
        }
        else
        {
            std::cerr << "Erro desconhecido" << std::endl;
        }
        return true;
    }

    return false;
}

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

    char buffer[BUF_SIZE];
    std::memset(buffer, CLEAN_CHAR, sizeof(buffer));
    ssize_t recv_len = socket.receive(buffer, sizeof(IndividualTokenResponse));

    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        exit(EXIT_FAILURE);
    }

    IndividualTokenResponse response;
    std::memcpy(&response, buffer, sizeof(IndividualTokenResponse));

    if (!isPacketError(buffer, recv_len))
    {
        std::cout << response << std::endl;
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

    char buffer[BUF_SIZE];
    std::memset(buffer, CLEAN_CHAR, sizeof(IndividualTokenStatus));
    ssize_t recv_len = socket.receive(buffer, sizeof(IndividualTokenStatus));

    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        exit(EXIT_FAILURE);
    }

    IndividualTokenStatus status;
    std::memcpy(&status, buffer, sizeof(IndividualTokenStatus));

    if (!isPacketError(buffer, recv_len))
    {
        std::cout << status << std::endl;
    }
}

void sendGroupTokenRequest(const char* host, uint16_t port, std::vector<SAS>& sas)
{
    UdpSocket socket(host, port);

    GroupTokenRequest request(sas);
    char              serializedRequest[request.packetSize()];
    request.serialize(serializedRequest);

    if (socket.send(serializedRequest, request.packetSize()) < 0)
    {
        perror("Erro ao enviar mensagem");
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE];
    std::memset(buffer, CLEAN_CHAR, sizeof(buffer));
    ssize_t recv_len = socket.receive(buffer, BUF_SIZE);

    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        exit(EXIT_FAILURE);
    }

    if (!isPacketError(buffer, recv_len))
    {
        std::cout << getGroupTokenResponse(buffer, request) << std::endl;
    }
}

void sendGroupTokenValidation(const char* host, uint16_t port, const char* sas)
{
    UdpSocket socket(host, port);

    GroupTokenValidation validation = parseGroupTokenValidationFromString(sas);

    char* serializedValidation = new char[validation.packetSize()];
    std::memset(serializedValidation, CLEAN_CHAR, validation.packetSize());

    validation.serialize(serializedValidation);

    if (socket.send(serializedValidation, validation.packetSize()) < 0)
    {
        perror("Erro ao enviar mensagem");
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE];
    std::memset(buffer, CLEAN_CHAR, sizeof(buffer));
    ssize_t recv_len = socket.receive(buffer, BUF_SIZE);

    if (recv_len < 0)
    {
        perror("Erro ao receber resposta");
        exit(EXIT_FAILURE);
    }

    int status = getGroupTokenStatus(buffer, validation);

    if (!isPacketError(buffer, recv_len))
    {
        std::cout << status << std::endl;
    }

    delete serializedValidation;
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
