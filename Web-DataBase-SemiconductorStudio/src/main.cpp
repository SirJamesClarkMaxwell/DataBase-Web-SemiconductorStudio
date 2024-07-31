#pragma once
#include "pch.hpp"
#include "tester.hpp"
#include "main_json.hpp"
#include "../cpr/include/cpr/cpr.h"



int main()
{

	Platform::Window::initProperties(1920, 1080, "Web-DataBase-SemiconductorStudio");
	// TestedFitting to be initialized in the constructor of Tester
	Tester t1{};
	t1.run(UI::Components::draw_tester);

	 

	return 0;
};