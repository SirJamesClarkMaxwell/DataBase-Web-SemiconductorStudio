#include "pch.hpp"
#include "components.hpp"
#include "data.hpp"
#include <filesystem>
#include <ranges>
#include <algorithm>

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
		drawContentBrowserData(state->dataPreview);
		drawPlots(state->dataPreview);

		/*
		ImGui::Begin("Advanced Table");
		Demo_LinePlots();
		*/
		/*
		drawAdvancedTable();
		ImGui::Text("Hello World");
		ImGui::End();
		ShowSelectableTable();
		ImPlot::ShowDemoWindow();
		ImGui::ShowDemoWindow();
		*/
	}
	bool operator==(const Characteristic &lhs, const Characteristic &rhs) { return lhs.getTemperature() == rhs.getTemperature(); };
	void drawPlots(Data::DataPreview &dataPreview)
	{
		using namespace UI::Data;
		static ImPlotFlags plot_flags = ImPlotAxisFlags_None;
		PlotData &plotData = dataPreview.plotData;
		// plot_flags = plot_flags | ImPlotAxisFlags_AutoFit; // TODO move this property into  some class, probably PlotProperties

		ImGui::CheckboxFlags("ImPlotAxisFlags_AutoFit##X", (unsigned int *)&plot_flags, ImPlotAxisFlags_AutoFit);
		ImGui::SameLine();
		if (ImGui::Button("Sort"))
			std::sort(plotData.characteristics.begin(), plotData.characteristics.end(), [](Characteristic &a, Characteristic &b)
					  { return a.getTemperature() < b.getTemperature(); });

		if (ImGui::Button("Reverse"))
			std::reverse(plotData.characteristics.begin(), plotData.characteristics.end());

		ImVec2 plot_size(-1, ImGui::GetContentRegionAvail().x * 0.7f);

		if (!ImGui::GetIO().KeyCtrl && ImGui::GetScrollMaxY() > 0)
			plot_flags = plot_flags | ImPlotFlags_NoInputs;

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV;
		std::tuple<std::string, std::string> axisProperties = plotData.plotProperties.axis;

		PlotProperties &plotProperties = plotData.plotProperties;
		/*
				ImU32 &name = plotProperties.name;
				if (plotProperties.addedColorMap == -1 && plotProperties.colors.size() > 0)
				{
					plotProperties.customRGMMap = ImPlot::AddColormap(std::to_string(name).c_str(), plotProperties.colorMapPointer, plotProperties.colorMap.size());
					plotProperties.addedColorMap = 1;
				}

				if (plotProperties.colors.size() > 0)
					ImPlot::PushColormap(plotProperties.customRGMMap);
		*/
		ImPlot::BeginPlot("testing Plot", plot_size, plot_flags);
		// setupPlot(plot_flags, plotData);
		ImPlot::SetupAxes("V", "I", plot_flags, plot_flags);
		if (!plotData.plotProperties.lin_x_scale)
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
		if (!plotData.plotProperties.lin_y_scale)
			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);
		for (auto &item : plotData.characteristics)
			if (item)
				plotOneCharacteristic(item);

		ImPlot::EndPlot();
		/*
		if (plotData.plotProperties.colors.size() > 1)
			ImPlot::PopColormap();
		*/
	}
	void plotOneCharacteristic(UI::Data::Characteristic &characteristic)
	{
		auto V = characteristic.getVoltage();
		auto I = characteristic.getLogCurrent();
		std::string title = "I(V) " + std::to_string(characteristic.getTemperature()) + " K";
		ImPlot::SetNextLineStyle(characteristic.m_color);
		ImPlot::PlotLine(title.c_str(), V.data(), I.data(), V.size());
	}
	void setupPlot(ImPlotFlags plot_flags, UI::Data::PlotData &plotData)
	{
		ImPlot::SetupAxes("V", "I");
		if (!plotData.plotProperties.lin_x_scale)
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
		if (!plotData.plotProperties.lin_y_scale)
			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
	}
	void drawContentBrowserData(Data::DataPreview &contentBrowserData)
	{

		ImGui::Begin("Content Browser");

		if (ImGui::Button("Read all"))
			readAllDataFromDirectory(contentBrowserData.rootPath, contentBrowserData.currentPath, contentBrowserData.contentBrowserData);
		ImGui::SameLine();
		if (ImGui::Button("Transfer Read data to Plot"))
		{
			for (auto &item : contentBrowserData.contentBrowserData.characteristics)
			{
				if (!checkExistence(contentBrowserData.plotData.characteristics, item.name))
					contentBrowserData.plotData.characteristics.push_back(item);
			}
		}
		if (contentBrowserData.currentPath != std::filesystem::path(contentBrowserData.rootPath))
		{
			if (ImGui::Button("<-"))
				contentBrowserData.currentPath = contentBrowserData.currentPath.parent_path();
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
					contentBrowserData.contentBrowserData.readCharacteristic(path);
			}
		}

		ImGui::Begin("Plotting Settings ");

		if (ImGui::Button("Plot all"))
		{
			for (Characteristic &item : contentBrowserData.contentBrowserData.characteristics)
				contentBrowserData.plotData.addCharacteristic(item);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove All"))
		{
			for (Characteristic &item : contentBrowserData.contentBrowserData.characteristics)
				item.selected = false;
		}
		auto &plotData = contentBrowserData.plotData;

		ImGui::SameLine();
		if (ImGui::ColorEdit4("start color", (float *)&plotData.startColor, plotData.colorFlags))
		{
		}
		ImGui::SameLine();
		if (ImGui::ColorEdit4("end color", (float *)&plotData.endColor, plotData.colorFlags))
		{
		}
		ImGui::SameLine();
		if (ImGui::Button("Update Characteristics Color"))
		{
			plotData.setColorsOfCharacteristics();
			plotData.setColorsOfGraph();

			// static std::vector<ImU32> colorMap;

			if (plotData.characteristics.size() > 1)
			{
				auto func = [&](const ImColor &item)
				{
					return ImGui::ColorConvertFloat4ToU32(item);
				};

				plotData.plotProperties.colorMap.resize(plotData.plotProperties.colors.size());
				std::transform(plotData.plotProperties.colors.begin(), plotData.plotProperties.colors.end(), plotData.plotProperties.colorMap.begin(), func);
				plotData.plotProperties.colorMapPointer = plotData.plotProperties.colorMap.data();
				plotData.plotProperties.addedColorMap = -1;
				const ImColor diff = plotData.endColor - plotData.startColor;
				plotData.plotProperties.name = ImGui::ColorConvertFloat4ToU32(diff);
			}
		}
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::Spacing();

		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());

		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);

		ImU32 col_a_ = ImGui::ColorConvertFloat4ToU32(plotData.startColor);
		ImU32 col_b_ = ImGui::ColorConvertFloat4ToU32(plotData.endColor);
		draw_list->AddRectFilledMultiColor(p0, p1, col_a_, col_b_, col_b_, col_a_);

		ImGui::Dummy(ImVec2(0.0f, ImGui::GetFrameHeight()));
		if (ImGui::BeginTable("Characteristic", 4, contentBrowserData.plotData.flags))
		{
			ImGui::TableSetupColumn("Checked");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Temperature");
			ImGui::TableSetupColumn("Color");
			ImGui::TableHeadersRow();

			for (auto &item : contentBrowserData.plotData.characteristics)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Checkbox(item.name.c_str(), &item.selected);
				ImGui::TableNextColumn();
				ImGui::Text(item.name.c_str());
				ImGui::TableNextColumn();
				ImGui::Text(std::to_string(item.getTemperature()).c_str());
				ImGui::TableNextColumn();
				ImGui::ColorEdit4(item.name.c_str(), (float *)&item.m_color, contentBrowserData.plotData.colorFlags);
			}

			ImGui::EndTable();
		}

		ImGui::End();
		ImGui::End();
	}
	void readAllDataFromDirectory(const std::filesystem::path &rootPath, const std::filesystem::path &currentPath, ContentBrowserData &contentBrowserData)
	{
		for (auto &directoryEntry : std::filesystem::directory_iterator(currentPath))
		{
			const auto &path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, rootPath);
			std::string fileNameString = relativePath.filename().string();
			if (directoryEntry.is_directory())
				continue;
			else
				contentBrowserData.readCharacteristic(path);
		}
	};

	void Demo_LinePlots()
	{
		static float xs1[1001], ys1[1001];
		for (int i = 0; i < 1001; ++i)
		{
			xs1[i] = i * 0.001f;
			ys1[i] = 0.5f + 0.5f * sinf(50 * (xs1[i] + (float)ImGui::GetTime() / 10));
		}

		static float xs3[1001], ys3[1001];
		for (int i = 0; i < 1001; ++i)
		{
			xs3[i] = i * 0.001f;
			ys3[i] = 0.5f + 0.5f * sinf(50 * (xs3[i] + (float)ImGui::GetTime() / 5));
		}
		static double xs2[20], ys2[20];
		for (int i = 0; i < 20; ++i)
		{
			xs2[i] = i * 1 / 19.0f;
			ys2[i] = xs2[i] * xs2[i];
		}
		static ImU32 colorDataRGB[3] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF};
		static int customRGBMap;
		static int i = 0;
		if (i == 0)
		{
			customRGBMap = ImPlot::AddColormap("RGBColors", colorDataRGB, 3);
			i++;
		}
		/*
				int customRGBMap2 = ImPlot::GetColormapIndex("RGBColors");
				if (customRGBMap2 >= ImPlot::GetColormapCount())
		*/
		ImPlot::PushColormap(customRGBMap);

		if (ImPlot::BeginPlot("Line Plots"))
		{
			ImPlot::SetupAxes("x", "y");
			ImPlot::PlotLine("f(x)", xs1, ys1, 1001);
			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::PlotLine("g(x)", xs2, ys2, 20, ImPlotLineFlags_Segments);
			ImPlot::PlotLine("h(x)", xs3, ys3, 20, ImPlotLineFlags_Segments);
			ImPlot::EndPlot();
			/*
			int customRGBMap1 = ImPlot::GetColormapIndex("RGBColors");
			if (customRGBMap1 > ImPlot::GetColormapCount())
			{
			*/
			ImPlot::PopColormap();
			//}
		}
	}
};