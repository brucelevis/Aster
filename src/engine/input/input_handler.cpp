#include "input_handler.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace
{
  KeyAction GlfwKeyActionToEngine(const int action)
  {
    switch (action)
    {
    case GLFW_RELEASE:
      return KeyAction::Release;
    case GLFW_PRESS:
      return KeyAction::Press;
    case GLFW_REPEAT:
      return KeyAction::Repeat;
    default:
      throw std::runtime_error("unknwon key action");
    }
  }

  const std::unordered_map<std::string, int> StringToGlfwKeyMap = {
        { "SPACE", GLFW_KEY_SPACE },
        { "`", GLFW_KEY_APOSTROPHE },
        { ",", GLFW_KEY_COMMA },
        { "-", GLFW_KEY_MINUS },
        { ".", GLFW_KEY_PERIOD },
        { "/", GLFW_KEY_SLASH },
        { "0", GLFW_KEY_0 },
        { "1", GLFW_KEY_1 },
        { "2", GLFW_KEY_2 },
        { "3", GLFW_KEY_3 },
        { "4", GLFW_KEY_4 },
        { "5", GLFW_KEY_5 },
        { "6", GLFW_KEY_6 },
        { "7", GLFW_KEY_7 },
        { "8", GLFW_KEY_8 },
        { "9", GLFW_KEY_9 },
        { "=", GLFW_KEY_EQUAL },
        { ";", GLFW_KEY_SEMICOLON },
        {"A", GLFW_KEY_A},
        {"B", GLFW_KEY_B},
        {"C", GLFW_KEY_C},
        {"D", GLFW_KEY_D},
        {"E", GLFW_KEY_E},
        {"F", GLFW_KEY_F},
        {"G", GLFW_KEY_G},
        {"H", GLFW_KEY_H},
        {"I", GLFW_KEY_I},
        {"J", GLFW_KEY_J},
        {"K", GLFW_KEY_K},
        {"L", GLFW_KEY_L},
        {"M", GLFW_KEY_M},
        {"N", GLFW_KEY_N},
        {"O", GLFW_KEY_O},
        {"P", GLFW_KEY_P},
        {"Q", GLFW_KEY_Q},
        {"R", GLFW_KEY_R},
        {"S", GLFW_KEY_S},
        {"T", GLFW_KEY_T},
        {"U", GLFW_KEY_U},
        {"V", GLFW_KEY_V},
        {"W", GLFW_KEY_W},
        {"X", GLFW_KEY_X},
        {"Y", GLFW_KEY_Y},
        {"Z", GLFW_KEY_Z},
        { "ESCAPE", GLFW_KEY_ESCAPE },
        { "ENTER", GLFW_KEY_ENTER },
        { "TAB", GLFW_KEY_TAB },
  };
}

void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  auto* handler = reinterpret_cast<InputHandler*>(glfwGetWindowUserPointer(window));
  if (handler != nullptr)
    handler->KeyCallback(window, key, scancode, action, mods);
}

void GLFW_CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
  auto* handler = reinterpret_cast<InputHandler*>(glfwGetWindowUserPointer(window));
  if (handler != nullptr)
    handler->CursorPositionCallback(window, xpos, ypos);
}

InputHandler::InputHandler(GLFWwindow* window)
  : window(window)
{
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, GLFW_KeyCallback);
  glfwSetCursorPosCallback(window, GLFW_CursorPositionCallback);
}

void InputHandler::PollEvents()
{
  glfwPollEvents();
}

void InputHandler::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  KeyState& state = keyStates[key];
  if (state.binding && (action != GLFW_REPEAT))
  {
    KeyAction currentKeyAction = GlfwKeyActionToEngine(action);

    const float value = currentKeyAction == KeyAction::Release ? 0.0f : state.value;
    state.binding(value);
    state.lastAction = currentKeyAction;
  }
}

void InputHandler::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
  static bool isInitialized = false;
  if (!isInitialized)
  {
    //to prevent `wtf` rotation after a first input, since lastX,lastY = 0.0f at the start.
    mouseState.lastX = xpos;
    mouseState.lastY = ypos;
    isInitialized = true;
    return;
  }

  const float dx = xpos - mouseState.lastX;
  const float dy = ypos - mouseState.lastY;

  if (mouseState.binding)
    mouseState.binding(dx, dy);

  mouseState.lastX = xpos;
  mouseState.lastY = ypos;
}

void InputHandler::SetKeyBinding(const std::string& key, float value, KeyInputBinding binding)
{
  const int glfwKey = StringToGlfwKeyMap.at(key);
  keyStates[glfwKey].binding = binding;
  keyStates[glfwKey].value = value;
}

void InputHandler::DisableCursor()
{
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}