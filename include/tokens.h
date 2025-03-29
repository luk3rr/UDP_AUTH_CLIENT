#include "utils.h"
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <vector>

class BaseIndividualToken
{
    public:
        uint16_t type;
        char     id[12];
        uint32_t nonce;

        BaseIndividualToken(uint16_t type, const char* id, uint32_t nonce)
            : type(toNetworkShort(type)),
              nonce(toNetworkLong(nonce))
        {
            std::memcpy(this->id, id, sizeof(this->id));
        }
} __attribute__((packed));

/* [1]

    0         2                        14                  18
    +----+----+----+----/    /----+----+----+----+----+----+
    | 1       | ID                     | nonce             |
    +----+----+----+----/    /----+----+----+----+----+----+
*/
class IndividualTokenRequest : public BaseIndividualToken
{
    public:
        IndividualTokenRequest(const char* id, uint32_t nonce)
            : BaseIndividualToken(1, id, nonce)
        { }

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
class IndividualTokenResponse : public BaseIndividualToken
{
    public:
        char token[64];

        IndividualTokenResponse(const char* id, uint32_t nonce, const char* token)
            : BaseIndividualToken(2, id, nonce)
        {
            std::memset(this->token, 0, sizeof(this->token));
            std::memcpy(this->token, token, sizeof(this->token));
        }

        friend std::ostream& operator<<(std::ostream&                  os,
                                        const IndividualTokenResponse& response)
        {
            os << response.id << ":" << fromNetworkLong(response.nonce) << ":"
               << std::string(response.token, sizeof(response.token));
            return os;
        }
} __attribute__((packed));

/* [3]

    0       2               14              18                    82
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+
    | 3     | ID            | nonce         | token               |
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+
*/
class IndividualTokenValidation : public BaseIndividualToken
{
    public:
        char token[64];

        IndividualTokenValidation(const char* id, uint32_t nonce, const char* token)
            : BaseIndividualToken(3, id, nonce)
        {
            std::memset(this->token, 0, sizeof(this->token));
            std::memcpy(this->token, token, sizeof(this->token));
        }

        friend std::ostream& operator<<(std::ostream&                    os,
                                        const IndividualTokenValidation& validation)
        {
            os << validation.id << ":" << fromNetworkLong(validation.nonce) << ":"
               << std::string(validation.token, sizeof(validation.token));
            return os;
        }
} __attribute__((packed));

/* [4]

    0       2               14              18                    82  83
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+---+
    | 4     | ID            | nonce         | token               | s |
    +---+---+---+---/   /---+---+---+---+---+---+---/         /---+---+
*/
class IndividualTokenStatus : public BaseIndividualToken
{
    public:
        char token[64];
        char status;

        IndividualTokenStatus(const char* id,
                              uint32_t    nonce,
                              const char* token,
                              char        status)
            : BaseIndividualToken(4, id, nonce),
              status(status)
        {
            std::memset(this->token, 0, sizeof(this->token));
            std::memcpy(this->token, token, sizeof(this->token));
        }

        friend std::ostream& operator<<(std::ostream&                os,
                                        const IndividualTokenStatus& status)
        {
            os << status.id << ":" << fromNetworkLong(status.nonce) << ":"
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

        SAS(const char* sas)
        {
            std::stringstream ss(sas);
            std::string       idStr, nonceStr, tokenStr;

            if (!std::getline(ss, idStr, ':') || !std::getline(ss, nonceStr, ':') ||
                !std::getline(ss, tokenStr))
            {
                throw std::invalid_argument("Invalid SAS format");
            }

            std::memset(id, 0, sizeof(id));
            std::memcpy(id, idStr.c_str(), std::min(sizeof(id), idStr.size()));

            nonce = toNetworkLong(static_cast<uint32_t>(std::stoul(nonceStr)));

            std::memset(token, 0, sizeof(token));
            std::memcpy(token,
                        tokenStr.c_str(),
                        std::min(sizeof(token), tokenStr.size()));

            std::cout << titleOutput("SAS") << std::endl;

            std::cout << "\tid: " << id << " || size: " << sizeof(id) << "\n"
                      << "\tnonce: " << fromNetworkLong(nonce)
                      << " || size: " << sizeof(nonce) << "\n"
                      << "\ttoken: " << std::string(token, sizeof(token))
                      << " || size: " << sizeof(token) << std::endl;
        }

        void serialize(char* buffer, size_t offset = 0, bool debug = false)
        {
            std::memcpy(buffer + offset, id, sizeof(id)); // Copia o id
            std::memcpy(buffer + offset + sizeof(id),
                        &nonce,
                        sizeof(nonce)); // Copia o nonce
            std::memcpy(buffer + offset + sizeof(id) + sizeof(nonce),
                        token,
                        sizeof(token)); // Copia o token

            if (debug)
            {
                std::cout << "Serialized SAS: ";
                for (size_t i = 0; i < sizeof(id) + sizeof(nonce) + sizeof(token); i++)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                              << (unsigned int)(unsigned char)buffer[offset + i] << " ";
                }
                std::cout << std::dec << std::endl;
            }
        }

        friend std::ostream& operator<<(std::ostream& os, const SAS& sas)
        {
            os << sas.id << ":" << fromNetworkLong(sas.nonce) << ":"
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

            std::memset(sas, 0, 80 * n);
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

        GroupTokenResponse(std::vector<SAS>& gas, const char* token)
            : BaseGroupToken(6, gas.size())
        {
            std::memset(this->token, 0, sizeof(this->token));
            std::memcpy(this->token, token, sizeof(this->token));

            for (uint16_t i = 0; i < gas.size(); i++)
            {
                gas[i].serialize(this->sas, i * 80);
            }
        }

        friend std::ostream& operator<<(std::ostream&             os,
                                        const GroupTokenResponse& response)
        {

            os << titleOutput("GroupTokenResponse") << "\n";

            os << "\tType: " << fromNetworkShort(response.type) << "\n";
            os << "\tN: " << fromNetworkShort(response.n) << "\n";
            os << "\tToken: " << std::string(response.token, sizeof(response.token))
               << "\n";

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

        GroupTokenValidation(std::vector<SAS>& gas, const char* token)
            : BaseGroupToken(7, gas.size())
        {
            std::memset(this->token, 0, sizeof(this->token));
            std::memcpy(this->token, token, sizeof(this->token));

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
            os << "\tToken: " << std::string(validation.token, sizeof(validation.token))
               << "\n";

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

        GroupTokenStatus(std::vector<SAS>& gas, const char* token, char status)
            : BaseGroupToken(8, gas.size()),
              status(status)
        {
            std::memset(this->token, 0, sizeof(this->token));
            std::memcpy(this->token, token, sizeof(this->token));

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

    return IndividualTokenValidation(id.c_str(), nonce, token.c_str());
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
            sasList.emplace_back(sas.c_str());
        }
        catch (const std::invalid_argument& e)
        {
            // if is not a valid SAS, then it is the token
            token = sas;
        }
    }

    return GroupTokenValidation(sasList, token.c_str());
}
