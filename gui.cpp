#include "gui.h"

#include <imgui.h>
#include <implot.h>
#include <imgui_stdlib.h>

#include <string>
#include <tuple>
#include <array>
#include <algorithm>


void Gui::menuBar(WCSys::Account account, bool& showUsersWindow, bool& showReportWindow)
{
    static bool isAdmin{};
    isAdmin = account.satisfyPermission(WCSys::permissions::all);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu(u8"Меню"))
        {
            if (isAdmin && ImGui::MenuItem(u8"Пользователи"))
                showUsersWindow = true;

            if (ImGui::MenuItem(u8"Отчёты"))
                showReportWindow = true;

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Gui::usersWindow(const WCSys::Account& account, bool& showUsersWindow)
{
    ImGui::Begin(u8"Пользователи", &showUsersWindow);

    if (ImGui::BeginTabBar("UsersActions"))
    {
        createUserTab(account);
        changeUsersTab(account);

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Gui::createUserTab(const WCSys::Account& account)
{
    if (ImGui::BeginTabItem(u8"Создать"))
    {
        static std::string login{};
        ImGui::InputText(u8"Логин", &login, ImGuiInputTextFlags_CharsNoBlank);

        static std::string password{};
        ImGui::InputText(u8"Пароль", &password, ImGuiInputTextFlags_Password);

        static std::array<bool, 3> permissions{};
        ImGui::Text(u8"Права");
        ImGui::Checkbox(u8"Товары", &permissions[0]);
        ImGui::Checkbox(u8"Заявки", &permissions[1]);
        ImGui::Checkbox(u8"Склады", &permissions[2]);

        static bool showResult{};
        static std::string oldLogin{};
        static std::string oldPassword{};
        if (ImGui::Button(u8"Создать") && !login.empty() && !password.empty() && login != "admin")
        {
            account.createAccount(login, password, WCSys::getPermissionsFromArray(permissions));
            oldLogin = login;
            oldPassword = password;
            showResult = true;
        }

        if (ImGui::Button(u8"Отчистить"))
        {
            login.clear();
            login.shrink_to_fit();
            password.clear();
            password.shrink_to_fit();
            oldLogin.clear();
            oldLogin.shrink_to_fit();
            oldPassword.clear();
            oldPassword.shrink_to_fit();
            std::fill(permissions.begin(), permissions.end(), 0);
            showResult = false;
        }

        if (showResult)
            showActionResult("");

        if (showResult && (oldLogin != login || oldPassword != password))
            showResult = false;

        ImGui::EndTabItem();
    }
}

void Gui::changeUsersTab(const WCSys::Account& account)
{
    if (ImGui::BeginTabItem(u8"Изменить права"))
    {
        static std::string login{};
        ImGui::InputText(u8"Логин", &login, ImGuiInputTextFlags_CharsNoBlank);

        static std::string oldLogin{};
        static std::array<bool, 3> permissions{};
        static bool showPermissions{};
        if (ImGui::Button(u8"Найти") && !login.empty())
        {
            permissions = WCSys::getArrayFromPermissions(account.getUserPermissions(login));
            oldLogin = login;
            showPermissions = true;
        }

        static bool showResults{};
        if (showPermissions && oldLogin == login)
        {
            ImGui::Text(u8"Права");
            ImGui::Checkbox(u8"Товары", &permissions[0]);
            ImGui::Checkbox(u8"Заявки", &permissions[1]);
            ImGui::Checkbox(u8"Склады", &permissions[2]);

            if (ImGui::Button(u8"Изменить"))
            {
                account.changeAccountPermissions(login, WCSys::getPermissionsFromArray(permissions));
                showResults = true;
            }

            if (showResults)
                showActionResult("");
        }
        else
            showResults = false;

        ImGui::EndTabItem();
    }
}

void Gui::reportWindow(const DB::Database& db, bool& showReportWindow)
{
    ImGui::Begin(u8"Отчёты", &showReportWindow);

    static int table = 0;
    ImGui::Combo(u8"Выберете таблицу", &table, "goods\0sales\0warehouse1\0warehouse2\0\0");

    if (ImGui::Button(u8"Создать отчёт"))
    {
        switch (table)
        {
        case 0:
            DB::createReport(db.goods->getData());
            break;
        case 1:
            DB::createReport(db.sales->getData());
            break;
        case 2:
            DB::createReport(db.warehouse1->getData());
            break;
        case 3:
            DB::createReport(db.warehouse2->getData());
            break;
        default:
            break;
        }
    }

    ImGui::End();
}

bool Gui::logInWindow(pqxx::connection& connection, WCSys::UAccount& account)
{
    ImGui::Begin(u8"Вход в приложение");

    static std::string login{};
    ImGui::InputText(u8"Логин", &login, ImGuiInputTextFlags_CharsNoBlank);

    static std::string password{};
    ImGui::InputText(u8"Пароль", &password, ImGuiInputTextFlags_Password);

    static std::string_view oldLogin{};
    static std::string_view oldPassword{};

    static bool showError{};
    static std::string errorStr{};
    if (ImGui::Button(u8"Войти"))
    {
        try
        {
            account = std::make_unique<WCSys::Account>(WCSys::Account::logIn(connection, login, password));

            login.clear();
            login.shrink_to_fit();
            password.clear();
            password.shrink_to_fit();

            ImGui::End();
            return true;
        }
        catch (const WCSys::BadCredentials& e)
        {
            oldLogin = login;
            oldPassword = password;
            errorStr = e.what();
            showError = true;
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"Отчистить"))
    {
        login.clear();
        login.shrink_to_fit();
        password.clear();
        password.shrink_to_fit();
        showError = false;
    }

    if (showError && (oldLogin != login || oldPassword != password))
        showError = false;

    if (showError && oldLogin == login && oldPassword == password)
        ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, errorStr.c_str());

    ImGui::End();

    return false;
}

void Gui::DBGoodsInteraction(DB::GoodsTable& goods, const WCSys::Account& account)
{
    static bool disableGoods{};
    disableGoods = !account.satisfyPermission(WCSys::permissions::modifyGoods);

    ImGui::SeparatorText(u8"Взаимодействие с товарами");
    if (ImGui::BeginTabBar("DBGoodsInteractionTabBar"))
    {
        if (disableGoods)
            ImGui::BeginDisabled();

        DBGoodsCreateTab(goods);
        DBGoodsModifyTab(goods);
        DBGoodsDeleteTab(goods);

        if (disableGoods)
            ImGui::EndDisabled();

        ImGui::EndTabBar();
    }
}

void Gui::DBGoodsCreateTab(DB::GoodsTable& goods)
{
    if (ImGui::BeginTabItem(u8"Создать"))
    {
        static std::string newName{};
        ImGui::InputText(u8"Название", &newName, ImGuiInputTextFlags_CharsNoBlank);

        static int newPriority{};
        ImGui::InputInt(u8"Приоритет", &newPriority);

        static bool showResultStatus{};
        static std::string resultStatus{};
        if (ImGui::Button(u8"Создать"))
        {
            resultStatus = goods.create(newName, newPriority);
            showResultStatus = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"Отчистить"))
        {
            newName.clear();
            newName.shrink_to_fit();
            newPriority = 0;
            showResultStatus = false;
        }

        if (showResultStatus)
            showActionResult(resultStatus);

        ImGui::EndTabItem();
    }
}

void Gui::DBGoodsModifyTab(DB::GoodsTable& goods)
{
    if (ImGui::BeginTabItem(u8"Изменить"))
    {
        static int goodId{};
        ImGui::InputInt(u8"ID товара", &goodId);

        static std::tuple<int, std::string, int> goodRow{};
        static std::string name{};
        static int priority{};
        if (ImGui::Button(u8"Найти"))
        {
            goodRow  = goods.getById(goodId);
            name     = std::get<1>(goodRow);
            priority = std::get<2>(goodRow);
        }

        static bool showResultStatus{};
        if (std::get<0>(goodRow) != 0 && goodId == std::get<0>(goodRow))
        {
            ImGui::InputText(u8"Название", &name, ImGuiInputTextFlags_CharsNoBlank);
            ImGui::InputInt(u8"Приоритет", &priority);

            static std::string resultStatus{};
            if (ImGui::Button(u8"Изменить") &&
                (name != std::get<1>(goodRow) || priority != std::get<2>(goodRow)))
            {
                resultStatus = goods.modify(goodId, name, priority);
                goodRow      = std::make_tuple(goodId, name, priority);
                showResultStatus = true;
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"Отчистить"))
            {
                name     = std::get<1>(goodRow);
                name.shrink_to_fit();
                priority = std::get<2>(goodRow);
                showResultStatus = false;
            }

            if (showResultStatus)
                showActionResult(resultStatus);
        }
        else
            showResultStatus = false;

        ImGui::EndTabItem();
    }
    
}

