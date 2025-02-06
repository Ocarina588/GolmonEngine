#include <iostream>
#include <glm/glm.hpp>
#include <array>
#include "GolmonEngine.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Core.hpp"

int main(int ac, char** av)
{
	try {
		Core core;
		return core.main(ac, av);
	}
	catch (std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}