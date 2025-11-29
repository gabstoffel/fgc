#include "Input.h"
#include "Player.h"
#include "Game.h"
#include "Logger.h"
#include <cstdio>

Player* Input::s_player = nullptr;
Game* Input::s_game = nullptr;
float Input::s_screenRatio = 1.0f;
bool Input::s_leftMouseButtonPressed = false;
bool Input::s_shootRequested = false;
double Input::s_lastCursorPosX = 0.0;
double Input::s_lastCursorPosY = 0.0;

void Input::init(Player* player, Game* game)
{
    s_player = player;
    s_game = game;
}

void Input::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    s_screenRatio = (float)width / height;
}

void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &s_lastCursorPosX, &s_lastCursorPosY);
        s_leftMouseButtonPressed = true;

        if (s_player != nullptr && s_player->isFirstPerson() &&
            s_game != nullptr && s_game->getGameState() == GameState::PLAYING)
            s_shootRequested = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        s_leftMouseButtonPressed = false;
    }
}

void Input::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    static int callback_count = 0;
    callback_count++;
    bool should_log = (callback_count % 100 == 0);

    if (s_player == nullptr)
    {
        if (should_log)
            Logger::logEvent("Input.cursorPosCallback.error", "{\"reason\":\"s_player is null\"}");
        return;
    }

    float dx = xpos - s_lastCursorPosX;
    float dy = ypos - s_lastCursorPosY;

    if (should_log)
        Logger::logEvent("Input.cursorPosCallback",
            "{\"x\":%.1f,\"y\":%.1f,\"dx\":%.1f,\"dy\":%.1f,\"firstPerson\":%s,\"leftPressed\":%s,\"count\":%d}",
            xpos, ypos, dx, dy,
            s_player->isFirstPerson() ? "true" : "false",
            s_leftMouseButtonPressed ? "true" : "false",
            callback_count);

    if (!s_player->isFirstPerson())
    {
        if (!s_leftMouseButtonPressed)
        {
            s_lastCursorPosX = xpos;
            s_lastCursorPosY = ypos;
            return;
        }
    }

    s_player->handleMouseMove(dx, dy);

    s_lastCursorPosX = xpos;
    s_lastCursorPosY = ypos;
}

void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (s_player == nullptr)
        return;

    s_player->handleScroll(yoffset);
}

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    Logger::logEvent("Input.keyCallback",
        "{\"key\":%d,\"action\":%d,\"isF\":%s,\"isESC\":%s,\"isK\":%s}",
        key, action,
        (key == GLFW_KEY_F) ? "true" : "false",
        (key == GLFW_KEY_ESCAPE) ? "true" : "false",
        (key == GLFW_KEY_K) ? "true" : "false");

    if (s_game == nullptr)
        return;

    GameState gameState = s_game->getGameState();

    if (gameState == GameState::MENU && action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_1)
        {
            s_game->setDifficulty(0);
            s_game->startGame();
            return;
        }
        else if (key == GLFW_KEY_2)
        {
            s_game->setDifficulty(1);
            s_game->startGame();
            return;
        }
        else if (key == GLFW_KEY_3)
        {
            s_game->setDifficulty(2);
            s_game->startGame();
            return;
        }
    }

    if ((gameState == GameState::GAME_OVER || gameState == GameState::WIN) && action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_R)
        {
            s_game->resetGame();
            return;
        }
        else if (key == GLFW_KEY_M)
        {
            s_game->returnToMenu();
            return;
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if (gameState == GameState::PLAYING)
        {
            s_game->returnToMenu();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        return;
    }

    if (gameState != GameState::PLAYING)
        return;

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        Logger::logEvent("Input.keyCallback.F_pressed", "{}");

        if (s_player == nullptr)
        {
            Logger::logEvent("Input.keyCallback.F_pressed.error", "{\"reason\":\"s_player is null\"}");
            return;
        }

        s_player->toggleCamera();

        if (s_player->isFirstPerson())
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            s_lastCursorPosX = xpos;
            s_lastCursorPosY = ypos;
            Logger::logEvent("Input.cursorMode.disabled",
                "{\"x\":%.1f,\"y\":%.1f}",
                xpos, ypos);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            Logger::logEvent("Input.cursorMode.normal", "{}");
        }
    }
}

void Input::errorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

bool Input::isShootingRequested()
{
    bool requested = s_shootRequested;
    s_shootRequested = false;
    return requested;
}
