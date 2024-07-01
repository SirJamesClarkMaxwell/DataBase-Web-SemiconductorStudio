#include <iostream>
#include "tester.hpp"

int main()
{

	Platform::Window::initProperties(1920, 1080, "MyApp");

	// TestedFitting to be initialized in the constructor of Tester
	Tester t1{};

	t1.run(UI::Components::draw_tester);

	return 0;
}