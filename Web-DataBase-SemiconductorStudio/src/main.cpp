#include <iostream>
#include "tester.hpp"
#include "json.hpp"

int main()
{
	//TODO add seperate group of projects and include there JSON and YAML
	auto json = nlohmann::json::parse("{\"value1\": \"string\"}");

	// mutate the json
	json["value1"] = "new string";

	// write to a stream, or the same file
	std::cout << json; // print the json

	/*
	Platform::Window::initProperties(1920, 1080, "Web-DataBase-SemiconductorStudio");

	// TestedFitting to be initialized in the constructor of Tester
	Tester t1{};

	t1.run(UI::Components::draw_tester);
	*/

	return 0;
}