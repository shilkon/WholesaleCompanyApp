#include "system.h"

#include <imgui.h>
#include <implot.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <pqxx/pqxx>

#include <iostream>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

namespace WCSys = wholesaleCompanySystem;
namespace DB = database;

int main()
{
    std::unique_ptr<po::parsed_options> configOptions{};
    std::unique_ptr<WCSys::WCSystem> system{ std::make_unique<WCSys::WCSystem>() };
    system->database = std::make_unique<DB::Database>();
    try
    {
        configOptions = std::make_unique<po::parsed_options>(WCSys::getConfigOptions());

        system->connection   = std::make_unique<pqxx::connection>(WCSys::getConnectionString(*configOptions));
        system->database->goods        = std::make_unique<DB::GoodsTable>(*system->connection, DB::getQueries(DB::TableType::goods));
        system->database->sales        = std::make_unique<DB::SalesTable>(*system->connection, DB::getQueries(DB::TableType::sales));
        system->database->warehouse1   = std::make_unique<DB::WarehouseTable>(*system->connection, DB::getQueries(DB::TableType::warehouse1));
        system->database->warehouse2   = std::make_unique<DB::WarehouseTable>(*system->connection, DB::getQueries(DB::TableType::warehouse2));
        system->database->popularGoods = std::make_unique<DB::PopularGoodsOfMonthView>(
            *system->connection, DB::getQueries(DB::TableType::popularGoodsOfTheMonth)
        );
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    bool loggedIn{};
    bool showUsersWindow{};
    bool showReportWindow{};
    bool exit{};

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version{ "#version 460" };
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window{ glfwCreateWindow(1280, 720, u8"Оптовая компания", nullptr, nullptr) };
    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io{ ImGui::GetIO() };
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    auto font{ io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf",
        20.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic()) };
    IM_ASSERT(font != nullptr);
    auto clear_color{ ImVec4(0.45f, 0.55f, 0.60f, 0.00f) };

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;

    ImGuiStyle& style{ ImGui::GetStyle() };
    style.FrameRounding      = 3.0f;
    style.TabBarBorderSize   = 1.0f;
    //style.WindowBorderSize   = 1.0f;
    style.SeparatorTextAlign = ImVec2{ 0.5f, 0.5f };

    while (!glfwWindowShouldClose(window))
    {
        if (exit)
            break;

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (loggedIn)
        {
            ImGui::SetNextWindowSize(ImVec2{ io.DisplaySize.x, io.DisplaySize.y });
            ImGui::SetNextWindowPos(ImVec2{ 0, 0 });

            if (!ImGui::Begin("Wholesale App", nullptr, window_flags))
            {
                ImGui::End();
                break;
            }

            if (showUsersWindow)
                Gui::usersWindow(*system->account, showUsersWindow);

            if (showReportWindow)
                Gui::reportWindow(*system->database, showReportWindow);

            Gui::menuBar(*system->account, showUsersWindow, showReportWindow);

            static float leftWidth{};
            leftWidth = ImGui::GetContentRegionAvail().x * 0.3f;

            ImGui::BeginGroup();
            ImGui::BeginChild("DBGoodsInteraction", ImVec2{ leftWidth, ImGui::GetContentRegionAvail().y / 3 });
            Gui::DBGoodsInteraction(*system->database->goods, *system->account);
            ImGui::EndChild();

            ImGui::BeginChild("DBSalesInteraction", ImVec2{ leftWidth, ImGui::GetContentRegionAvail().y * 0.5f });
            Gui::DBSalesInteraction(*system->database->sales, *system->database->warehouse1, *system->database->warehouse2, *system->account);
            ImGui::EndChild();

            ImGui::BeginChild("DBWarehouseInteraction", ImVec2{ leftWidth, 0 });
            Gui::DBWarehouseInteraction(*system->database->warehouse1, *system->database->warehouse2, *system->account);
            ImGui::EndChild();
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::BeginChild("DatabaseLog", ImVec2{ ImGui::GetContentRegionAvail().x * 0.4f / 0.7f, 0 });
            Gui::databaseLog(*system->database);
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::BeginChild("AccountInfo", ImVec2{ 0, ImGui::GetContentRegionAvail().y * 0.14f });
            ImGui::SeparatorText(u8"Пользователь");
            Gui::accountInfo(*system->account, loggedIn);
            ImGui::EndChild();

            ImGui::BeginChild("PopularGoods", ImVec2{ 0, ImGui::GetContentRegionAvail().y * 0.26f });
            ImGui::SeparatorText(u8"Самые популярные товары");
            Gui::popularGoodsDataTable(*system->database->popularGoods);
            ImGui::EndChild();

            ImGui::BeginChild("DemandChange", ImVec2{ 0, 0 });
            Gui::productDemandInfo(*system->connection);
            ImGui::EndChild();
            ImGui::EndGroup();

            ImGui::End();
        }
        else if (Gui::logInWindow(*system->connection, system->account))
            loggedIn = true;

        ImGui::Render();
        int display_w;
        int display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
            clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
