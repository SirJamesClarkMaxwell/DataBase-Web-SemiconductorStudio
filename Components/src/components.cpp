#pragma once
#include "components.hpp"
#include "./../include/data.hpp"
#include <filesystem>
namespace UI::Components
{

	void draw_bg_window()
	{
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoNav;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

		const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y));
		ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x, main_viewport->WorkSize.y));
		bool open = true;
		ImGui::Begin("##", &open, window_flags);

		ImGui::End();
	}

	void draw_tester(std::shared_ptr<Data::MyData> state)
	{

		drawPlots(state->dataPreview);
		drawContentBrowserData(state->dataPreview);
		ImPlot::ShowDemoWindow();
		// ImGui::ShowDemoWindow();
	}

	void drawPlots(Data::DataPreview &dataPreview)
	{
		using namespace UI::Data;

		static ImPlotFlags plot_flags = ImPlotAxisFlags_AutoFit; // TODO move this property into  some class, probably PlotProperties
		ImGui::CheckboxFlags("ImPlotAxisFlags_AutoFit##X", (unsigned int *)&plot_flags, ImPlotAxisFlags_AutoFit);
		ImVec2 plot_size(-1, ImGui::GetContentRegionAvail().x * 0.7f);
		if (!ImGui::GetIO().KeyCtrl && ImGui::GetScrollMaxY() > 0)
			plot_flags = ImPlotFlags_NoInputs;

		PlotData plotData = dataPreview.plotData;

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV;
		std::tuple<std::string, std::string> axisProperties = plotData.plotProperties.axis;
		if (ImPlot::BeginPlot("testing Plot", plot_size))
		{
			// setupPlot(plot_flags, plotData);
			ImPlot::SetupAxes("V", "I", plot_flags, plot_flags);
			if (!plotData.plotProperties.lin_x_scale)
				ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
			if (!plotData.plotProperties.lin_y_scale)
				ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);
			for (int i = 0; i < plotData.numberOfCharacteristics; i++)
			{
				Characteristic characteristic = plotData[i];
				plotOneCharacteristic(characteristic);
			}
			ImPlot::EndPlot();
		}
	}
	void plotOneCharacteristic(UI::Data::Characteristic &characteristic)
	{
		auto V = characteristic.getVoltage();
		auto I = characteristic.getLogCurrent();

		ImPlot::PlotLine("I(V)", V.data(), I.data(), V.size());
	}
	void setupPlot(ImPlotFlags plot_flags, UI::Data::PlotData &plotData)
	{
		ImPlot::SetupAxes("V", "I", plot_flags, plot_flags);
		if (!plotData.plotProperties.lin_x_scale)
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
		if (!plotData.plotProperties.lin_y_scale)
			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
	}
	void drawContentBrowserData(Data::DataPreview &contentBrowserData)
	{
		ImGui::Begin("Content Browser");
		if (contentBrowserData.currentPath != std::filesystem::path(contentBrowserData.rootPath))
		{
			if (ImGui::Button("<-"))
			{
				contentBrowserData.currentPath = contentBrowserData.currentPath.parent_path();
			}
		}
		for (auto &directoryEntry : std::filesystem::directory_iterator(contentBrowserData.currentPath))
		{
			const auto &path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, contentBrowserData.rootPath);
			std::string fileNameString = relativePath.filename().string();
			if (directoryEntry.is_directory())
			{
				if (ImGui::Button(fileNameString.c_str()))
					contentBrowserData.currentPath /= path.filename();
			}
			else
			{
				if (ImGui::Button(fileNameString.c_str()))
					contentBrowserData.plotData.addCharacteristic(path);
			}
		}
		ImGui::End();
	};
}