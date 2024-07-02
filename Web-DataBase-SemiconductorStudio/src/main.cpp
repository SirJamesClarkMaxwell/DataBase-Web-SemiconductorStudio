#include <iostream>
#include <fstream>
#include "tester.hpp"
#include "json.hpp"
#include "main_json.hpp"

int main()
{
	// TODO add seperate group of projects and include there JSON and YAML
	
	testingJSON();


#if 0
	
	Platform::Window::initProperties(1920, 1080, "Web-DataBase-SemiconductorStudio");

	// TestedFitting to be initialized in the constructor of Tester
	Tester t1{};

	t1.run(UI::Components::draw_tester);
	
#endif
	return 0;
};