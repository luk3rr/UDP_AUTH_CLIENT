#include "utils.h"
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <vector>

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

        friend std::ostream& operator<<(std::ostream&                 os,
                                        const IndividualTokenRequest& request)
        {
            os << "ID: " << request.id << "\n"
               << "Nonce: " << fromNetworkLong(request.nonce);
            return os;
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

        friend std::ostream& operator<<(std::ostream&                    os,
                                        const IndividualTokenValidation& validation)
        {
            os << removeSpaces(validation.id) << ":"
               << fromNetworkLong(validation.nonce) << ":"
               << std::string(validation.token, sizeof(validation.token));
            return os;
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
            os << removeSpaces(status.id) << ":" << fromNetworkLong(status.nonce) << ":"
               << std::string(status.token, sizeof(status.token)) << "\n"
               << "Status: " << static_cast<int>(status.status);
            return os;
        }

} __attribute__((packed));

class SAS
{
    public:
        char     id[12];
        uint32_t nonce;
        char     token[64];

        SAS(const char* buffer)
        {
            std::memset(id, CLEAN_CHAR, sizeof(id));
            std::memcpy(id, buffer, sizeof(id));

            std::memcpy(&nonce, buffer + sizeof(id), sizeof(nonce));
            nonce = fromNetworkLong(nonce); // Se necessário, converte de rede para host

            std::memset(token, CLEAN_CHAR, sizeof(token));
            std::memcpy(token,
                        buffer + sizeof(id) + sizeof(nonce),
                        sizeof(token)); // Copia o token
        }

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
            std::memcpy(id, idStr.c_str(), std::min(idStr.size(), sizeof(id)));

            nonce = toNetworkLong(static_cast<uint32_t>(std::stoul(nonceStr)));

            std::memset(token, CLEAN_CHAR, sizeof(token));
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

