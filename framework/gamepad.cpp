#include "gamepad.h"

#include <cmath>
#include <xinput.h>

#pragma comment(lib, "xinput.lib")

static XINPUT_STATE gCurrentStates[XUSER_MAX_COUNT] = {};
static XINPUT_STATE gPreviousStates[XUSER_MAX_COUNT] = {};
static bool gConnected[XUSER_MAX_COUNT] = {};
static Gamepad_Layout gLayout = GAMEPAD_LAYOUT_SWITCH_ABXY;

static int ClampInt(int value, int minValue, int maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }
    if (value > maxValue)
    {
        return maxValue;
    }
    return value;
}

static float ClampFloat01(float value)
{
    if (value < 0.0f)
    {
        return 0.0f;
    }
    if (value > 1.0f)
    {
        return 1.0f;
    }
    return value;
}

static bool IsValidPlayerIndex(int playerIndex)
{
    return playerIndex >= 0 && playerIndex < XUSER_MAX_COUNT;
}

static WORD ToPhysicalMask(Gamepad_Button button)
{
    switch (button)
    {
    case GPB_A:
        return (gLayout == GAMEPAD_LAYOUT_SWITCH_ABXY) ? XINPUT_GAMEPAD_B : XINPUT_GAMEPAD_A;
    case GPB_B:
        return (gLayout == GAMEPAD_LAYOUT_SWITCH_ABXY) ? XINPUT_GAMEPAD_A : XINPUT_GAMEPAD_B;
    case GPB_X:
        return (gLayout == GAMEPAD_LAYOUT_SWITCH_ABXY) ? XINPUT_GAMEPAD_Y : XINPUT_GAMEPAD_X;
    case GPB_Y:
        return (gLayout == GAMEPAD_LAYOUT_SWITCH_ABXY) ? XINPUT_GAMEPAD_X : XINPUT_GAMEPAD_Y;
    case GPB_DPAD_UP:
        return XINPUT_GAMEPAD_DPAD_UP;
    case GPB_DPAD_DOWN:
        return XINPUT_GAMEPAD_DPAD_DOWN;
    case GPB_DPAD_LEFT:
        return XINPUT_GAMEPAD_DPAD_LEFT;
    case GPB_DPAD_RIGHT:
        return XINPUT_GAMEPAD_DPAD_RIGHT;
    case GPB_LEFT_SHOULDER:
        return XINPUT_GAMEPAD_LEFT_SHOULDER;
    case GPB_RIGHT_SHOULDER:
        return XINPUT_GAMEPAD_RIGHT_SHOULDER;
    case GPB_START:
        return XINPUT_GAMEPAD_START;
    case GPB_BACK:
        return XINPUT_GAMEPAD_BACK;
    case GPB_LEFT_STICK:
        return XINPUT_GAMEPAD_LEFT_THUMB;
    case GPB_RIGHT_STICK:
        return XINPUT_GAMEPAD_RIGHT_THUMB;
    default:
        return 0;
    }
}

static float NormalizeAxis(SHORT raw, SHORT deadZone)
{
    const int value = static_cast<int>(raw);
    const int absValue = std::abs(value);
    if (absValue <= deadZone)
    {
        return 0.0f;
    }

    const int range = 32767 - deadZone;
    const int adjusted = ClampInt(absValue - deadZone, 0, range);
    const float normalized = (range > 0) ? (static_cast<float>(adjusted) / static_cast<float>(range)) : 0.0f;
    return (value < 0) ? -normalized : normalized;
}

