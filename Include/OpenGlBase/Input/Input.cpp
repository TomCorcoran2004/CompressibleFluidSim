#include "Input.h"

#include <array>

#include <GLFW/glfw3.h>
#include "../Window/Window.h"
namespace Base
{
    namespace Input
    {
        struct Key
        {
            bool Pressed = false;
            bool Released = true;
            f64 TimeSincePressed = 0.0;
        };

        void KeyCallBack(GLFWwindow* WindowInstance, int Key, int Scancode, int Action, int Mods);
        void CursorPositionCallBack(GLFWwindow* WindowInstance, double XPosition, double YPosition);
        void MouseButtonCallBack(GLFWwindow* WindowInstance, int Button, int Action, int Mods);
        void ScrollCallBack(GLFWwindow* WindowInstance, double XOffset, double YOffset);

        constexpr KeyCodes ConvertGLFWKey(i32 GLFWKey);
        constexpr MouseButtonCodes ConvertGLFWMouseButton(i32 GLFWMouseButton);
        constexpr i32 ConvertToGLFWCursorMode(CursorModes CursorMode);

        std::array<Key, NumKeyCodes> Keys;
        std::array<Key, NumMouseButtonCodes> MouseButtons;

        dvec2 MousePosition = { 0.0, 0.0 };
        dvec2 LastMousePosition = { 0.0, 0.0 };
        dvec2 MouseDelta = { 0.0, 0.0 };
        f64 ScrollDelta = 0.0;
        f64 ScrollOffset = 0.0;

