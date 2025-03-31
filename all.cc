#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <netinet/in.h>
#include <string>

const uint16_t DEFAULT_TIMEOUT = 3; // seconds
const uint16_t BUF_SIZE        = 1024;
const char     CLEAN_CHAR      = ' ';

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

void printBufferHex(const void* data, size_t size)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    for (size_t i = 0; i < size; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(bytes[i]) << " ";

        // Quebra de linha a cada 16 bytes para facilitar leitura
        if ((i + 1) % 16 == 0)
            std::cout << std::endl;
    }
    std::cout << std::dec << std::endl; // Retorna ao formato decimal
}

std::string removeSpaces(const char* charArray)
{
    std::string result;
    size_t      length = std::strlen(charArray);

    for (size_t i = 0; i < length; ++i)
    {
        if (charArray[i] != ' ' && charArray[i] != '\0')
        {
            result.push_back(charArray[i]);
        }
    }
    return result;
}

/* [1]

    0         2                        14                  18
    +----+----+----+----/    /----+----+----+----+----+----+
    | 1       | ID                     | nonce             |
    +----+----+----+----/    /----+----+----+----+----+----+
*/
class IndividualTokenRequest
{
    public:
        uint16_t type;
        char     id[12];
        uint32_t nonce;

        IndividualTokenRequest()
            : type(toNetworkShort(1)),
              nonce(toNetworkLong(0))
        {
            std::memset(id, CLEAN_CHAR, sizeof(id));
        }

        IndividualTokenRequest(std::string id, uint32_t nonce)
            : type(toNetworkShort(1)),
              nonce(toNetworkLong(nonce))
        {
            std::memset(this->id, CLEAN_CHAR, sizeof(this->id));
            std::memcpy(this->id, id.c_str(), std::min(id.size(), sizeof(this->id)));
        }
} __attribute__((packed));

/* [2]

    0       2               14              18                    82
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+
    | 2     | ID            | nonce         | token               |
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+
*/
class IndividualTokenResponse
{
    public:
        uint16_t type;
        char     id[12];
        uint32_t nonce;
        char     token[64];

        IndividualTokenResponse()
            : type(toNetworkShort(2)),
              nonce(toNetworkLong(0))
        {
            std::memset(id, CLEAN_CHAR, sizeof(id));
            std::memset(token, CLEAN_CHAR, sizeof(token));
        }

        IndividualTokenResponse(std::string id, uint32_t nonce, std::string token)
            : type(toNetworkShort(2)),
              nonce(toNetworkLong(nonce))
        {
            std::memset(this->id, CLEAN_CHAR, sizeof(this->id));
            std::memset(this->token, CLEAN_CHAR, sizeof(this->token));

            std::memcpy(this->id, id.c_str(), std::min(id.size(), sizeof(this->id)));
            std::memcpy(this->token,
                        token.c_str(),
                        std::min(token.size(), sizeof(this->token)));
        }

        friend std::ostream& operator<<(std::ostream&                  os,
                                        const IndividualTokenResponse& response)
        {
            os << removeSpaces(response.id) << ":" << fromNetworkLong(response.nonce)
               << ":" << std::string(response.token, sizeof(response.token));
            return os;
        }
} __attribute__((packed));

/* [3]

    0       2               14              18                    82
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+
    | 3     | ID            | nonce         | token               |
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+
*/
class IndividualTokenValidation
{
    public:
        uint16_t type;
        char     id[12];
        uint32_t nonce;
        char     token[64];

        IndividualTokenValidation()
            : type(toNetworkShort(3)),
              nonce(toNetworkLong(0))
        {
            std::memset(id, CLEAN_CHAR, sizeof(id));
            std::memset(token, CLEAN_CHAR, sizeof(token));
        }

