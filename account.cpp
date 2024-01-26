#include "account.h"

#include "queries.h"

#include <pqxx/pqxx>

#include <exception> 

namespace WCSys = wholesaleCompanySystem;

int WCSys::getPermissionsFromArray(const std::array<bool, 3>& permissions)
{
    int result{};
    for (int i{}; i < 3; ++i)
        result += permissions[i] ? 1 << i : 0;
    return result;
}

std::array<bool, 3> WCSys::getArrayFromPermissions(int permissions)
{
    std::array<bool, 3> result{};
    for (int i{}; i < 3; ++i)
        result[i] = (1 << i) & permissions;
    return result;
}

WCSys::Account::Account(pqxx::connection& con, std::string_view login, int permissions):
    con_(&con),
    login_(login),
    permissions_(permissions)
{}

std::string_view WCSys::Account::getLogin() const
{
    return login_;
}

void WCSys::Account::updatePassword(std::string_view password) const
{
    pqxx::work tx{ *con_ };
    tx.exec_params0(std::string(DB::accounts::UPDATE_PASSWORD_QUERY), login_, password);
    tx.commit();
}

void WCSys::Account::createAccount(std::string_view login, std::string_view password, int permissions) const
{
    pqxx::work tx{ *con_ };
    tx.exec_params0(std::string(DB::accounts::CREATE_ACCOUNT_QUERY), login, password, permissions);
    tx.commit();
}

int WCSys::Account::getUserPermissions(std::string_view login) const
{
    pqxx::work tx{ *con_ };
    auto result{ tx.exec_params(std::string(DB::accounts::GET_PERMISSIONS_QUERY), login) };
    tx.commit();

    return result.size() == 1 ? result.front().front().as<int>() : -1;
}

void WCSys::Account::changeAccountPermissions(std::string_view login, int permissions) const
{
    pqxx::work tx{ *con_ };
    tx.exec_params0(std::string(DB::accounts::UPDATE_PERMISSIONS_QUERY), login, permissions);
    tx.commit();
}

bool WCSys::Account::satisfyPermission(int permission) const
{
    return permissions_ & permission;
}

WCSys::Account WCSys::Account::logIn(pqxx::connection& con, std::string_view login, std::string_view password)
{
    std::tuple<bool, int> logInResult{};

    pqxx::work tx{ con };
    auto result{ tx.exec_params(std::string(DB::accounts::LOG_IN_QUERY), login, password) };
    tx.commit();

    if (result.size() == 1)
        result.front().to(logInResult);

    if (result.size() != 1 || !std::get<0>(logInResult))
        throw BadCredentials{};

    return Account{ con, login, std::get<1>(logInResult) };
}
