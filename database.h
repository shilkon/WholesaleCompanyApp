#ifndef DATABASE_H
#define DATABASE_H

#include <pqxx/pqxx>

#include "table.h"
#include "queries.h"

#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <iostream>
#include <fstream>

namespace database
{
    struct Database
    {
        UGoodsTable goods{};
        USalesTable sales{};
        UWarehouseTable warehouse1{};
        UWarehouseTable warehouse2{};
        UPopularGoodsOfMonthView popularGoods{};
    };

    using UDatabase = std::unique_ptr<Database>;

    using ProductDemandPlotData = std::pair<std::vector<int>, std::vector<int>>;

    std::vector<std::tuple<int, std::string, int>> getProductDemand(pqxx::connection& con,
        std::string_view firstDate, std::string_view secondDate, int productId);

    ProductDemandPlotData getProductDemandPlotData(const std::vector<std::tuple<int, std::string, int>>& data);

    template <int i = 0, typename... T>
    void writeDataToReport(std::ofstream& out, const std::tuple<T...>& row)
    {
        if constexpr (i == sizeof...(T))
            return;
        else
        {
            out << std::get<i>(row) << ';';
            writeDataToReport<i + 1>(out, row);
        }
    }

    template <typename T>
    void createReport(const std::vector<T>& data)
    {
        std::ofstream reportFile;
        reportFile.open("report.csv");
        for (const auto& row : data)
        {
            writeDataToReport(reportFile, row);
            reportFile << '\n';
        }
        reportFile.close();
    }
}

#endif
