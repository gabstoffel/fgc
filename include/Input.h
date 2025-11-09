#ifndef INPUT_H
#define INPUT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Player;
class Game;

class Input
{
public:
    static void init(Player* player, Game* game);

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod);
    static void errorCallback(int error, const char* description);

    static float getScreenRatio() { return s_screenRatio; }
    static bool isLeftMouseButtonPressed() { return s_leftMouseButtonPressed; }
    static bool isShootingRequested();

private:
    static Player* s_player;
    static Game* s_game;

    static float s_screenRatio;
    static bool s_leftMouseButtonPressed;
    static bool s_shootRequested;
    static double s_lastCursorPosX;
    static double s_lastCursorPosY;
};

#endif 
