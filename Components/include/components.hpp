#pragma once
#include "data.hpp"
#include "stl.hpp"
#include "imgui.h"
#include "implot.h"

namespace UI::Components
{
	using namespace UI::Data;
	void draw_bg_window();
	void draw_tester(std::shared_ptr<MyData> state);

	void drawPlots(DataPreview &plotData);
	void plotOneCharacteristic(UI::Data::Characteristic &characteristic);
	void setupPlot(ImPlotFlags plot_flags, UI::Data::PlotData &plotData);
	void drawContentBrowserData(DataPreview &ContentBrowserDataData);
	void ShowSelectableTable();
	void ShowMultiSelectableTable();
	void drawAdvancedTable();
	void Demo_LinePlots();
}