void Gui::DBGoodsDeleteTab(DB::GoodsTable& goods)
{
    if (ImGui::BeginTabItem(u8"Удалить"))
    {
        static int goodId{};
        ImGui::InputInt(u8"ID товара", &goodId);

        static bool showResultStatus{};
        static std::string resultStatus{};
        if (ImGui::Button(u8"Удалить"))
        {
            resultStatus = goods.deleteById(goodId);
            showResultStatus = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"Отчистить"))
        {
            goodId = 0;
            showResultStatus = false;
        }

        if (showResultStatus)
            showActionResult(resultStatus);

        ImGui::EndTabItem();
    }
}

void Gui::DBSalesInteraction(DB::SalesTable& sales, DB::WarehouseTable& wh1, DB::WarehouseTable& wh2, const WCSys::Account& account)
{
    static bool disableSales{};
    disableSales = !account.satisfyPermission(WCSys::permissions::modifySales);

    ImGui::SeparatorText(u8"Взаимодействие с заявками");
    if (ImGui::BeginTabBar("DBSalesInteractionTabBar"))
    {
        if (disableSales)
            ImGui::BeginDisabled();

        DBSalesCreateTab(sales, wh1, wh2);
        DBSalesModifyTab(sales);
        DBSalesDeleteTab(sales);

        if (disableSales)
            ImGui::EndDisabled();

        ImGui::EndTabBar();
    }
}

