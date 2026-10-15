#pragma once
#include <Windows.h>

#include <cstdint>
#include <memory>
#include <stdexcept>

enum class KeyState : bool
{
	ePressed = true,
	eReleased = false
};

struct KeyboardInput
{
	uint8_t key{ 0xff };
	KeyState state{ KeyState::eReleased };
};

enum class MouseState : uint8_t
{
	eNone = 0,
	eLeft = 1,
	eRight = 2,
	eMiddle = 3,
	eX1 = 4,
	eX2 = 5
};

struct MouseInput
{
	MouseInput() = default;
	int32_t dX{ 0 }, dY{ 0 };
	int32_t wheelDelta{ 0 };
	MouseState key{ MouseState::eNone };
	KeyState state{ KeyState::eReleased };
};

struct UserInput
{
	KeyboardInput keyboardInput;
	MouseInput mouseInput;
};
