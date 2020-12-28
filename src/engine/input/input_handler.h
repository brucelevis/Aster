#pragma once

#include <functional>
#include <array>

enum class KeyAction
{
  Press,
  Release,
  Repeat,
};

typedef std::function<void(float)> KeyInputBinding;
typedef std::function<void(float, float)> MouseInputBinding;

struct GLFWwindow;


class InputHandler
{
  struct KeyState
  {
    KeyAction lastAction = KeyAction::Release;
    KeyInputBinding binding;
    float Value = 0.0f;
  };

  struct MouseState
  {
    float lastX = 0.0f;
    float lastY = 0.0f;
    MouseInputBinding binding;
  };

public:
  InputHandler(GLFWwindow* window);

  void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

  void SetKeyBinding(const std::string& key, KeyInputBinding binding);

  inline void SetMouseInputBinding(MouseInputBinding binding)
  {
    mouseState.binding = binding;
  }

private:
  std::array<KeyState, 400> keyStates;
  MouseState mouseState;
};