void Gui::DBSalesCreateTab(DB::SalesTable& sales, DB::WarehouseTable& wh1, DB::WarehouseTable& wh2)
{
    if (ImGui::BeginTabItem(u8"Создать"))
    {
        static int newGoodId{};
        ImGui::InputInt(u8"ID товара", &newGoodId);

        static int newGoodCount{};
        ImGui::InputInt(u8"Количество", &newGoodCount);

        static int date[3]{};
        ImGui::InputInt3(u8"Дата", date);
        ImGui::Text(u8"Формат даты: YYYY-MM-DD");

        static std::string dateStr{};
        static bool showResultStatus{};
        static std::string resultStatus{};
        static std::tuple<int, int, int> wh1Row{};
        static std::tuple<int, int, int> wh2Row{};
        if (ImGui::Button(u8"Создать"))
        {
            dateStr      = std::to_string(date[0]) + '-' + std::to_string(date[1]) + '-' + std::to_string(date[2]);
            resultStatus = sales.create(newGoodId, newGoodCount, dateStr);
            
            if (resultStatus.empty())
            {
                wh1Row = wh1.getByColumnValue(1, newGoodId);
                wh2Row = wh2.getByColumnValue(1, newGoodId);
                if (std::get<2>(wh1Row) < newGoodCount)
                {
                    wh1.modify(std::get<0>(wh1Row), newGoodId, 0);
                    wh2.modify(std::get<0>(wh2Row), newGoodId,
                        std::get<2>(wh2Row) - newGoodCount + std::get<2>(wh1Row));
                }
                else
                    wh1.modify(std::get<0>(wh1Row), newGoodId, std::get<2>(wh1Row) - newGoodCount);
            }
            
            showResultStatus = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"Отчистить"))
        {
            newGoodId    = 0;
            newGoodCount = 0;
            date[0]      = 0;
            date[1]      = 0;
            date[2]      = 0;
            showResultStatus = false;
        }

        if (showResultStatus)
            showActionResult(resultStatus);

        ImGui::EndTabItem();
    }
}