        friend std::ostream& operator<<(std::ostream& os, const SAS& sas)
        {
            os << removeSpaces(sas.id) << ":" << fromNetworkLong(sas.nonce) << ":"
               << std::string(sas.token, sizeof(sas.token));
            return os;
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
class GroupTokenRequest : public BaseGroupToken
{
    public:
        GroupTokenRequest(std::vector<SAS>& gas)
            : BaseGroupToken(5, gas.size())
        {
            for (uint16_t i = 0; i < gas.size(); i++)
            {
                gas[i].serialize(sas, i * 80);
            }
        }

        friend std::ostream& operator<<(std::ostream&            os,
                                        const GroupTokenRequest& request)
        {
            os << titleOutput("GroupTokenRequest") << "\n";
            os << "\tType: " << fromNetworkShort(request.type) << "\n";
            os << "\tN: " << fromNetworkShort(request.n) << "\n";

            for (size_t i = 0; i < fromNetworkShort(request.n); i++)
            {
                os << "\tSAS-" << i + 1 << ": ";
                for (size_t j = 0; j < 80; j++)
                {
                    os << std::hex << std::setw(2) << std::setfill('0')
                       << (unsigned int)(unsigned char)request.sas[i * 80 + j] << " ";
                }
                os << std::dec << std::endl;
            }
            os << "\tPacket size: " << request.packetSize() << " bytes" << std::endl;

            return os;
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
class GroupTokenResponse : public BaseGroupToken
{
    public:
        char token[64];

        GroupTokenResponse(std::vector<SAS>& gas, std::string token)
            : BaseGroupToken(6, gas.size())
        {
            std::memset(this->token, CLEAN_CHAR, sizeof(this->token));
            std::memcpy(this->token,
                        token.c_str(),
                        std::min(token.size(), sizeof(this->token)));

            for (uint16_t i = 0; i < gas.size(); i++)
            {
                std::cout << "Serializando SAS " << i << ": " << gas[i] << std::endl;

                gas[i].serialize(this->sas, i * 80);
            }
        }

        friend std::ostream& operator<<(std::ostream&             os,
                                        const GroupTokenResponse& response)
        {

            os << titleOutput("GroupTokenResponse") << "\n";

            os << "\tType: " << fromNetworkShort(response.type) << "\n";
            os << "\tN: " << fromNetworkShort(response.n) << "\n";
            os << "\tToken: " << std::string(response.token, sizeof(response.token));

            return os;
        }
} __attribute__((packed));

/* [7]

    0       2       4          84         164       4+80N         68+80N
    +---+---+---+---+--/     /--+--/     /--+--/     /--+--/   /--+
    | 7     | N     | SAS-1     | SAS-2     | SAS-N     | token   |
    +---+---+---+---+--/     /--+--/     /--+--/     /--+--/   /--+
*/
class GroupTokenValidation : public BaseGroupToken
{
    public:
        char token[64];

        GroupTokenValidation(std::vector<SAS>& gas, std::string token)
            : BaseGroupToken(7, gas.size())
        {
            std::memset(this->token, CLEAN_CHAR, sizeof(this->token));
            std::memcpy(this->token,
                        token.c_str(),
                        std::min(token.size(), sizeof(this->token)));

            for (uint16_t i = 0; i < gas.size(); i++)
            {
                gas[i].serialize(this->sas, i * 80);
            }
        }

        friend std::ostream& operator<<(std::ostream&               os,
                                        const GroupTokenValidation& validation)
        {
            os << titleOutput("GroupTokenValidation") << "\n";

            os << "\tType: " << fromNetworkShort(validation.type) << "\n";
            os << "\tN: " << fromNetworkShort(validation.n) << "\n";
            os << "\tToken: "
               << std::string(validation.token, sizeof(validation.token));

            return os;
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
class GroupTokenStatus : public BaseGroupToken
{
    public:
        char token[64];
        char status;

        GroupTokenStatus(std::vector<SAS>& gas, std::string token, char status)
            : BaseGroupToken(8, gas.size()),
              status(status)
        {
            std::memset(this->token, CLEAN_CHAR, sizeof(this->token));
            std::memcpy(this->token,
                        token.c_str(),
                        std::min(token.size(), sizeof(this->token)));

            for (uint16_t i = 0; i < gas.size(); i++)
            {
                gas[i].serialize(this->sas, i * 80);
            }
        }

        friend std::ostream& operator<<(std::ostream&           os,
                                        const GroupTokenStatus& status)
        {
            os << "N: " << fromNetworkShort(status.n) << "\n";
            os << "SAS: " << std::string(status.sas, 80 * status.n) << "\n";
            os << "Token: " << std::string(status.token, sizeof(status.token)) << "\n"
               << "Status: " << static_cast<int>(status.status);
            return os;
        }
} __attribute__((packed));

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

GroupTokenResponse
parseGroupTokenResponse(const char* buffer, size_t bufferSize, uint16_t numSas)
{
    uint16_t type;
    uint16_t n;
    char     sas[80 * numSas];
    char     token[64];

    // Interpretando os dados do buffer
    size_t offset = 0;

    std::memcpy(&type, buffer + offset, sizeof(type));
    type = fromNetworkShort(type);
    offset += sizeof(type);


    std::memcpy(&n, buffer + offset, sizeof(n));
    n = fromNetworkShort(n); // Conversão de rede para host
    offset += sizeof(n);

    // Preencher SAS (80 * N bytes)
    std::memcpy(sas, buffer + offset, 80 * numSas);
    offset += 80 * numSas;

    std::memcpy(token, buffer + offset, sizeof(token));

    // Criar GroupTokenResponse com os dados extraídos
    std::vector<SAS> sasVector;
    for (size_t i = 0; i < numSas; ++i)
    {
        SAS sasItem(sas + i * 80);
        sasVector.push_back(sasItem);
    }

    // Criar GroupTokenResponse
    std::string tokenStr(token, sizeof(token));
    return GroupTokenResponse(sasVector, tokenStr);
}

GroupTokenStatus
parseGroupTokenStatus(const char* buffer, size_t bufferSize, uint16_t numSas)
{
    std::cout << "PARSE" << std::endl;
    uint16_t type;
    uint16_t n;
    char     sas[80 * numSas];
    char     token[64];
    char status;

    // Interpretando os dados do buffer
    size_t offset = 0;

    std::memcpy(&type, buffer + offset, sizeof(type));
    type = fromNetworkShort(type);
    offset += sizeof(type);

    std::memcpy(&n, buffer + offset, sizeof(n));
    n = fromNetworkShort(n); // Conversão de rede para host
    offset += sizeof(n);

    // Preencher SAS (80 * N bytes)
    std::memcpy(sas, buffer + offset, 80 * n);
    offset += 80 * n;

    std::memcpy(token, buffer + offset, sizeof(token));
    offset += sizeof(token);

    std::memcpy(&status, buffer + offset, sizeof(status));

    // Criar GroupTokenResponse com os dados extraídos
    std::vector<SAS> sasVector;
    for (size_t i = 0; i < n; ++i)
    {
        std::cout << "HERE" << std::endl;
        SAS sasItem(sas + i * 80);
        std::cout << "SHOW" << std::endl;
        sasVector.push_back(sasItem);
    }

    // Criar GroupTokenResponse
    std::string tokenStr(token, sizeof(token));
    return GroupTokenStatus(sasVector, tokenStr, status);
}