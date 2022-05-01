package ni.android.util;

import ni.niLang.EKey;
import android.view.KeyEvent;

public class Keys {
    static public final int[] mVKMap = new int[0xFF];

    static {
        for (int i = 0; i < 0xFF; ++i) {
            mVKMap[i] = EKey.Unknown;
        }

        mVKMap[KeyEvent.KEYCODE_DEL] = EKey.BackSpace;
        mVKMap[KeyEvent.KEYCODE_TAB] = EKey.Tab;
        mVKMap[KeyEvent.KEYCODE_ENTER] = EKey.Enter;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD_RETURN] = EKey.NumPadEnter;
        // mVKMap[KeyEvent.KEYCODE_PAUSE] = EKey.Pause;
        // mVKMap[KeyEvent.KEYCODE_ESCAPE] = EKey.Escape;
        mVKMap[KeyEvent.KEYCODE_SPACE] = EKey.Space;
        mVKMap[KeyEvent.KEYCODE_MENU] = EKey.Space;
        mVKMap[KeyEvent.KEYCODE_APOSTROPHE] = EKey.Apostrophe;
        mVKMap[KeyEvent.KEYCODE_COMMA] = EKey.Comma;
        mVKMap[KeyEvent.KEYCODE_MINUS] = EKey.Minus;
        mVKMap[KeyEvent.KEYCODE_PERIOD] = EKey.Period;
        mVKMap[KeyEvent.KEYCODE_SLASH] = EKey.Slash;
        mVKMap[KeyEvent.KEYCODE_0] = EKey.n0;
        mVKMap[KeyEvent.KEYCODE_1] = EKey.n1;
        mVKMap[KeyEvent.KEYCODE_2] = EKey.n2;
        mVKMap[KeyEvent.KEYCODE_3] = EKey.n3;
        mVKMap[KeyEvent.KEYCODE_4] = EKey.n4;
        mVKMap[KeyEvent.KEYCODE_5] = EKey.n5;
        mVKMap[KeyEvent.KEYCODE_6] = EKey.n6;
        mVKMap[KeyEvent.KEYCODE_7] = EKey.n7;
        mVKMap[KeyEvent.KEYCODE_8] = EKey.n8;
        mVKMap[KeyEvent.KEYCODE_9] = EKey.n9;
        mVKMap[KeyEvent.KEYCODE_SEMICOLON] = EKey.Semicolon;
        mVKMap[KeyEvent.KEYCODE_EQUALS] = EKey.Equals;
        mVKMap[KeyEvent.KEYCODE_LEFT_BRACKET] = EKey.LBracket;
        mVKMap[KeyEvent.KEYCODE_BACKSLASH] = EKey.BackSlash;
        mVKMap[KeyEvent.KEYCODE_RIGHT_BRACKET] = EKey.RBracket;
        mVKMap[KeyEvent.KEYCODE_GRAVE] = EKey.Grave;
        mVKMap[KeyEvent.KEYCODE_A] = EKey.A;
        mVKMap[KeyEvent.KEYCODE_B] = EKey.B;
        mVKMap[KeyEvent.KEYCODE_C] = EKey.C;
        mVKMap[KeyEvent.KEYCODE_D] = EKey.D;
        mVKMap[KeyEvent.KEYCODE_E] = EKey.E;
        mVKMap[KeyEvent.KEYCODE_F] = EKey.F;
        mVKMap[KeyEvent.KEYCODE_G] = EKey.G;
        mVKMap[KeyEvent.KEYCODE_H] = EKey.H;
        mVKMap[KeyEvent.KEYCODE_I] = EKey.I;
        mVKMap[KeyEvent.KEYCODE_J] = EKey.J;
        mVKMap[KeyEvent.KEYCODE_K] = EKey.K;
        mVKMap[KeyEvent.KEYCODE_L] = EKey.L;
        mVKMap[KeyEvent.KEYCODE_M] = EKey.M;
        mVKMap[KeyEvent.KEYCODE_N] = EKey.N;
        mVKMap[KeyEvent.KEYCODE_O] = EKey.O;
        mVKMap[KeyEvent.KEYCODE_P] = EKey.P;
        mVKMap[KeyEvent.KEYCODE_Q] = EKey.Q;
        mVKMap[KeyEvent.KEYCODE_R] = EKey.R;
        mVKMap[KeyEvent.KEYCODE_S] = EKey.S;
        mVKMap[KeyEvent.KEYCODE_T] = EKey.T;
        mVKMap[KeyEvent.KEYCODE_U] = EKey.U;
        mVKMap[KeyEvent.KEYCODE_V] = EKey.V;
        mVKMap[KeyEvent.KEYCODE_W] = EKey.W;
        mVKMap[KeyEvent.KEYCODE_X] = EKey.X;
        mVKMap[KeyEvent.KEYCODE_Y] = EKey.Y;
        mVKMap[KeyEvent.KEYCODE_Z] = EKey.Z;