void Gui::DBSalesModifyTab(DB::SalesTable& sales)
{
    if (ImGui::BeginTabItem(u8"Изменить"))
    {
        static int saleId{};
        ImGui::InputInt(u8"ID заявки", &saleId);

        static std::tuple<int, int, int, std::string> saleRow{};
        static int goodId{};
        static int goodCount{};
        static std::string dateStr{};
        static int date[3]{};
        if (ImGui::Button(u8"Найти"))
        {
            saleRow   = sales.getById(saleId);
            goodId    = std::get<1>(saleRow);
            goodCount = std::get<2>(saleRow);
            dateStr   = std::get<3>(saleRow);
            date[0]   = std::stoi(dateStr.substr(0, 4));
            date[1]   = std::stoi(dateStr.substr(5, 2));
            date[2]   = std::stoi(dateStr.substr(8, 2));
        }

        static bool showResultStatus{};
        if (std::get<0>(saleRow) != 0 && saleId == std::get<0>(saleRow))
        {
            ImGui::InputInt(u8"ID товара", &goodId);
            ImGui::InputInt(u8"Количество", &goodCount);

            ImGui::InputInt3(u8"Дата", date);
            ImGui::Text(u8"Формат даты: YYYY-MM-DD");

            static std::string resultStatus{};
            if (ImGui::Button(u8"Изменить") && (goodId != std::get<1>(saleRow) || goodCount != std::get<2>(saleRow) ||
                dateStr != std::to_string(date[0]) + '-' + std::to_string(date[1]) + '-' + std::to_string(date[2])))
            {
                dateStr      = std::to_string(date[0]) + '-' + std::to_string(date[1]) + '-' + std::to_string(date[2]);
                resultStatus = sales.modify(saleId, goodId, goodCount, dateStr);
                saleRow      = std::make_tuple(saleId, goodId, goodCount, dateStr);
                showResultStatus = true;
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"Отчистить"))
            {
                goodId    = std::get<1>(saleRow);
                goodCount = std::get<2>(saleRow);
                dateStr   = std::get<3>(saleRow);
                date[0]   = std::stoi(dateStr.substr(0, 4));
                date[1]   = std::stoi(dateStr.substr(5, 2));
                date[2]   = std::stoi(dateStr.substr(8, 2));
                showResultStatus = false;
            }

            if (showResultStatus)
                showActionResult(resultStatus);
        }
        else
            showResultStatus = false;

        ImGui::EndTabItem();
    }
}

void Gui::DBSalesDeleteTab(DB::SalesTable& sales)
{
    if (ImGui::BeginTabItem(u8"Удалить"))
    {
        static int saleId{};
        ImGui::InputInt(u8"ID заявки", &saleId);

        static bool showResultStatus{};
        static std::string resultStatus{};
        if (ImGui::Button(u8"Удалить"))
        {
            resultStatus = sales.deleteById(saleId);
            showResultStatus = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"Отчистить"))
        {
            saleId = 0;
            showResultStatus = false;
        }

        if (showResultStatus)
            showActionResult(resultStatus);

        ImGui::EndTabItem();
    }
}

void Gui::DBWarehouseInteraction(DB::WarehouseTable& warehouse1, DB::WarehouseTable& warehouse2, const WCSys::Account& account)
{
    static bool disableWarehouses{};
    disableWarehouses = !account.satisfyPermission(WCSys::permissions::modifyWarehouses);

    ImGui::SeparatorText(u8"Взаимодействие с товарами на складах");
    if (ImGui::BeginTabBar("DBWarehousesInteractionTabBar"))
    {
        if (disableWarehouses)
            ImGui::BeginDisabled();

        DBWarehouseCreateTab(warehouse1, warehouse2);
        DBWarehouseModifyTab(warehouse1, warehouse2);
        DBWarehouseDeleteTab(warehouse1, warehouse2);

        if (disableWarehouses)
            ImGui::EndDisabled();

        ImGui::EndTabBar();
    }
}

