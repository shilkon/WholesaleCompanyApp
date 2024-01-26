#include "queries.h"

namespace DB = database;

DB::TableQueries DB::getQueries(TableType table)
{
    TableQueries queries{};

    switch (table)
    {
    case TableType::goods:
        queries.tableName = std::string(goods::TABLE_NAME);
        queries.updateQuery = std::string(goods::UPDATE_QUERY);
        queries.createQuery = std::string(goods::CREATE_QUERY);
        queries.modifyQuery = std::string(goods::MODIFY_QUERY);
        queries.deleteByIdQuery = std::string(goods::DELETE_BY_ID_QUERY);
        queries.selectByColumnValueQueries = std::vector{ std::string(goods::SELECT_BY_ID_QUERY) };
        break;
    case TableType::sales:
        queries.tableName = std::string(sales::TABLE_NAME);
        queries.updateQuery = std::string(sales::UPDATE_QUERY);
        queries.createQuery = std::string(sales::CREATE_QUERY);
        queries.modifyQuery = std::string(sales::MODIFY_QUERY);
        queries.deleteByIdQuery = std::string(sales::DELETE_BY_ID_QUERY);
        queries.selectByColumnValueQueries = std::vector{ std::string(sales::SELECT_BY_ID_QUERY) };
        break;
    case TableType::warehouse1:
        queries.tableName = std::string(warehouse1::TABLE_NAME);
        queries.updateQuery = std::string(warehouse1::UPDATE_QUERY);
        queries.createQuery = std::string(warehouse1::CREATE_QUERY);
        queries.modifyQuery = std::string(warehouse1::MODIFY_QUERY);
        queries.deleteByIdQuery = std::string(warehouse1::DELETE_BY_ID_QUERY);
        queries.selectByColumnValueQueries = std::vector{
            std::string(warehouse1::SELECT_BY_ID_QUERY), std::string(warehouse1::SELECT_BY_GOOD_ID_QUERY)
        };
        break;
    case TableType::warehouse2:
        queries.tableName = std::string(warehouse2::TABLE_NAME);
        queries.updateQuery = std::string(warehouse2::UPDATE_QUERY);
        queries.createQuery = std::string(warehouse2::CREATE_QUERY);
        queries.modifyQuery = std::string(warehouse2::MODIFY_QUERY);
        queries.deleteByIdQuery = std::string(warehouse2::DELETE_BY_ID_QUERY);
        queries.selectByColumnValueQueries = std::vector{
            std::string(warehouse2::SELECT_BY_ID_QUERY), std::string(warehouse2::SELECT_BY_GOOD_ID_QUERY)
        };
        break;
    case TableType::popularGoodsOfTheMonth:
        queries.tableName = std::string(popularGoodsOfTheMonth::TABLE_NAME);
        queries.updateQuery = std::string(popularGoodsOfTheMonth::UPDATE_QUERY);
        queries.selectByColumnValueQueries = std::vector{ std::string(popularGoodsOfTheMonth::SELECT_BY_ID_QUERY) };
        break;
    }

    return queries;
}
