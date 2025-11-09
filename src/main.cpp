// Headers C
#include <cstdlib>
// Game includes
#include "Game.h"

int main()
{
    Game game;

    if (!game.init())
    {
        return EXIT_FAILURE;
    }

    game.run();

    game.cleanup();

    return EXIT_SUCCESS;
}