void Gui::DBWarehouseCreateTab(DB::WarehouseTable& warehouse1, DB::WarehouseTable& warehouse2)
{
    if (ImGui::BeginTabItem(u8"Создать"))
    {
        static int warehouseNumber = 0;
        ImGui::Combo(u8"Склад", &warehouseNumber, "warehouse1\0warehouse2\0\0");

        static int newGoodId{};
        ImGui::InputInt(u8"ID товара", &newGoodId);

        static int newGoodCount{};
        ImGui::InputInt(u8"Количество", &newGoodCount);

        static bool showResultStatus{};
        static std::string resultStatus{};
        if (ImGui::Button(u8"Создать"))
        {
            resultStatus = !warehouseNumber ? warehouse1.create(newGoodId, newGoodCount) :
                warehouse2.create(newGoodId, newGoodCount);
            showResultStatus = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"Отчистить"))
        {
            newGoodId    = 0;
            newGoodCount = 0;
            showResultStatus = false;
        }

        if (showResultStatus)
            showActionResult(resultStatus);

        ImGui::EndTabItem();
    }
}

void Gui::DBWarehouseModifyTab(DB::WarehouseTable& warehouse1, DB::WarehouseTable& warehouse2)
{
    if (ImGui::BeginTabItem(u8"Изменить"))
    {
        static int warehouseNumber = 0;
        ImGui::Combo(u8"Склад", &warehouseNumber, "warehouse1\0warehouse2\0\0");

        static int id{};
        ImGui::InputInt(u8"ID", &id);

        static int oldWarehouseNumber{};
        static std::tuple<int, int, int> row{};
        static int goodId{};
        static int goodCount{};
        if (ImGui::Button(u8"Найти"))
        {
            row       = !warehouseNumber ? warehouse1.getById(id) : warehouse2.getById(id);
            goodId    = std::get<1>(row);
            goodCount = std::get<2>(row);
            oldWarehouseNumber = warehouseNumber;
        }

        static bool showResultStatus{};
        if (std::get<0>(row) != 0 && warehouseNumber == oldWarehouseNumber && id == std::get<0>(row))
        {
            ImGui::InputInt(u8"ID товара", &goodId);
            ImGui::InputInt(u8"Количество", &goodCount);

            static std::string resultStatus{};
            if (ImGui::Button(u8"Изменить") && (goodId != std::get<1>(row) || goodCount != std::get<2>(row)))
            {
                resultStatus = !warehouseNumber ? warehouse1.modify(id, goodId, goodCount) :
                    warehouse2.modify(id, goodId, goodCount);
                row          = std::make_tuple(id, goodId, goodCount);
                showResultStatus = true;
            }

            ImGui::SameLine();

            if (ImGui::Button(u8"Отчистить"))
            {
                goodId    = std::get<1>(row);
                goodCount = std::get<2>(row);
                showResultStatus = false;
            }

            if (showResultStatus)
                showActionResult(resultStatus);
        }
        else
            showResultStatus = false;

        ImGui::EndTabItem();
    }
}

void Gui::DBWarehouseDeleteTab(DB::WarehouseTable& warehouse1, DB::WarehouseTable& warehouse2)
{
    if (ImGui::BeginTabItem(u8"Удалить"))
    {
        static int warehouseNumber = 0;
        ImGui::Combo(u8"Склад", &warehouseNumber, "warehouse1\0warehouse2\0\0");

        static int id{};
        ImGui::InputInt(u8"ID", &id);

        static bool showResultStatus{};
        static std::string resultStatus{};
        if (ImGui::Button(u8"Удалить"))
        {
            resultStatus = !warehouseNumber ? warehouse1.deleteById(id) : warehouse2.deleteById(id);
            showResultStatus = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"Отчистить"))
        {
            id = 0;
            showResultStatus = false;
        }

        if (showResultStatus)
            showActionResult(resultStatus);

        ImGui::EndTabItem();
    }
}