        bool Init()
        {
            if (Window::GetGLFWWindow() == nullptr)
            {
                assert(Window::GetGLFWWindow());
                return false;
            }

            if (glfwRawMouseMotionSupported())
                glfwSetInputMode(Window::GetGLFWWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

            glfwSetKeyCallback(Window::GetGLFWWindow(), KeyCallBack);
            glfwSetCursorPosCallback(Window::GetGLFWWindow(), CursorPositionCallBack);
            glfwSetMouseButtonCallback(Window::GetGLFWWindow(), MouseButtonCallBack);
            glfwSetScrollCallback(Window::GetGLFWWindow(), ScrollCallBack);

            return true;
        }

        void Destroy()
        {

        }

        void Tick()
        {
            LastMousePosition = MousePosition;
            MouseDelta = MousePosition - LastMousePosition;
            LastMousePosition = MousePosition;

            ScrollDelta = ScrollOffset;
            ScrollOffset = 0.0;
        }

        void SetCursorMode(CursorModes Mode)
        {
            assert(Window::GetGLFWWindow());

            glfwSetInputMode(Window::GetGLFWWindow(), GLFW_CURSOR, ConvertToGLFWCursorMode(Mode));
        }

        dvec2 GetMousePosition()
        {
            return MousePosition;
        }

        dvec2 GetMouseDelta()
        {
            return MouseDelta;
        }

        f64 GetScrollDelta()
        {
            return ScrollDelta;
        }

        bool IsMouseButtonDown(MouseButtonCodes Button)
        {
            return MouseButtons[Button].Pressed;
        }

        const char* GetMouseButtonName(MouseButtonCodes Button)
        {
            switch (Button)
            {
            case(LeftButton):    return "MouseLeft";
            case(RightButton):   return "MouseRight";
            case(MiddleWheel):   return "MouseMiddle";
            case(Button4):       return "Mouse4";
            case(Button5):       return "Mouse5";
            case(Button6):       return "Mouse6";
            case(Button7):       return "Mouse7";
            }

            return "";
        }

        bool WasMouseButtonJustPressed(MouseButtonCodes Button)
        {
            if (MouseButtons[Button].Pressed && MouseButtons[Button].Released)
            {
                MouseButtons[Button].Released = false;
                return true;
            }
            else return false;
        }

        f64 GetTimeSinceMouseButtonPressed(MouseButtonCodes Button)
        {
            return glfwGetTime() - MouseButtons[Button].TimeSincePressed;
        }

        const char* GetKeyName(KeyCodes Key)
        {
            switch (Key)
            {
            case(UnknownKey): return "Unknown";
            case(HatCentered): return "HatCentered";
            case(HatUp): return "HatUp";
            case(HatRight): return "HatRight";
            case(HatDown): return "HatDown";
            case(HatLeft): return "HatLeft";
            case(HatRightUp): return "HatRightUp";
            case(HatRightDown): return "HatRightDown";
            case(HatLeftUp): return "HatLeftUp";
            case(HatLeftDown): return "HatLeftDown";
            case(Space): return "Space";
            case(Apostrophe): return "Apostrophe";
            case(Comma): return "Comma";
            case(Minus): return "Minus";
            case(Period): return "Period";
            case(Slash): return "Slash";
            case(Key0): return "Key0";
            case(Key1): return "Key1";
            case(Key2): return "Key2";
            case(Key3): return "Key3";
            case(Key4): return "Key4";
            case(Key5): return "Key5";
            case(Key6): return "Key6";
            case(Key7): return "Key7";
            case(Key8): return "Key8";
            case(Key9): return "Key9";
            case(SemiColon): return "SemiColon";
            case(Equal): return "Equal";
            case(A): return "A";
            case(B): return "B";
            case(C): return "C";
            case(D): return "D";
            case(E): return "E";
            case(F): return "F";
            case(G): return "G";
            case(H): return "H";
            case(I): return "I";
            case(J): return "J";
            case(K): return "K";
            case(L): return "L";
            case(M): return "M";
            case(N): return "N";
            case(O): return "O";
            case(P): return "P";
            case(Q): return "Q";
            case(R): return "R";
            case(S): return "S";
            case(T): return "T";
            case(U): return "U";
            case(V): return "V";
            case(W): return "W";
            case(X): return "X";
            case(Y): return "Y";
            case(Z): return "Z";
            case(LeftBracket): return "LeftBracket";
            case(Backslash): return "Backslash";
            case(RightBracket): return "RightBracket";
            case(GraveAccent): return "GraveAccent";
            case(World1): return "World1";
            case(World2): return "World2";
            case(Escape): return "Escape";
            case(Enter): return "Enter";
            case(Tab): return "Tab";
            case(Backspace): return "Backspace";
            case(Insert): return "Insert";
            case(Delete): return "Delete";
            case(Right): return "Right";
            case(Left): return "Left";
            case(Down): return "Down";
            case(Up): return "Up";
            case(PageUp): return "PageUp";
            case(PageDown): return "PageDown";
            case(Home): return "Home";
            case(End): return "End";
            case(CapsLock): return "CapsLock";
            case(ScrollLock): return "ScrollLock";
            case(NumLock): return "NumLock";
            case(PrintScreen): return "PrintScreen";
            case(Pause): return "Pause";
            case(F1): return "F1";
            case(F2): return "F2";
            case(F3): return "F3";
            case(F4): return "F4";
            case(F5): return "F5";
            case(F6): return "F6";
            case(F7): return "F7";
            case(F8): return "F8";
            case(F9): return "F9";
            case(F10): return "F10";
            case(F11): return "F11";
            case(F12): return "F12";
            case(F13): return "F13";
            case(F14): return "F14";
            case(F15): return "F15";
            case(F16): return "F16";
            case(F17): return "F17";
            case(F18): return "F18";
            case(F19): return "F19";
            case(F20): return "F20";
            case(F21): return "F21";
            case(F22): return "F22";
            case(F23): return "F23";
            case(F24): return "F24";
            case(F25): return "F25";
            case(KP0): return "KP0";
            case(KP1): return "KP1";
            case(KP2): return "KP2";
            case(KP3): return "KP3";
            case(KP4): return "KP4";
            case(KP5): return "KP5";
            case(KP6): return "KP6";
            case(KP7): return "KP7";
            case(KP8): return "KP8";
            case(KP9): return "KP9";
            case(KPDecimal): return "KPDecimal";
            case(KPDivide): return "KPDivide";
            case(KPMultiply): return "KPMultiply";
            case(KPSubtract): return "KPSubtract";
            case(KPAdd): return "KPAdd";
            case(KPEnter): return "KPEnter";
            case(KPEqual): return "KPEqual";
            case(LeftShift): return "LeftShift";
            case(LeftControl): return "LeftControl";
            case(LeftAlt): return "LeftAlt";
            case(LeftSuper): return "LeftSuper";
            case(RightShift): return "RightShift";
            case(RightControl): return "RightControl";
            case(RightAlt): return "RightAlt";
            case(RightSuper): return "RightSuper";
            case(Menu): return "Menu";
            }

            return "";
        }

        bool IsKeyDown(KeyCodes Key)
        {
            return Keys[Key].Pressed;
        }

        bool WasKeyJustPressed(KeyCodes Key)
        {
            if (Keys[Key].Pressed && Keys[Key].Released)
            {
                Keys[Key].Released = false;
                return true;
            }
            else return false;
        }

        f64 GetTimeSinceKeyPressed(KeyCodes Key)
        {
            return glfwGetTime() - Keys[Key].TimeSincePressed;
        }

        constexpr KeyCodes ConvertGLFWKey(i32 GLFWKey)
        {
            switch (GLFWKey)
            {
            case(GLFW_KEY_UNKNOWN): return UnknownKey;
            case(GLFW_HAT_CENTERED): return HatCentered;
            case(GLFW_HAT_UP): return HatUp;
            case(GLFW_HAT_RIGHT): return HatRight;
            case(GLFW_HAT_DOWN): return HatDown;
            case(GLFW_HAT_LEFT): return HatLeft;
            case(GLFW_HAT_RIGHT_UP): return HatRightUp;
            case(GLFW_HAT_RIGHT_DOWN): return HatRightDown;
            case(GLFW_HAT_LEFT_UP): return HatLeftUp;
            case(GLFW_HAT_LEFT_DOWN): return HatLeftDown;
            case(GLFW_KEY_SPACE): return Space;
            case(GLFW_KEY_APOSTROPHE): return Apostrophe;
            case(GLFW_KEY_COMMA): return Comma;
            case(GLFW_KEY_MINUS): return Minus;
            case(GLFW_KEY_PERIOD): return Period;
            case(GLFW_KEY_SLASH): return Slash;
            case(GLFW_KEY_0): return Key0;
            case(GLFW_KEY_1): return Key1;
            case(GLFW_KEY_2): return Key2;
            case(GLFW_KEY_3): return Key3;
            case(GLFW_KEY_4): return Key4;
            case(GLFW_KEY_5): return Key5;
            case(GLFW_KEY_6): return Key6;
            case(GLFW_KEY_7): return Key7;
            case(GLFW_KEY_8): return Key8;
            case(GLFW_KEY_9): return Key9;
            case(GLFW_KEY_SEMICOLON): return SemiColon;
            case(GLFW_KEY_EQUAL): return Equal;
            case(GLFW_KEY_A): return A;
            case(GLFW_KEY_B): return B;
            case(GLFW_KEY_C): return C;
            case(GLFW_KEY_D): return D;
            case(GLFW_KEY_E): return E;
            case(GLFW_KEY_F): return F;
            case(GLFW_KEY_G): return G;
            case(GLFW_KEY_H): return H;
            case(GLFW_KEY_I): return I;
            case(GLFW_KEY_J): return J;
            case(GLFW_KEY_K): return K;
            case(GLFW_KEY_L): return L;
            case(GLFW_KEY_M): return M;
            case(GLFW_KEY_N): return N;
            case(GLFW_KEY_O): return O;
            case(GLFW_KEY_P): return P;
            case(GLFW_KEY_Q): return Q;
            case(GLFW_KEY_R): return R;
            case(GLFW_KEY_S): return S;
            case(GLFW_KEY_T): return T;
            case(GLFW_KEY_U): return U;
            case(GLFW_KEY_V): return V;
            case(GLFW_KEY_W): return W;
            case(GLFW_KEY_X): return X;
            case(GLFW_KEY_Y): return Y;
            case(GLFW_KEY_Z): return Z;
            case(GLFW_KEY_LEFT_BRACKET): return LeftBracket;
            case(GLFW_KEY_BACKSLASH): return Backslash;
            case(GLFW_KEY_RIGHT_BRACKET): return RightBracket;
            case(GLFW_KEY_GRAVE_ACCENT): return GraveAccent;
            case(GLFW_KEY_WORLD_1): return World1;
            case(GLFW_KEY_WORLD_2): return World2;
            case(GLFW_KEY_ESCAPE): return Escape;
            case(GLFW_KEY_ENTER): return Enter;
            case(GLFW_KEY_TAB): return Tab;
            case(GLFW_KEY_BACKSPACE): return Backspace;
            case(GLFW_KEY_INSERT): return Insert;
            case(GLFW_KEY_DELETE): return Delete;
            case(GLFW_KEY_RIGHT): return Right;
            case(GLFW_KEY_LEFT): return Left;
            case(GLFW_KEY_DOWN): return Down;
            case(GLFW_KEY_UP): return Up;
            case(GLFW_KEY_PAGE_UP): return PageUp;
            case(GLFW_KEY_PAGE_DOWN): return PageDown;
            case(GLFW_KEY_HOME): return Home;
            case(GLFW_KEY_END): return End;
            case(GLFW_KEY_CAPS_LOCK): return CapsLock;
            case(GLFW_KEY_SCROLL_LOCK): return ScrollLock;
            case(GLFW_KEY_NUM_LOCK): return NumLock;
            case(GLFW_KEY_PRINT_SCREEN): return PrintScreen;
            case(GLFW_KEY_PAUSE): return Pause;
            case(GLFW_KEY_F1): return F1;
            case(GLFW_KEY_F2): return F2;
            case(GLFW_KEY_F3): return F3;
            case(GLFW_KEY_F4): return F4;
            case(GLFW_KEY_F5): return F5;
            case(GLFW_KEY_F6): return F6;
            case(GLFW_KEY_F7): return F7;
            case(GLFW_KEY_F8): return F8;
            case(GLFW_KEY_F9): return F9;
            case(GLFW_KEY_F10): return F10;
            case(GLFW_KEY_F11): return F11;
            case(GLFW_KEY_F12): return F12;
            case(GLFW_KEY_F13): return F13;
            case(GLFW_KEY_F14): return F14;
            case(GLFW_KEY_F15): return F15;
            case(GLFW_KEY_F16): return F16;
            case(GLFW_KEY_F17): return F17;
            case(GLFW_KEY_F18): return F18;
            case(GLFW_KEY_F19): return F19;
            case(GLFW_KEY_F20): return F20;
            case(GLFW_KEY_F21): return F21;
            case(GLFW_KEY_F22): return F22;
            case(GLFW_KEY_F23): return F23;
            case(GLFW_KEY_F24): return F24;
            case(GLFW_KEY_F25): return F25;
            case(GLFW_KEY_KP_0): return KP0;
            case(GLFW_KEY_KP_1): return KP1;
            case(GLFW_KEY_KP_2): return KP2;
            case(GLFW_KEY_KP_3): return KP3;
            case(GLFW_KEY_KP_4): return KP4;
            case(GLFW_KEY_KP_5): return KP5;
            case(GLFW_KEY_KP_6): return KP6;
            case(GLFW_KEY_KP_7): return KP7;
            case(GLFW_KEY_KP_8): return KP8;
            case(GLFW_KEY_KP_9): return KP9;
            case(GLFW_KEY_KP_DECIMAL): return KPDecimal;
            case(GLFW_KEY_KP_DIVIDE): return KPDivide;
            case(GLFW_KEY_KP_MULTIPLY): return KPMultiply;
            case(GLFW_KEY_KP_SUBTRACT): return KPSubtract;
            case(GLFW_KEY_KP_ADD): return KPAdd;
            case(GLFW_KEY_KP_ENTER): return KPEnter;
            case(GLFW_KEY_KP_EQUAL): return KPEqual;
            case(GLFW_KEY_LEFT_SHIFT): return LeftShift;
            case(GLFW_KEY_LEFT_CONTROL): return LeftControl;
            case(GLFW_KEY_LEFT_ALT): return LeftAlt;
            case(GLFW_KEY_LEFT_SUPER): return LeftSuper;
            case(GLFW_KEY_RIGHT_SHIFT): return RightShift;
            case(GLFW_KEY_RIGHT_CONTROL): return RightControl;
            case(GLFW_KEY_RIGHT_ALT): return RightAlt;
            case(GLFW_KEY_RIGHT_SUPER): return RightSuper;
            case(GLFW_KEY_MENU): return Menu;
            }

            return UnknownKey;
        }

        constexpr MouseButtonCodes ConvertGLFWMouseButton(i32 GLFWMouseButton)
        {
            switch (GLFWMouseButton)
            {
            case(GLFW_MOUSE_BUTTON_LEFT): return LeftButton;
            case(GLFW_MOUSE_BUTTON_RIGHT): return RightButton;
            case(GLFW_MOUSE_BUTTON_MIDDLE): return MiddleWheel;
            case(GLFW_MOUSE_BUTTON_4): return Button4;
            case(GLFW_MOUSE_BUTTON_5): return Button5;
            case(GLFW_MOUSE_BUTTON_6): return Button6;
            case(GLFW_MOUSE_BUTTON_7): return Button7;
            }
        }

        constexpr i32 ConvertToGLFWCursorMode(CursorModes CursorMode)
        {
            switch (CursorMode)
            {
            case(CursorNormal): return GLFW_CURSOR_NORMAL;
            case(CursorHidden): return GLFW_CURSOR_HIDDEN;
            case(CursorDisabled): return GLFW_CURSOR_DISABLED;
            case(CursorCapured): return GLFW_CURSOR_CAPTURED;
            }
        }

        void KeyCallBack(GLFWwindow* WindowInstance, int Key, int Scancode, int Action, int Mods)
        {
            KeyCodes KeyCode = ConvertGLFWKey(Key);

            if (Action == GLFW_PRESS)
            {
                Keys[KeyCode].Pressed = true;
                Keys[KeyCode].TimeSincePressed = glfwGetTime();
            }
            else if (Action == GLFW_RELEASE)
            {
                Keys[KeyCode].Released = true;
                Keys[KeyCode].Pressed = false;
                Keys[KeyCode].TimeSincePressed = 0;
            }
        }

        void CursorPositionCallBack(GLFWwindow* WindowInstance, double XPosition, double YPosition)
        {
            MousePosition.x = XPosition;
            MousePosition.y = YPosition;
        }

        void MouseButtonCallBack(GLFWwindow* WindowInstance, int Button, int Action, int Mods)
        {
            MouseButtonCodes MouseButtonCode = ConvertGLFWMouseButton(Button);

            if (Action == GLFW_PRESS)
            {
                MouseButtons[MouseButtonCode].Pressed = true;
                MouseButtons[MouseButtonCode].TimeSincePressed = glfwGetTime();
            }
            else if (Action == GLFW_RELEASE)
            {
                MouseButtons[MouseButtonCode].Released = true;
                MouseButtons[MouseButtonCode].Pressed = false;
                MouseButtons[MouseButtonCode].TimeSincePressed = 0;
            }
        }

        void ScrollCallBack(GLFWwindow* WindowInstance, double XOffset, double YOffset)
        {
            ScrollOffset += XOffset;
        }
    }
}