        IndividualTokenValidation(std::string id, uint32_t nonce, std::string token)
            : type(toNetworkShort(3)),
              nonce(toNetworkLong(nonce))
        {
            std::memset(this->id, CLEAN_CHAR, sizeof(this->id));
            std::memset(this->token, CLEAN_CHAR, sizeof(this->token));

            std::memcpy(this->id, id.c_str(), std::min(id.size(), sizeof(this->id)));
            std::memcpy(this->token,
                        token.c_str(),
                        std::min(token.size(), sizeof(this->token)));
        }

        void serialize(char* buffer)
        {
            std::memset(buffer, CLEAN_CHAR, sizeof(IndividualTokenValidation));

            std::memcpy(buffer, &type, sizeof(type));
            std::memcpy(buffer + sizeof(type), id, sizeof(id));
            std::memcpy(buffer + sizeof(type) + sizeof(id), &nonce, sizeof(nonce));
            std::memcpy(buffer + sizeof(type) + sizeof(id) + sizeof(nonce),
                        token,
                        sizeof(token));
        }

        size_t packetSize() const
        {
            return sizeof(type) + sizeof(id) + sizeof(nonce) + sizeof(token);
        }
} __attribute__((packed));

/* [4]

    0       2               14              18                    82  83
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+---+
    | 4     | ID            | nonce         | token               | s |
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+---+
*/
class IndividualTokenStatus
{
    public:
        uint16_t type;
        char     id[12];
        uint32_t nonce;
        char     token[64];
        char     status;

        IndividualTokenStatus()
            : type(toNetworkShort(4)),
              nonce(toNetworkLong(0)),
              status(CLEAN_CHAR)
        {
            std::memset(id, CLEAN_CHAR, sizeof(id));
            std::memset(token, CLEAN_CHAR, sizeof(token));
        }

        IndividualTokenStatus(std::string id,
                              uint32_t    nonce,
                              std::string token,
                              char        status)
            : type(toNetworkShort(4)),
              nonce(toNetworkLong(nonce)),
              status(status)
        {
            std::memset(this->id, CLEAN_CHAR, sizeof(this->id));
            std::memset(this->token, CLEAN_CHAR, sizeof(this->token));

            std::memcpy(this->id, id.c_str(), std::min(id.size(), sizeof(this->id)));
            std::memcpy(this->token,
                        token.c_str(),
                        std::min(token.size(), sizeof(this->token)));
        }

        friend std::ostream& operator<<(std::ostream&                os,
                                        const IndividualTokenStatus& status)
        {
            os << static_cast<int>(status.status);
            return os;
        }

} __attribute__((packed));

class SAS
{
    public:
        char     id[12];
        uint32_t nonce;
        char     token[64];

        SAS(std::string sas)
        {
            std::stringstream ss(sas);
            std::string       idStr, nonceStr, tokenStr;

            if (!std::getline(ss, idStr, ':') || !std::getline(ss, nonceStr, ':') ||
                !std::getline(ss, tokenStr))
            {
                throw std::invalid_argument("Invalid SAS format");
            }

            std::memset(id, CLEAN_CHAR, sizeof(id));
            std::memset(token, CLEAN_CHAR, sizeof(token));

            std::memcpy(id, idStr.c_str(), std::min(idStr.size(), sizeof(id)));

            nonce = toNetworkLong(static_cast<uint32_t>(std::stoul(nonceStr)));

            std::memcpy(token,
                        tokenStr.c_str(),
                        std::min(tokenStr.size(), sizeof(token)));
        }

        void serialize(char* buffer, size_t offset = 0)
        {
            std::memcpy(buffer + offset, id, sizeof(id)); // Copia o id
            std::memcpy(buffer + offset + sizeof(id),
                        &nonce,
                        sizeof(nonce)); // Copia o nonce
            std::memcpy(buffer + offset + sizeof(id) + sizeof(nonce),
                        token,
                        sizeof(token)); // Copia o token
        }
} __attribute__((packed));

class BaseGroupToken
{
    public:
        uint16_t type;
        uint16_t n;
        char*    sas;