void Gui::showActionResult(std::string_view result)
{
    if (result.empty())
        ImGui::TextColored(ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f }, u8"Успешно!");
    else
    {
        ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f }, u8"Ошибка!");
        ImGui::TextWrapped("%s", result.data());
    }
}

void Gui::databaseLog(const DB::Database& db)
{
    if (ImGui::BeginTabBar("LogTabBar"))
    {
        if (ImGui::BeginTabItem(u8"Товары"))
        {
            ImGui::SeparatorText(u8"Журнал товаров");
            goodsDataTable(*db.goods);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Заявки"))
        {
            ImGui::SeparatorText(u8"Журнал заявок");
            salesDataTable(*db.sales);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Склады"))
        {
            ImGui::BeginChild("DBwarehouse1", ImVec2{ ImGui::GetContentRegionAvail().x * 0.5f, 0 }, true);
            ImGui::SeparatorText(u8"Журнал товаров на складе 1");
            warehouseDataTable(*db.warehouse1);
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("DBwarehouse2", ImVec2{ 0, 0 }, true);
            ImGui::SeparatorText(u8"Журнал товаров на складе 2");
            warehouseDataTable(*db.warehouse2);
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void Gui::goodsDataTable(DB::GoodsTable& goods)
{
    static std::vector<std::tuple<int, std::string, int>> data{};

    if (ImGui::BeginTable(goods.getTableName().data(), 3, TABLE_FLAGS))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn(u8"Название", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn(u8"Приоритет", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableHeadersRow();

        static ImGuiTableSortSpecs* sort_specs{};
        sort_specs = ImGui::TableGetSortSpecs();

        if (goods.isDataModified() || sort_specs->SpecsDirty)
        {
            data = goods.getData();
            goods.setDataNotMoified();
            sortGoodsDataTable(data, sort_specs);
            sort_specs->SpecsDirty = false;
        }

        for (const auto& [id, name, priority] : data)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", id);

            ImGui::TableNextColumn();
            ImGui::Text(name.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%d", priority);
        }

        ImGui::EndTable();
    }
}

void Gui::sortGoodsDataTable(std::vector<std::tuple<int, std::string, int>>& data, const ImGuiTableSortSpecs* sortSpecs)
{
    using Row = std::tuple<int, std::string, int>;
    std::function<bool(const Row& lhs, const Row& rhs)> cmp{};
    switch (sortSpecs->Specs[0].ColumnIndex)
    {
    case 0:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<0>(lhs) < std::get<0>(rhs);
            };
        break;
    case 1:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<1>(lhs) < std::get<1>(rhs);
            };
        break;
    case 2:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<2>(lhs) < std::get<2>(rhs);
            };
        break;
    default:
        IM_ASSERT(0);
        break;
    }

    sortTableData(data, cmp, sortSpecs->Specs[0].SortDirection);
}

void Gui::salesDataTable(DB::SalesTable& sales)
{
    static std::vector<std::tuple<int, int, int, std::string>> data{};

    if (ImGui::BeginTable(sales.getTableName().data(), 4, TABLE_FLAGS))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn(u8"ID товара");
        ImGui::TableSetupColumn(u8"Количество");
        ImGui::TableSetupColumn(u8"Дата заявки");
        ImGui::TableHeadersRow();

        static ImGuiTableSortSpecs* sort_specs{};
        sort_specs = ImGui::TableGetSortSpecs();

        if (sales.isDataModified() || sort_specs->SpecsDirty)
        {
            data = sales.getData();
            sales.setDataNotMoified();
            sortSalesDataTable(data, sort_specs);
            sort_specs->SpecsDirty = false;
        }

        for (const auto& [id, goodId, goodCount, createDate] : data)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", id);

            ImGui::TableNextColumn();
            ImGui::Text("%d", goodId);

            ImGui::TableNextColumn();
            ImGui::Text("%d", goodCount);

            ImGui::TableNextColumn();
            ImGui::Text(createDate.c_str());
        }

        ImGui::EndTable();
    }
}

void Gui::sortSalesDataTable(std::vector<std::tuple<int, int, int, std::string>>& data, const ImGuiTableSortSpecs* sortSpecs)
{
    using Row = std::tuple<int, int, int, std::string>;
    std::function<bool(const Row& lhs, const Row& rhs)> cmp{};
    switch (sortSpecs->Specs[0].ColumnIndex)
    {
    case 0:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<0>(lhs) < std::get<0>(rhs);
            };
        break;
    case 1:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<1>(lhs) < std::get<1>(rhs);
            };
        break;
    case 2:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<2>(lhs) < std::get<2>(rhs);
            };
        break;
    case 3:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<3>(lhs) < std::get<3>(rhs);
            };
        break;
    default:
        IM_ASSERT(0);
        break;
    }

    sortTableData(data, cmp, sortSpecs->Specs[0].SortDirection);
}

