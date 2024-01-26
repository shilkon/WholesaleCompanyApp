#include "database.h"

#include <pqxx/pqxx>

namespace DB = database;

std::vector<std::tuple<int, std::string, int>> DB::getProductDemand(pqxx::connection& con,
    std::string_view firstDate, std::string_view secondDate, int productId)
{
    std::vector<std::tuple<int, std::string, int>> data{};

    pqxx::work tx{ con };
    try
    {
        for (const auto& row : tx.exec_params(std::string(sales::PRODUCT_DEMAND_QUERY),
            std::string(firstDate), std::string(secondDate), productId))
            data.push_back(row.as<int, std::string, int>());
    }
    catch (const pqxx::sql_error& e)
    {
        data.clear();
        data.shrink_to_fit();
    }
    tx.commit();

    return data;
}

DB::ProductDemandPlotData DB::getProductDemandPlotData(const std::vector<std::tuple<int, std::string, int>>& data)
{
    ProductDemandPlotData plotData{};
    for (const auto&[day, date, demand] : data)
    {
        plotData.first.push_back(day);
        plotData.second.push_back(demand);
    }

    return plotData;
}
