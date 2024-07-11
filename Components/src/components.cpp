#include "pch.hpp"
#include "components.hpp"
#include "data.hpp"
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
		drawContentBrowserData(state->dataPreview);
		drawPlots(state->dataPreview);

		/*
		ImGui::Begin("Advanced Table");
		Demo_LinePlots();
		drawAdvancedTable();
		ImGui::Text("Hello World");
		ImGui::End();
		ShowSelectableTable();
		ImPlot::ShowDemoWindow();
		ImGui::ShowDemoWindow();
		*/
	}
	bool operator==(const std::pair<std::string, Characteristic> &lhs, const std::pair<std::string, Characteristic> &rhs) { return lhs.first == rhs.first; };
	void drawPlots(Data::DataPreview &dataPreview)
	{
		using namespace UI::Data;
		static ImPlotFlags plot_flags = ImPlotAxisFlags_None;
		PlotData &plotData = dataPreview.plotData;
		// plot_flags = plot_flags | ImPlotAxisFlags_AutoFit; // TODO move this property into  some class, probably PlotProperties

		ImGui::CheckboxFlags("ImPlotAxisFlags_AutoFit##X", (unsigned int *)&plot_flags, ImPlotAxisFlags_AutoFit);
		ImGui::SameLine();
		if (ImGui::Button("Sort"))
		{
			std::unordered_map<std::string, Characteristic> &map = plotData.characteristics;
			std::vector<std::pair<std::string, Characteristic>> characteristics(map.begin(), map.end());
			std::sort(characteristics.begin(), characteristics.end(), [](std::pair<std::string, Characteristic> &a, std::pair<std::string, Characteristic> &b)
					  { return a.second.getTemperature() < b.second.getTemperature(); });

			plotData.characteristics.clear();
			for (std::pair<std::string, Characteristic> &item : characteristics)
				plotData.characteristics[item.first] = item.second;
		}
		ImVec2 plot_size(-1, ImGui::GetContentRegionAvail().x * 0.7f);

		if (!ImGui::GetIO().KeyCtrl && ImGui::GetScrollMaxY() > 0)
			plot_flags = plot_flags | ImPlotFlags_NoInputs;

		static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV;
		std::tuple<std::string, std::string> axisProperties = plotData.plotProperties.axis;
		if (ImPlot::BeginPlot("testing Plot", plot_size, plot_flags))
		{
			// setupPlot(plot_flags, plotData);
			ImPlot::SetupAxes("V", "I", plot_flags, plot_flags);
			if (!plotData.plotProperties.lin_x_scale)
				ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Linear);
			if (!plotData.plotProperties.lin_y_scale)
				ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Linear);
			for (auto &item : plotData.characteristics)
				plotOneCharacteristic(item.second);

			ImPlot::EndPlot();
		}
	}
	void plotOneCharacteristic(UI::Data::Characteristic &characteristic)
	{
		auto V = characteristic.getVoltage();
		auto I = characteristic.getLogCurrent();
		std::string title = "I(V) " + std::to_string(characteristic.getTemperature()) + " K";
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
			for (auto &item : contentBrowserData.contentBrowserData.characteristics)
				contentBrowserData.plotData.addCharacteristic(item.second);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove All"))
		{
			for (auto &item : contentBrowserData.contentBrowserData.characteristics)
				contentBrowserData.plotData.removeCharacteristic(item.second);
		}
		auto &plotData = contentBrowserData.plotData;

		ImGui::SameLine();
		if (ImGui::ColorEdit4("start color", (float *)&plotData.startColor, plotData.colorFlags))

			ImGui::SameLine();
		if (ImGui::ColorEdit4("end color", (float *)&plotData.endColor, plotData.colorFlags))

			ImGui::Spacing();
		ImGui::Spacing();

		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());

		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = ImVec2(p0.x + gradient_size.x, p0.y + gradient_size.y);

		// Ensure conversion to ImU32
		ImVec4 col_a = plotData.startColor.Value;
		ImVec4 col_b = plotData.endColor.Value;

		ImU32 col_a_ = ImGui::GetColorU32(IM_COL32(col_a.x,col_a.y,col_a.z,col_a.w));
		ImU32 col_b_ = ImGui::GetColorU32(IM_COL32(col_b.x,col_b.y,col_b.z,col_b.w));
		draw_list->AddRectFilledMultiColor(p0, p1, col_a_, col_b_, col_b_, col_a_);
		std::cout << "Bad one " << "col A " << col_a_ << " col B " << col_b_ << std::endl;
		std::cout << "Right one " << " col A " << ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)) << " col B " << ImGui::GetColorU32(IM_COL32(0, 0, 255, 255)) << std::endl;
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		if (ImGui::BeginTable("Characteristic", 4, contentBrowserData.plotData.flags))
		{
			ImGui::TableSetupColumn("Checked");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Temperature");
			ImGui::TableSetupColumn("Color");
			ImGui::TableHeadersRow();

			for (auto &item : contentBrowserData.contentBrowserData.characteristics)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Checkbox(item.first.c_str(), &item.second.selected);
				if (item.second.selected)
					contentBrowserData.plotData.addCharacteristic(item.second);
				else
					contentBrowserData.plotData.removeCharacteristic(item.second);

				ImGui::TableNextColumn();
				ImGui::Text(item.first.c_str());
				ImGui::TableNextColumn();
				ImGui::Text(std::to_string(item.second.getTemperature()).c_str());
				ImGui::TableNextColumn();
				ImGui::ColorEdit4(item.first.c_str(), (float *)&item.second.m_color, contentBrowserData.plotData.colorFlags);
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
	struct TableRow
	{
		std::string column1;
		std::string column2;
		std::string column3;
	};
	void ShowSelectableTable()
	{
		static std::vector<TableRow> rows = {
			{"Row 1 Col 1", "Row 1 Col 2", "Row 1 Col 3"},
			{"Row 2 Col 1", "Row 2 Col 2", "Row 2 Col 3"},
			{"Row 3 Col 1", "Row 3 Col 2", "Row 3 Col 3"},
			{"Row 4 Col 1", "Row 4 Col 2", "Row 4 Col 3"}};
		static int selected = -1;

		ImGui::Begin("Selectable Table");

		if (ImGui::BeginTable("##table", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
		{
			ImGui::TableSetupColumn("Column 1");
			ImGui::TableSetupColumn("Column 2");
			ImGui::TableSetupColumn("Column 3");
			ImGui::TableHeadersRow();

			for (int row = 0; row < rows.size(); row++)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				for (int column = 0; column < 3; column++)
				{
					ImGui::TableSetColumnIndex(column);
					std::string cellValue = (column == 0) ? rows[row].column1 : (column == 1) ? rows[row].column2
																							  : rows[row].column3;

					if (ImGui::Selectable(cellValue.c_str(), selected == row))
					{
						selected = row; // Update the selected row
					}
				}
			}
			ImGui::EndTable();
		}

		ImGui::End();
	}
	void Demo_LinePlots()
	{
		static float xs1[1001], ys1[1001];
		for (int i = 0; i < 1001; ++i)
		{
			xs1[i] = i * 0.001f;
			ys1[i] = 0.5f + 0.5f * sinf(50 * (xs1[i] + (float)ImGui::GetTime() / 10));
		}
		static double xs2[20], ys2[20];
		for (int i = 0; i < 20; ++i)
		{
			xs2[i] = i * 1 / 19.0f;
			ys2[i] = xs2[i] * xs2[i];
		}
		if (ImPlot::BeginPlot("Line Plots"))
		{
			ImPlot::SetupAxes("x", "y");
			ImPlot::PlotLine("f(x)", xs1, ys1, 1001);
			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::PlotLine("g(x)", xs2, ys2, 20, ImPlotLineFlags_Segments);
			ImPlot::EndPlot();
		}
	}
};