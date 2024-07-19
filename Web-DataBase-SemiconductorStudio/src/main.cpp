#include "pch.hpp"
#include "tester.hpp"
#include "main_json.hpp"
#include "../cpr/include/cpr/cpr.h"

int main()
{
	// TODO add seperate group of projects and include there JSON and YAML

	// testingJSON();

	 #if 0

	Platform::Window::initProperties(1920, 1080, "Web-DataBase-SemiconductorStudio");
	// TestedFitting to be initialized in the constructor of Tester
	Tester t1{};
	t1.run(UI::Components::draw_tester);
	 #endif


	cpr::Response r = cpr::Get(cpr::Url{ "https://api.github.com/repos/whoshuu/cpr/contributors" },
		cpr::Authentication{ "user", "pass", cpr::AuthMode::BASIC },
		cpr::Parameters{ {"anon", "true"}, {"key", "value"} });
	r.status_code;                  // 200
	r.header["content-type"];       // application/json; charset=utf-8
	r.text;                         // JSON text string
	return 0;
};