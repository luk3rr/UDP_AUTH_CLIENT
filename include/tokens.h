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