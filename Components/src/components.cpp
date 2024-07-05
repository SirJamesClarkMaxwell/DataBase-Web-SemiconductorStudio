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

		drawPlots(state->dataPreview);
		drawContentBrowserData(state->dataPreview);

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

	void drawPlots(Data::DataPreview &dataPreview)
	{
		using namespace UI::Data;
		static ImPlotFlags plot_flags = ImPlotAxisFlags_None;
		PlotData plotData = dataPreview.plotData;
		// plot_flags = plot_flags | ImPlotAxisFlags_AutoFit; // TODO move this property into  some class, probably PlotProperties

		ImGui::CheckboxFlags("ImPlotAxisFlags_AutoFit##X", (unsigned int *)&plot_flags, ImPlotAxisFlags_AutoFit);
		ImGui::SameLine();
		if (ImGui::Button("Sort"))
		{
			auto &characteristics = plotData.getCharacteristics();

			std::sort(characteristics.begin(), characteristics.end());
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
			for (auto &characteristic : plotData.getCharacteristics())
				plotOneCharacteristic(characteristic);

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

		static std::vector<std::string> items = {"Item 1", "Item 2", "Item 3", "Item 4"};
		static int selected = -1;

		ImGui::End();
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