        // mVKMap[KeyEvent.KEYCODE_NUMPAD0] = EKey.NumPad0;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD1] = EKey.NumPad1;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD2] = EKey.NumPad2;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD3] = EKey.NumPad3;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD4] = EKey.NumPad4;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD5] = EKey.NumPad5;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD6] = EKey.NumPad6;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD7] = EKey.NumPad7;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD8] = EKey.NumPad8;
        // mVKMap[KeyEvent.KEYCODE_NUMPAD9] = EKey.NumPad9;
        // mVKMap[KeyEvent.KEYCODE_DECIMAL] = EKey.NumPadPeriod;
        // mVKMap[KeyEvent.KEYCODE_DIVIDE] = EKey.NumPadSlash;
        // mVKMap[KeyEvent.KEYCODE_MULTIPLY] = EKey.NumPadStar;
        // mVKMap[KeyEvent.KEYCODE_SUBTRACT] = EKey.NumPadMinus;
        // mVKMap[KeyEvent.KEYCODE_ADD] = EKey.NumPadPlus;

        //mVKMap[KeyEvent.KEYCODE_VOLUME_UP] = EKey.Up;
        //mVKMap[KeyEvent.KEYCODE_VOLUME_DOWN] = EKey.Down;

        //mVKMap[KeyEvent.KEYCODE_MEDIA_NEXT] = EKey.PgUp;
        //mVKMap[KeyEvent.KEYCODE_MEDIA_PREVIOUS] = EKey.PgDn;

        mVKMap[KeyEvent.KEYCODE_DPAD_UP] = EKey.Up;
        mVKMap[KeyEvent.KEYCODE_DPAD_DOWN] = EKey.Down;
        mVKMap[KeyEvent.KEYCODE_DPAD_RIGHT] = EKey.Right;
        mVKMap[KeyEvent.KEYCODE_DPAD_LEFT] = EKey.Left;
        // mVKMap[KeyEvent.KEYCODE_INSERT] = EKey.Insert;
        // mVKMap[KeyEvent.KEYCODE_HOME] = EKey.Home;
        // mVKMap[KeyEvent.KEYCODE_END] = EKey.End;
        // mVKMap[KeyEvent.KEYCODE_PRIOR] = EKey.PgUp;
        // mVKMap[KeyEvent.KEYCODE_NEXT] = EKey.PgDn;

        // mVKMap[KeyEvent.KEYCODE_F1] = EKey.F1;
        // mVKMap[KeyEvent.KEYCODE_F2] = EKey.F2;
        // mVKMap[KeyEvent.KEYCODE_F3] = EKey.F3;
        // mVKMap[KeyEvent.KEYCODE_F4] = EKey.F4;
        // mVKMap[KeyEvent.KEYCODE_F5] = EKey.F5;
        // mVKMap[KeyEvent.KEYCODE_F6] = EKey.F6;
        // mVKMap[KeyEvent.KEYCODE_F7] = EKey.F7;
        // mVKMap[KeyEvent.KEYCODE_F8] = EKey.F8;
        // mVKMap[KeyEvent.KEYCODE_F9] = EKey.F9;
        // mVKMap[KeyEvent.KEYCODE_F10] = EKey.F10;
        // mVKMap[KeyEvent.KEYCODE_F11] = EKey.F11;
        // mVKMap[KeyEvent.KEYCODE_F12] = EKey.F12;
        // mVKMap[KeyEvent.KEYCODE_F13] = EKey.F13;
        // mVKMap[KeyEvent.KEYCODE_F14] = EKey.F14;
        // mVKMap[KeyEvent.KEYCODE_F15] = EKey.F15;

        // mVKMap[KeyEvent.KEYCODE_NUMLOCK] = EKey.NumLock;
        // mVKMap[KeyEvent.KEYCODE_CAPITAL] = EKey.CapsLock;
        // mVKMap[KeyEvent.KEYCODE_SCROLL] = EKey.Scroll;
        mVKMap[KeyEvent.KEYCODE_SHIFT_LEFT] = EKey.RShift;
        mVKMap[KeyEvent.KEYCODE_SHIFT_RIGHT] = EKey.LShift;        
        mVKMap[KeyEvent.KEYCODE_ALT_RIGHT] = EKey.RAlt;
        mVKMap[KeyEvent.KEYCODE_ALT_LEFT] = EKey.LAlt;
        // mVKMap[KeyEvent.KEYCODE_RWIN] = EKey.RWin;
        // mVKMap[KeyEvent.KEYCODE_LWIN] = EKey.LWin;
        // mVKMap[KeyEvent.KEYCODE_SNAPSHOT] = EKey.PrintScreen;
        // mVKMap[KeyEvent.KEYCODE_CANCEL] = EKey.Pause;
        // mVKMap[KeyEvent.KEYCODE_APPS] = EKey.Apps;

        // Key codes I got by testing with an actual keyboard...
        mVKMap[113] = EKey.LControl;
        mVKMap[114] = EKey.RControl;
    }
}