void Gui::warehouseDataTable(DB::WarehouseTable& warehouse)
{
    static std::vector<std::tuple<int, int, int>> data{};

    if (ImGui::BeginTable(warehouse.getTableName().data(), 3, TABLE_FLAGS))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn(u8"ID товара");
        ImGui::TableSetupColumn(u8"Количество");
        ImGui::TableHeadersRow();

        static ImGuiTableSortSpecs* sort_specs{};
        sort_specs = ImGui::TableGetSortSpecs();

        if (warehouse.isDataModified() || sort_specs->SpecsDirty)
        {
            data = warehouse.getData();
            warehouse.setDataNotMoified();
            sortWarehouseDataTable(data, sort_specs);
            sort_specs->SpecsDirty = false;
        }

        for (auto [id, goodId, goodCount] : data)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", id);

            ImGui::TableNextColumn();
            ImGui::Text("%d", goodId);

            ImGui::TableNextColumn();
            ImGui::Text("%d", goodCount);
        }

        ImGui::EndTable();
    }
}

void Gui::sortWarehouseDataTable(std::vector<std::tuple<int, int, int>>& data, const ImGuiTableSortSpecs* sortSpecs)
{
    using Row = std::tuple<int, int, int>;
    std::function<bool(const Row& lhs, const Row& rhs)> cmp{};
    switch (sortSpecs->Specs[0].ColumnIndex)
    {
    case 0:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<0>(lhs) < std::get<0>(rhs);
            };
        break;
    case 1:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<1>(lhs) < std::get<1>(rhs);
            };
        break;
    case 2:
        cmp = [](const Row& lhs, const Row& rhs)
            {
                return std::get<2>(lhs) < std::get<2>(rhs);
            };
        break;
    default:
        IM_ASSERT(0);
        break;
    }

    sortTableData(data, cmp, sortSpecs->Specs[0].SortDirection);
}

void Gui::accountInfo(WCSys::Account& account, bool& loggedIn)
{
    ImGui::Text(u8"Логин: %s", account.getLogin().data());

    static bool showLogs{};
    static bool changePassword{};
    static std::string newPassword{};
    static std::string newConfirmedPassword{};
    if (ImGui::Button(u8"Изменить пароль"))
    {
        changePassword = true;
        newPassword.clear();
        newPassword.shrink_to_fit();
        newConfirmedPassword.clear();
        newConfirmedPassword.shrink_to_fit();
    }

    if (changePassword)
    {
        ImGui::Begin(u8"Изменение пароля", &changePassword);

        ImGui::InputText(u8"Новый пароль", &newPassword, ImGuiInputTextFlags_Password);
        ImGui::InputText(u8"Подтверждение пароля", &newConfirmedPassword, ImGuiInputTextFlags_Password);

        static bool showResult{};
        if (ImGui::Button(u8"Изменить") && !newPassword.empty() && newPassword == newConfirmedPassword)
        {
            account.updatePassword(newPassword);
            showResult = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(u8"Отчистить"))
        {
            newPassword.clear();
            newPassword.shrink_to_fit();
            newConfirmedPassword.clear();
            newConfirmedPassword.shrink_to_fit();
            showResult = false;
        }

        if (!newPassword.empty() && !newConfirmedPassword.empty() && newPassword != newConfirmedPassword)
        {
            showActionResult(u8"Пароли не совпадают!");
            if (showResult)
                showResult = false;
        }

        if (showResult)
            showActionResult("");

        ImGui::End();
    }

    if (ImGui::Button(u8"Сменить пользователя"))
        loggedIn = false;

    if (ImGui::Button(u8"Посмотреть логи"))
        showLogs = true;

    if (showLogs)
    {
        ImGui::Begin(u8"Логи базы данных", &showLogs);



        ImGui::End();
    }
}

