#include "game.hpp"
#include "color.hpp"
#include "shader.hpp"

int main(int argc, char* argv[])
{
	Game podracer("PODRACER - STAR WARS");
	podracer.start();
	podracer.quit();

	return 0;
}
