#ifndef QUERIES_H
#define QUERIES_H

#include <string>
#include <vector>

namespace database
{
    enum class TableType
    {
        goods,
        sales,
        warehouse1,
        warehouse2,
        popularGoodsOfTheMonth
    };

    struct TableQueries
    {
        std::string tableName{};
        std::string updateQuery{};
        std::string createQuery{};
        std::string modifyQuery{};
        std::string deleteByIdQuery{};
        std::vector<std::string> selectByColumnValueQueries{};
    };

    TableQueries getQueries(TableType table);

    namespace goods
    {
        inline constexpr std::string_view TABLE_NAME{ "goods" };
        inline constexpr std::string_view UPDATE_QUERY{ "SELECT * FROM goods ORDER BY id" };
        inline constexpr std::string_view SELECT_BY_ID_QUERY{ "SELECT * FROM goods WHERE id = $1" };
        inline constexpr std::string_view CREATE_QUERY{ "INSERT INTO goods(name, priority) VALUES ($1, $2)" };
        inline constexpr std::string_view MODIFY_QUERY{ "UPDATE goods SET name = $2, priority = $3 WHERE id = $1" };
        inline constexpr std::string_view DELETE_BY_ID_QUERY{ "DELETE FROM goods WHERE id = $1" };
    }

    namespace sales
    {
        inline constexpr std::string_view TABLE_NAME{ "sales" };
        inline constexpr std::string_view UPDATE_QUERY{ "SELECT * FROM sales ORDER BY create_date DESC, id" };
        inline constexpr std::string_view SELECT_BY_ID_QUERY{ "SELECT * FROM sales WHERE id = $1" };
        inline constexpr std::string_view CREATE_QUERY{ "INSERT INTO sales(good_id, good_count, create_date) VALUES ($1, $2, $3)" };
        inline constexpr std::string_view MODIFY_QUERY{ "UPDATE sales SET good_id = $2, good_count = $3, create_date = $4 WHERE id = $1" };
        inline constexpr std::string_view DELETE_BY_ID_QUERY{ "DELETE FROM sales WHERE id = $1" };

        inline constexpr std::string_view PRODUCT_DEMAND_QUERY{ "SELECT * FROM ProductDemand($1, $2, $3)" };
    }

    namespace warehouse1
    {
        inline constexpr std::string_view TABLE_NAME{ "warehouse1" };
        inline constexpr std::string_view UPDATE_QUERY{ "SELECT * FROM warehouse1 ORDER BY id" };
        inline constexpr std::string_view SELECT_BY_ID_QUERY{ "SELECT * FROM warehouse1 WHERE id = $1" };
        inline constexpr std::string_view SELECT_BY_GOOD_ID_QUERY{ "SELECT * FROM warehouse1 WHERE good_id = $1 ORDER BY good_count" };
        inline constexpr std::string_view CREATE_QUERY{ "INSERT INTO warehouse1(good_id, good_count) VALUES ($1, $2)" };
        inline constexpr std::string_view MODIFY_QUERY{ "UPDATE warehouse1 SET good_id = $2, good_count = $3 WHERE id = $1" };
        inline constexpr std::string_view DELETE_BY_ID_QUERY{ "DELETE FROM warehouse1 WHERE id = $1" };
    }

    namespace warehouse2
    {
        inline constexpr std::string_view TABLE_NAME{ "warehouse2" };
        inline constexpr std::string_view UPDATE_QUERY{ "SELECT * FROM warehouse2 ORDER BY id" };
        inline constexpr std::string_view SELECT_BY_ID_QUERY{ "SELECT * FROM warehouse2 WHERE id = $1" };
        inline constexpr std::string_view SELECT_BY_GOOD_ID_QUERY{ "SELECT * FROM warehouse2 WHERE good_id = $1 ORDER BY good_count" };
        inline constexpr std::string_view CREATE_QUERY{ "INSERT INTO warehouse2(good_id, good_count) VALUES ($1, $2)" };
        inline constexpr std::string_view MODIFY_QUERY{ "UPDATE warehouse2 SET good_id = $2, good_count = $3 WHERE id = $1" };
        inline constexpr std::string_view DELETE_BY_ID_QUERY{ "DELETE FROM warehouse2 WHERE id = $1" };
    }

    namespace popularGoodsOfTheMonth
    {
        inline constexpr std::string_view TABLE_NAME{ "populargoodsofmonth" };
        inline constexpr std::string_view UPDATE_QUERY{ "SELECT * FROM populargoodsofmonth ORDER BY sales_count DESC" };
        inline constexpr std::string_view SELECT_BY_ID_QUERY{ "SELECT * FROM populargoodsofmonth WHERE id = $1" };
    }

    namespace accounts
    {
        inline constexpr std::string_view LOG_IN_QUERY{ "SELECT (password = crypt($2, password)), permissions FROM accounts WHERE login = $1" };
        inline constexpr std::string_view CREATE_ACCOUNT_QUERY{
            "INSERT INTO accounts(login, password, permissions) VALUES ($1, crypt($2, gen_salt('md5')), $3)"
            "ON CONFLICT(login) DO UPDATE SET password = crypt($2, gen_salt('md5')), permissions = $3"
        };
        inline constexpr std::string_view GET_PERMISSIONS_QUERY{ "SELECT permissions FROM accounts WHERE login = $1" };
        inline constexpr std::string_view UPDATE_PERMISSIONS_QUERY{ "UPDATE accounts SET permissions = $2 WHERE login = $1" };
        inline constexpr std::string_view UPDATE_PASSWORD_QUERY{ "UPDATE accounts SET password = crypt($2, gen_salt('md5')) WHERE login = $1" };
    }
}

#endif