void Gui::popularGoodsDataTable(DB::PopularGoodsOfMonthView& popularGoods)
{
    if (ImGui::Button(u8"Обновить данные"))
        popularGoods.updateData();

    if (ImGui::BeginTable(popularGoods.getTableName().data(), 4, TABLE_FLAGS ^ ImGuiTableFlags_Sortable))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn(u8"Название");
        ImGui::TableSetupColumn(u8"Приоритет");
        ImGui::TableSetupColumn(u8"Заявки");
        ImGui::TableHeadersRow();

        for (const auto& [id, name, priority, salesCount] : popularGoods.getData())
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", id);

            ImGui::TableNextColumn();
            ImGui::Text(name.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%d", priority);

            ImGui::TableNextColumn();
            ImGui::Text("%d", salesCount);
        }

        ImGui::EndTable();
    }
}

void Gui::productDemandInfo(pqxx::connection& con)
{
    ImGui::SeparatorText(u8"Изменение спроса на товар");

    static int goodId{};
    ImGui::InputInt(u8"ID товара", &goodId);

    static int firstDate[3]{};
    static std::string firstDateStr{};
    ImGui::InputInt3(u8"Начало интервала", firstDate);

    static int secondDate[3]{};
    static std::string secondDateStr{};
    ImGui::InputInt3(u8"Конец интервала", secondDate);

    ImGui::Text(u8"Формат даты: YYYY-MM-DD");
    static std::vector<std::tuple<int, std::string, int>> data{};
    static DB::ProductDemandPlotData plotData{};
    static bool showResults{};

    if (ImGui::Button(u8"Получить данные"))
    {
        firstDateStr  = std::to_string(firstDate[0])  + '-' + std::to_string(firstDate[1])  + '-' + std::to_string(firstDate[2]);
        secondDateStr = std::to_string(secondDate[0]) + '-' + std::to_string(secondDate[1]) + '-' + std::to_string(secondDate[2]);
        data          = DB::getProductDemand(con, firstDateStr, secondDateStr, goodId);
        
        if (!data.empty())
        {
            plotData    = DB::getProductDemandPlotData(data);
            showResults = true;
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"Отчистить"))
    {
        firstDate[0] = secondDate[0] = 0;
        firstDate[1] = secondDate[1] = 0;
        firstDate[2] = secondDate[2] = 0;
        showResults  = false;
    }

    if (showResults)
    {
        productDemandPlot(plotData);
        productDemandDataTable(data);
    }
}

void Gui::productDemandPlot(const DB::ProductDemandPlotData& data)
{
    if (ImPlot::BeginPlot(u8"График изминения спроса"))
    {
        ImPlot::SetupAxes(u8"Номера дат", u8"Спрос на товар", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::PlotLine("", data.first.data(), data.second.data(), data.first.size());
        ImPlot::EndPlot();
    }
}

void Gui::productDemandDataTable(const std::vector<std::tuple<int, std::string, int>>& data)
{
    if (ImGui::BeginTable("ProductDemandTable", 3, TABLE_FLAGS ^ ImGuiTableFlags_Sortable))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("#");
        ImGui::TableSetupColumn(u8"Дата");
        ImGui::TableSetupColumn(u8"Спрос");
        ImGui::TableHeadersRow();

        for (const auto& [number, date, demand] : data)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", number);

            ImGui::TableNextColumn();
            ImGui::Text(date.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%d", demand);
        }

        ImGui::EndTable();
    }
}