static float NormalizeTrigger(BYTE raw)
{
    if (raw <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
    {
        return 0.0f;
    }

    const int range = 255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    const int adjusted = ClampInt(static_cast<int>(raw) - XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 0, range);
    return (range > 0) ? (static_cast<float>(adjusted) / static_cast<float>(range)) : 0.0f;
}

void Gamepad_Initialize(void)
{
    ZeroMemory(gCurrentStates, sizeof(gCurrentStates));
    ZeroMemory(gPreviousStates, sizeof(gPreviousStates));
    ZeroMemory(gConnected, sizeof(gConnected));
    gLayout = GAMEPAD_LAYOUT_SWITCH_ABXY;
}

void Gamepad_Finalize(void)
{
    for (int i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        Gamepad_SetVibration(i, 0.0f, 0.0f);
    }
}

void Gamepad_Update(void)
{
    for (int i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        gPreviousStates[i] = gCurrentStates[i];

        XINPUT_STATE state = {};
        const DWORD result = XInputGetState(i, &state);
        gConnected[i] = (result == ERROR_SUCCESS);

        if (gConnected[i])
        {
            gCurrentStates[i] = state;
        }
        else
        {
            ZeroMemory(&gCurrentStates[i], sizeof(XINPUT_STATE));
        }
    }
}

bool Gamepad_IsConnected(int playerIndex)
{
    if (!IsValidPlayerIndex(playerIndex))
    {
        return false;
    }
    return gConnected[playerIndex];
}

bool Gamepad_IsButtonDown(int playerIndex, Gamepad_Button button)
{
    if (!IsValidPlayerIndex(playerIndex) || !gConnected[playerIndex])
    {
        return false;
    }

    const WORD mask = ToPhysicalMask(button);
    if (mask == 0)
    {
        return false;
    }

    return (gCurrentStates[playerIndex].Gamepad.wButtons & mask) != 0;
}

bool Gamepad_IsButtonTrigger(int playerIndex, Gamepad_Button button)
{
    if (!IsValidPlayerIndex(playerIndex) || !gConnected[playerIndex])
    {
        return false;
    }

    const WORD mask = ToPhysicalMask(button);
    if (mask == 0)
    {
        return false;
    }

    const bool isDownNow = (gCurrentStates[playerIndex].Gamepad.wButtons & mask) != 0;
    const bool wasDown = (gPreviousStates[playerIndex].Gamepad.wButtons & mask) != 0;
    return isDownNow && !wasDown;
}

Gamepad_ThumbStick Gamepad_GetLeftStick(int playerIndex)
{
    Gamepad_ThumbStick out = { 0.0f, 0.0f };
    if (!IsValidPlayerIndex(playerIndex) || !gConnected[playerIndex])
    {
        return out;
    }

    out.x = NormalizeAxis(gCurrentStates[playerIndex].Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    out.y = NormalizeAxis(gCurrentStates[playerIndex].Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    return out;
}

Gamepad_ThumbStick Gamepad_GetRightStick(int playerIndex)
{
    Gamepad_ThumbStick out = { 0.0f, 0.0f };
    if (!IsValidPlayerIndex(playerIndex) || !gConnected[playerIndex])
    {
        return out;
    }

    out.x = NormalizeAxis(gCurrentStates[playerIndex].Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    out.y = NormalizeAxis(gCurrentStates[playerIndex].Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    return out;
}

float Gamepad_GetLeftTrigger(int playerIndex)
{
    if (!IsValidPlayerIndex(playerIndex) || !gConnected[playerIndex])
    {
        return 0.0f;
    }

    return NormalizeTrigger(gCurrentStates[playerIndex].Gamepad.bLeftTrigger);
}

float Gamepad_GetRightTrigger(int playerIndex)
{
    if (!IsValidPlayerIndex(playerIndex) || !gConnected[playerIndex])
    {
        return 0.0f;
    }

    return NormalizeTrigger(gCurrentStates[playerIndex].Gamepad.bRightTrigger);
}

void Gamepad_SetVibration(int playerIndex, float leftMotor, float rightMotor)
{
    if (!IsValidPlayerIndex(playerIndex))
    {
        return;
    }

    const float left = ClampFloat01(leftMotor);
    const float right = ClampFloat01(rightMotor);

    XINPUT_VIBRATION vibration = {};
    vibration.wLeftMotorSpeed = static_cast<WORD>(left * 65535.0f);
    vibration.wRightMotorSpeed = static_cast<WORD>(right * 65535.0f);
    XInputSetState(playerIndex, &vibration);
}

void Gamepad_SetLayout(Gamepad_Layout layout)
{
    gLayout = layout;
}

Gamepad_Layout Gamepad_GetLayout(void)
{
    return gLayout;
}