        BaseGroupToken(uint16_t type, uint16_t n)
            : type(toNetworkShort(type)),
              n(toNetworkShort(n))
        {
            sas = static_cast<char*>(std::malloc(80 * n));

            if (!sas)
                throw std::bad_alloc();

            std::memset(sas, CLEAN_CHAR, 80 * n);
        }

        ~BaseGroupToken()
        {
            std::free(sas);
        }
} __attribute__((packed));

/* [5]

    0       2       4          84         164       4+80N
    +---+---+---+---+--/     /--+--/     /--+--/     /--+
    | 5     | N     | SAS-1     | SAS-2     | SAS-N     |
    +---+---+---+---+--/     /--+--/     /--+--/     /--+
*/
class GroupTokenRequest
{
    public:
        uint16_t type;
        uint16_t n;
        char*    sas;

        GroupTokenRequest(uint16_t type, uint16_t n)
            : type(toNetworkShort(type)),
              n(toNetworkShort(n))
        {
            sas = static_cast<char*>(std::malloc(80 * n));

            if (!sas)
                throw std::bad_alloc();

            std::memset(sas, CLEAN_CHAR, 80 * n);
        }

        ~GroupTokenRequest()
        {
            std::free(sas);
        }

        GroupTokenRequest(std::vector<SAS>& gas)
            : type(toNetworkShort(5)),
              n(toNetworkShort(gas.size()))
        {
            sas = static_cast<char*>(std::malloc(80 * n));

            if (!sas)
                throw std::bad_alloc();

            std::memset(sas, CLEAN_CHAR, 80 * n);

            for (uint16_t i = 0; i < gas.size(); i++)
            {
                gas[i].serialize(sas, i * 80);
            }
        }

        size_t packetSize() const
        {
            return sizeof(type) + sizeof(n) + 80 * fromNetworkShort(n);
        }

        void serialize(char* buffer, size_t offset = 0)
        {
            std::memset(buffer, CLEAN_CHAR, packetSize());
            std::memcpy(buffer + offset, &type, sizeof(type));
            std::memcpy(buffer + offset + sizeof(type), &n, sizeof(n));
            std::memcpy(buffer + offset + sizeof(type) + sizeof(n),
                        sas,
                        80 * fromNetworkShort(n));
        }
} __attribute__((packed));

/* [6]

    0       2       4          84         164       4+80N         4+80N+64
    +---+---+---+---+--/    /--+--/     /--+--/     /--+--/   /--+
    | 6     | N     | SAS-1    | SAS-2     | SAS-N     | token   |
    +---+---+---+---+--/    /--+--/     /--+--/     /--+--/   /---
*/
// class GroupTokenResponse

/* [7]

    0       2       4          84         164       4+80N         68+80N
    +---+---+---+---+--/     /--+--/     /--+--/     /--+--/   /--+
    | 7     | N     | SAS-1     | SAS-2     | SAS-N     | token   |
    +---+---+---+---+--/     /--+--/     /--+--/     /--+--/   /--+
*/
class GroupTokenValidation
{
    public:
        uint16_t type;
        uint16_t n;
        char*    sas;
        char token[64];

        GroupTokenValidation(std::vector<SAS>& gas, std::string token)
            : type(toNetworkShort(7)),
              n(toNetworkShort(gas.size()))
        {
            sas = static_cast<char*>(std::malloc(80 * n));

            if (!sas)
                throw std::bad_alloc();

            std::memset(sas, CLEAN_CHAR, 80 * n);

            std::memset(this->token, CLEAN_CHAR, sizeof(this->token));
            std::memcpy(this->token,
                        token.c_str(),
                        std::min(token.size(), sizeof(this->token)));

            for (uint16_t i = 0; i < gas.size(); i++)
            {
                gas[i].serialize(this->sas, i * 80);
            }
        }

        ~GroupTokenValidation()
        {
            std::free(sas);
        }

