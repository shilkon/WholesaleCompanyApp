#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "database.h"

#include <pqxx/pqxx>

#include <string>
#include <memory>

namespace wholesaleCompanySystem
{
    namespace DB = database;

    namespace permissions
    {
        inline constexpr int none{ 0 };
        inline constexpr int modifyGoods{ 1 << 0 };
        inline constexpr int modifySales{ 1 << 1 };
        inline constexpr int modifyWarehouses{ 1 << 2 };
        inline constexpr int all{ 1 << 3 };
    }

    int getPermissionsFromArray(const std::array<bool, 3>& permissions);
    std::array<bool, 3> getArrayFromPermissions(int permissions);

    class Account
    {
    public:
        Account(pqxx::connection& con, std::string_view login, int permissions);

        std::string_view getLogin() const;

        void updatePassword(std::string_view password) const;
        void createAccount(std::string_view login, std::string_view password, int permissions) const;
        int getUserPermissions(std::string_view login) const;
        void changeAccountPermissions(std::string_view login, int permissions) const;

        bool satisfyPermission(int permission) const;

        static Account logIn(pqxx::connection& con, std::string_view login, std::string_view password);

    private:
        pqxx::connection* con_;
        std::string login_;
        int permissions_;
    };

    using UAccount = std::unique_ptr<Account>;

    class BadCredentials : public std::exception
    {
    public:
        explicit BadCredentials() :
            std::exception{ "Invalid login or password!" }
        {}
    };
}

#endif
