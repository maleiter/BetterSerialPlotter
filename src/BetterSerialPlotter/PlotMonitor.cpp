#include <BetterSerialPlotter/PlotMonitor.hpp>
#include <BetterSerialPlotter/BSP.hpp>
#include <iostream>
#include <Mahi/Util/Logging/Csv.hpp>
#include <array>
#include <Mahi/Util/Random.hpp>
#include <Mahi/Util/Print.hpp>
#include <thread>

using namespace mahi::util;

namespace bsp{

PlotMonitor::PlotMonitor(BSP* gui_): 
    Widget(gui_)
    {
        all_plots.emplace_back(this);
    }

void PlotMonitor::render(){

    for (auto i = 0; i < all_plots.size(); i++){
        all_plots[i].make_plot(paused ? paused_time : gui->time, i);
    }
    
    ImGui::PushStyleColor(ImGuiCol_Button,gui->PalleteBlue);

    if(ImGui::Button("Add Plot")) {
        all_plots.emplace_back(this);
        all_plots[all_plots.size()-1].name = "Plot " + std::to_string(all_plots.size());
    }
    ImGui::PopStyleColor();
    if (all_plots.size() > 1){
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,gui->PalleteRed);
        if(ImGui::Button("Remove Plot")) {
            all_plots.pop_back();
        }
        ImGui::PopStyleColor();
    }
}

void PlotMonitor::plot_all_data(){
    for (const auto &i : gui->all_data){
        all_plots[0].add_identifier(i.identifier);
    }
}

void PlotMonitor::export_data(){
    std::string filepath;
    auto result = mahi::gui::save_dialog(filepath, {{"CSV", "csv"}}, "", "data.csv");
    if (result == mahi::gui::DialogResult::DialogOkay){


        auto& datasrc = paused ? gui->all_data_paused: gui->all_data;

       // all_plot_paused_data
        auto num_datas = datasrc.size();

        if (num_datas == 0){
            std::cout << "no data to export";
            return;
        }

        auto largest_data_set = std::max_element(datasrc.cbegin(), datasrc.cend(), [](auto const& left, auto const& right) {return left.Data.size() < right.Data.size(); });
        auto max_samples = largest_data_set->Data.size();

        mahi::util::print("Path: {}",filepath);
        
        std::vector<std::string> headers;
        std::vector<std::vector<double>> all_rows;
        
        headers.reserve(num_datas+1);

        // add the names as headers for the csv file
        headers.push_back("Program Time [s]");
        for (const auto &data : datasrc){
            headers.push_back(gui->get_name(data.identifier));
        }
        
        // // add all of the data points
        for (auto i = 0; i < max_samples; i++){
            std::vector<double> row;
            row.reserve(num_datas+1);
            row.push_back(datasrc[0].Data[(i + datasrc[0].Offset)% datasrc[0].Data.size()].x);
            for (const auto &data : datasrc){
                if (i < data.Data.size())
                    row.push_back(data.Data[(i + data.Offset) % data.Data.size()].y);
                else
                    row.push_back(NAN);
            }
            all_rows.push_back(row);
        }
        
        csv_write_row(filepath, headers);
        csv_append_rows(filepath, all_rows);
    }
}

} // namespace bsp