#pragma once
#include "pch.hpp"
#include "storage.hpp"
#include "imgui.h"
#include "implot.h"
#include "./LambertW.h"

namespace UI::Components
{
	using namespace UI::Data;
	void draw_bg_window();
	void draw_tester(std::shared_ptr<MyData> state);

	void drawPlots(DataPreview &plotData);
	void plotOneCharacteristic(UI::Data::Characteristic &characteristic, bool log = false);
	void setupPlot(ImPlotFlags plot_flags, UI::Data::PlotData &plotData);
	void drawContentBrowserData(DataPreview &ContentBrowserDataData);
	void readAllDataFromDirectory(const std::filesystem::path &rootPath, const std::filesystem::path &currentPath, ContentBrowserData &contentBrowserData);
	void ShowSelectableTable();
	void ShowMultiSelectableTable();
	void drawAdvancedTable();
	void Demo_LinePlots();
	void drawGeneratingCurvesPanel(GeneratingCharacteristicsPanelData &data);
	void LamberWWindow(ImVec2 &plot_size, ImPlotFlags plot_flags, UI::Data::PlotProperties &plotProperties);
	void TestFittingAndMC(JunctionFitMasterUI::FittingTesting &data);
}
