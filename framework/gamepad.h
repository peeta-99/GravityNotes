#pragma once

#include <windows.h>

typedef enum Gamepad_Layout_tag
{
    GAMEPAD_LAYOUT_XBOX = 0,
    GAMEPAD_LAYOUT_SWITCH_ABXY,
} Gamepad_Layout;

typedef enum Gamepad_Button_tag
{
    GPB_A = 0,
    GPB_B,
    GPB_X,
    GPB_Y,
    GPB_DPAD_UP,
    GPB_DPAD_DOWN,
    GPB_DPAD_LEFT,
    GPB_DPAD_RIGHT,
    GPB_LEFT_SHOULDER,
    GPB_RIGHT_SHOULDER,
    GPB_START,
    GPB_BACK,
    GPB_LEFT_STICK,
    GPB_RIGHT_STICK,
} Gamepad_Button;

typedef struct Gamepad_ThumbStick_tag
{
    float x;
    float y;
} Gamepad_ThumbStick;

void Gamepad_Initialize(void);
void Gamepad_Finalize(void);
void Gamepad_Update(void);

bool Gamepad_IsConnected(int playerIndex);
bool Gamepad_IsButtonDown(int playerIndex, Gamepad_Button button);
bool Gamepad_IsButtonTrigger(int playerIndex, Gamepad_Button button);

Gamepad_ThumbStick Gamepad_GetLeftStick(int playerIndex);
Gamepad_ThumbStick Gamepad_GetRightStick(int playerIndex);
float Gamepad_GetLeftTrigger(int playerIndex);
float Gamepad_GetRightTrigger(int playerIndex);

void Gamepad_SetVibration(int playerIndex, float leftMotor, float rightMotor);

void Gamepad_SetLayout(Gamepad_Layout layout);
Gamepad_Layout Gamepad_GetLayout(void);