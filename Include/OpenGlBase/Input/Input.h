#pragma once
#include <glm/glm.hpp>

namespace Base
{
    namespace Input
    {
        enum CursorModes
        {
            CursorNormal,
            CursorHidden,
            CursorDisabled,
            CursorCapured,
        };

        enum MouseButtonCodes
        {
            LeftButton,
            RightButton,
            MiddleWheel,
            Button4,
            Button5,
            Button6,
            Button7,
            NumMouseButtonCodes,
        };

        enum KeyCodes
        {
            UnknownKey,

            HatCentered,
            HatUp,
            HatRight,
            HatDown,
            HatLeft,
            HatRightUp,
            HatRightDown,
            HatLeftUp,
            HatLeftDown,

            Space,
            Apostrophe,
            Comma,
            Minus,
            Period,
            Slash,

            Key0, Key1, Key2, Key3, Key4,
            Key5, Key6, Key7, Key8, Key9,

            SemiColon,
            Equal,

            A, B, C, D, E, F, G, H, I, J,
            K, L, M, N, O, P, Q, R, S, T,
            U, V, W, X, Y, Z,

            LeftBracket,
            Backslash,
            RightBracket,
            GraveAccent,
            World1,
            World2,
            Escape,
            Enter,
            Tab,
            Backspace,
            Insert,
            Delete,

            Right,
            Left,
            Down,
            Up,

            PageUp,
            PageDown,
            Home,
            End,
            CapsLock,
            ScrollLock,
            NumLock,
            PrintScreen,
            Pause,

            F1, F2, F3, F4, F5,
            F6, F7, F8, F9, F10,
            F11, F12, F13, F14, F15,
            F16, F17, F18, F19, F20,
            F21, F22, F23, F24, F25,

            KP0, KP1, KP2, KP3, KP4,
            KP5, KP6, KP7, KP8, KP9,

            KPDecimal,
            KPDivide,
            KPMultiply,
            KPSubtract,
            KPAdd,
            KPEnter,
            KPEqual,

            LeftShift,
            LeftControl,
            LeftAlt,
            LeftSuper,
            RightShift,
            RightControl,
            RightAlt,
            RightSuper,

            Menu,

            NumKeyCodes,
        };

        bool Init();
        void Destroy();
        void Tick();

        void SetCursorMode(CursorModes Mode);
        dvec2 GetMousePosition();
        dvec2 GetMouseDelta();
        f64 GetScrollDelta();

        bool IsMouseButtonDown(MouseButtonCodes Button);
        static const char* GetMouseButtonName(MouseButtonCodes Button);
        bool WasMouseButtonJustPressed(MouseButtonCodes Button);
        f64 GetTimeSinceMouseButtonPressed(MouseButtonCodes Button);

        const char* GetKeyName(KeyCodes Key);
        bool IsKeyDown(KeyCodes Key);
        bool WasKeyJustPressed(KeyCodes Key);
        f64 GetTimeSinceKeyPressed(KeyCodes Key);

    }
}