        size_t packetSize() const
        {
            return sizeof(type) + sizeof(n) + 80 * fromNetworkShort(n) + sizeof(token);
        }

        void serialize(char* buffer, size_t offset = 0)
        {
            std::memcpy(buffer + offset, &type, sizeof(type));
            std::memcpy(buffer + offset + sizeof(type), &n, sizeof(n));
            std::memcpy(buffer + offset + sizeof(type) + sizeof(n),
                        sas,
                        80 * fromNetworkShort(n));
            std::memcpy(buffer + offset + sizeof(type) + sizeof(n) +
                            80 * fromNetworkShort(n),
                        token,
                        sizeof(token));
        }
} __attribute__((packed));

/* [8]

    0       2       4          84         164       4+80N    68+80N   69+80N
    +---+---+---+---+--/     /--+--/     /--+--/     /--+--/   /--+---+
    | 8     | N     | SAA-1     | SAA-2     | SAA-N     | token   | s |
    +---+---+---+---+--/     /--+--/     /--+--/     /--+--/   /--+---|
*/
//class GroupTokenStatus 

/* [9]

    0         2         4
    +----+----+----+----+
    | 256     | error   |
    +----+----+----+----+
*/
class ErrorResponse
{
    public:
        uint16_t type;
        uint16_t error;

        ErrorResponse(uint16_t error)
            : type(toNetworkShort(256)),
              error(toNetworkShort(error))
        { }

        friend std::ostream& operator<<(std::ostream&        os,
                                        const ErrorResponse& error_response)
        {
            os << "Error: "
               << getErrorDescription(fromNetworkShort(error_response.error));
            return os;
        }
} __attribute__((packed));

IndividualTokenValidation parseIndividualTokenValidationFromString(const char* sas)
{
    if (!isValidAscii(sas))
    {
        std::cerr << "SAS contém caracteres não-ASCII!" << std::endl;
    }

    std::stringstream ss(sas);
    std::string       id, token;
    uint32_t          nonce;

    // Split the input string by :
    std::getline(ss, id, ':');
    std::string nonceStr;
    std::getline(ss, nonceStr, ':');
    nonce = static_cast<uint32_t>(std::stoul(nonceStr));
    std::getline(ss, token, ':');

    return IndividualTokenValidation(id, nonce, token);
}

GroupTokenValidation parseGroupTokenValidationFromString(const char* allSas)
{
    if (!isValidAscii(allSas))
    {
        std::cerr << "SAS contém caracteres não-ASCII!" << std::endl;
    }

    std::stringstream ss(allSas);

    std::string      sas;
    std::string      token;
    std::vector<SAS> sasList;

    while (std::getline(ss, sas, '+'))
    {
        try
        {
            sasList.emplace_back(sas);
        }
        catch (const std::invalid_argument& e)
        {
            // if is not a valid SAS, then it is the token
            token = sas;
        }
    }

    return GroupTokenValidation(sasList, token);
}

std::string getGroupTokenResponse(const char* buffer, GroupTokenRequest& request)
{
    char token[64];

    size_t offset =
        sizeof(uint16_t) + sizeof(uint16_t) + 80 * fromNetworkShort(request.n);

    std::memset(token, CLEAN_CHAR, sizeof(token));
    std::memcpy(token, buffer + offset, sizeof(token));

    return std::string(token, sizeof(token));
}

int getGroupTokenStatus(const char* buffer, GroupTokenValidation& gtv)
{
    char status;

    // Interpretando os dados do buffer
    size_t offset =
        sizeof(uint16_t) + sizeof(uint16_t) + 80 * fromNetworkShort(gtv.n) + 64;
    std::memcpy(&status, buffer + offset, sizeof(status));

    return static_cast<int>(status);
}

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
    char*             serializedRequest = new char[request.packetSize()];
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
    delete[] serializedRequest;
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

    delete[] serializedValidation;
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
