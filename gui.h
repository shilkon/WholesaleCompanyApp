#ifndef GUI_H
#define GUI_H

#include "account.h"
#include "database.h"

#include <imgui.h>

namespace Gui
{
    namespace WCSys = wholesaleCompanySystem;
    namespace DB = database;

    inline constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable;


    bool logInWindow(pqxx::connection& connection, WCSys::UAccount& account);


    void menuBar(WCSys::Account account, bool& showUsersWindow, bool& showReportWindow);

    void usersWindow(const WCSys::Account& account, bool& showUsersWindow);
    void createUserTab(const WCSys::Account& account);
    void changeUsersTab(const WCSys::Account& account);

    void reportWindow(const DB::Database& db, bool& showReportWindow);


    void DBGoodsInteraction(DB::GoodsTable& goods, const WCSys::Account& account);
    void DBGoodsCreateTab(DB::GoodsTable& goods);
    void DBGoodsModifyTab(DB::GoodsTable& goods);
    void DBGoodsDeleteTab(DB::GoodsTable& goods);

    void DBSalesInteraction(DB::SalesTable& sales, DB::WarehouseTable& wh1, DB::WarehouseTable& wh2, const WCSys::Account& account);
    void DBSalesCreateTab(DB::SalesTable& sales, DB::WarehouseTable& wh1, DB::WarehouseTable& wh2);
    void DBSalesModifyTab(DB::SalesTable& sales);
    void DBSalesDeleteTab(DB::SalesTable& sales);

    void DBWarehouseInteraction(DB::WarehouseTable& warehouse1, DB::WarehouseTable& warehouse2, const WCSys::Account& account);
    void DBWarehouseCreateTab(DB::WarehouseTable& warehouse1, DB::WarehouseTable& warehouse2);
    void DBWarehouseModifyTab(DB::WarehouseTable& warehouse1, DB::WarehouseTable& warehouse2);
    void DBWarehouseDeleteTab(DB::WarehouseTable& warehouse1, DB::WarehouseTable& warehouse2);

    void showActionResult(std::string_view result);


    void databaseLog(const DB::Database& db);

    void goodsDataTable(DB::GoodsTable& goods);
    void sortGoodsDataTable(std::vector<std::tuple<int, std::string, int>>& data, const ImGuiTableSortSpecs* sortSpecs);

    void salesDataTable(DB::SalesTable& sales);
    void sortSalesDataTable(std::vector<std::tuple<int, int, int, std::string>>& data, const ImGuiTableSortSpecs* sortSpecs);

    void warehouseDataTable(DB::WarehouseTable& warehouse);
    void sortWarehouseDataTable(std::vector<std::tuple<int, int, int>>& data, const ImGuiTableSortSpecs* sortSpecs);

    template <typename T, typename Cmp>
    void sortTableData(std::vector<T>& data, Cmp cmp, ImGuiSortDirection direction)
    {
        if (direction == ImGuiSortDirection_Ascending)
            std::sort(data.begin(), data.end(), cmp);
        else
            std::sort(data.rbegin(), data.rend(), cmp);
    }


    void accountInfo(WCSys::Account& account, bool& loggedIn);

    void popularGoodsDataTable(DB::PopularGoodsOfMonthView& popularGoods);

    void productDemandInfo(pqxx::connection& con);
    void productDemandPlot(const DB::ProductDemandPlotData& data);
    void productDemandDataTable(const std::vector<std::tuple<int, std::string, int>>& data);
}

#endif
