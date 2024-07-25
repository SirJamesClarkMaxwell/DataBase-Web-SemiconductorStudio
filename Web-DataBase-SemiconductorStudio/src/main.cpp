#include "pch.hpp"
#include "tester.hpp"
#include "main_json.hpp"
#include "../cpr/include/cpr/cpr.h"



int main()
{
	// TODO add seperate group of projects and include there JSON and YAML

	// testingJSON();

	 #if 1

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

	std::cout << r.text << std::endl;
	YAML::Node config = YAML::LoadFile("C:\\Users\\Franek\\Desktop\\STUDIA\\SemiconductorStudio-All\\DataBase-Web-SemiconductorStudio\\Web-DataBase-SemiconductorStudio\\config.yaml");

	/*if (config["lastLogin"]) {
		std::cout << "Last logged in: " << config["lastLogin"].as<DateTime>() << "\n";
	}*/

	const std::string username = config["username"].as<std::string>();
	const std::string password = config["password"].as<std::string>();
	std::cout << "Username: " << username << "\n";
		std::cout << "Password: " << password << "\n";

	std::ofstream fout("config.yaml");
	fout << config;
	return 0;
};