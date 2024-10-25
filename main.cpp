#define  _CRT_SECURE_NO_WARNINGS 1
#define  _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <cctype>

#include "uevr/Plugin.hpp"
#include "main.h"


void DebugPrint(char* Format, ...);
using namespace uevr;

#define PLUGIN_LOG_ONCE(...) \
    static bool _logged_ = false; \
    if (!_logged_) { \
        _logged_ = true; \
        API::get()->log_info(__VA_ARGS__); \
    }
    
class JoyToKey : public uevr::Plugin {
public:
    KEY_ITEM m_Keys[TOTAL_BUTTONS];
    std::string m_Path;

    JoyToKey() = default;
	
    const UEVR_PluginInitializeParam* m_Param;
    const UEVR_VRData* m_VR;
	
    void on_dllmain(HANDLE handle) override {
         StoreConfigFileLocation(handle);
   }

    void on_initialize() override {
      API::get()->log_info("JoytoKey.dll: Config file should be: %s\n", m_Path.c_str());  
      ReadConfig(m_Path);
    }

    void send_key_or_mouse(int Index, bool key_up) {
        if(m_Keys[Index].Mouse == true)
        {
            send_mouse(m_Keys[Index].Key, key_up);
        }
        else
        {
            send_key(m_Keys[Index].Key, key_up);
        }
    }
    

    void MoveMousePointer(SHORT& StickValue, bool Horizontal) {
		int Value = StickValue;
		
        // Clamp Value to the range -20000 to +20000
        if (Value < -20000) Value = -20000;
        if (Value > 20000) Value = 20000;
        
        // Flip the axis. The joystick is inverted to the mouse + vs -
        if (Horizontal == false) Value = Value * -1;
        
        // Dead zone 15%
        if (Value > 0 && Value < 3000) return;
        if (Value < 0 && Value > -3000) return;
        
        if (Value < 5000 && Value > -5000) Value = Value / 2;
        if (Value > 15000 || Value < -15000) Value = (int)((Value * 3)/2);

        if(Horizontal == false) Value = (Value / 3) * 2;
        // Scale Value to the range -10 to +10
        float scaledValue = (static_cast<float>(Value) / 2000.0f);

        // Calculate new position based on Horizontal flag
        int newX = (Horizontal ? static_cast<int>(scaledValue) : 0);
        int newY = (Horizontal ? 0 : static_cast<int>(scaledValue));

        // Create an INPUT structure for the mouse movement
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dx = newX; // Move relative
        input.mi.dy = newY; // Move relative
        input.mi.dwFlags = MOUSEEVENTF_MOVE; // Use absolute movement

        // Send the input to move the mouse
        SendInput(1, &input, sizeof(INPUT));
		
		// Clear stick value
		Value = 0;
    }

    void send_key(WORD key, bool key_up) {
        INPUT input;
        ZeroMemory(&input, sizeof(INPUT));
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key;
        if(key_up) input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
    
    void send_mouse(WORD key, bool key_up) {
        INPUT input;
        ZeroMemory(&input, sizeof(INPUT));
        input.type = INPUT_MOUSE;
        
        // Handle mouse button events
        if(key == VK_LBUTTON) {
            input.mi.dwFlags = (key_up ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_LEFTDOWN);
        }
        else if(key == VK_RBUTTON) {
            input.mi.dwFlags = (key_up ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_RIGHTDOWN);
        }
        else if(key == VK_MBUTTON) {
            input.mi.dwFlags = (key_up ? MOUSEEVENTF_MIDDLEUP : MOUSEEVENTF_MIDDLEDOWN);
        }
        else if(key == VK_XBUTTON1) {
            input.mi.dwFlags = (key_up ? MOUSEEVENTF_XUP : MOUSEEVENTF_XDOWN);
            input.mi.mouseData = XBUTTON1;
        }
        else if(key == VK_XBUTTON2) {
            input.mi.dwFlags = (key_up ? MOUSEEVENTF_XUP : MOUSEEVENTF_XDOWN);
            input.mi.mouseData = XBUTTON2;
        }
        // Handle mouse wheel events
        else if(key == 0x0A) { // Wheel up
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            input.mi.mouseData = WHEEL_DELTA; // Positive for wheel up
        }
        else if(key == 0x0B) { // Wheel down
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            input.mi.mouseData = -WHEEL_DELTA; // Negative for wheel down
        }

        SendInput(1, &input, sizeof(INPUT));
    }

    //*******************************************************************************************
    // This is the controller input routine. Everything happens here.
    //*******************************************************************************************
    void on_xinput_get_state(uint32_t* retval, uint32_t user_index, XINPUT_STATE* state) 
    {
        int i = 0;
        int Start = 0;
        int Limit = GAMEPAD_NUMBER_OF_BUTTONS;
        if(state != NULL)
        {
            // if using shift key
            if(m_Keys[GAMEPAD_LB].Key == 0)
            {
                // LB is not down, so we need to clear every LB key
                if(!(state->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER))
                {
                    // OK If LB is not down, go through every shift key and if one is down, release it
                    for(i = GAMEPAD_NUMBER_OF_BUTTONS; i < TOTAL_BUTTONS; i++)
                    {
                        if(m_Keys[i].KeyDown == true)
                        {
                            send_key_or_mouse(i, KEYUP);
                            m_Keys[i].KeyDown = false;
                        }
                    }
                }
                
                // LB is down, so we need to clear every regular key.
                else
                {
                    // OK If LB not down, go through every shift key and if one is down, release it
                    for(i = 1; i < GAMEPAD_NUMBER_OF_BUTTONS; i++)
                    {
                        if(m_Keys[i].KeyDown == true)
                        {
                            send_key_or_mouse(i, KEYUP);
                            m_Keys[i].KeyDown = false;
                        }
                    }
                    Start = GAMEPAD_NUMBER_OF_BUTTONS;
                    Limit = TOTAL_BUTTONS;
                }
            }
            
            // go through each xinput button. If it's down check if we need to set it. If its not, check if we need to clear it
            SetKeyForGamepad(state, Start + GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_UP);
            SetKeyForGamepad(state, Start + GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN);
            SetKeyForGamepad(state, Start + GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT);
            SetKeyForGamepad(state, Start + GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT);
           
            SetKeyForGamepad(state, Start + GAMEPAD_START, XINPUT_GAMEPAD_START);
            SetKeyForGamepad(state, Start + GAMEPAD_BACK, XINPUT_GAMEPAD_BACK);
            SetKeyForGamepad(state, Start + GAMEPAD_L3, XINPUT_GAMEPAD_LEFT_THUMB);
            SetKeyForGamepad(state, Start + GAMEPAD_R3, XINPUT_GAMEPAD_RIGHT_THUMB);
            
            SetKeyForGamepad(state, Start + GAMEPAD_LB, XINPUT_GAMEPAD_LEFT_SHOULDER);
            SetKeyForGamepad(state, Start + GAMEPAD_RB, XINPUT_GAMEPAD_RIGHT_SHOULDER);
            
            SetKeyForGamepad(state, Start + GAMEPAD_A, XINPUT_GAMEPAD_A);
            SetKeyForGamepad(state, Start + GAMEPAD_B, XINPUT_GAMEPAD_B);
            SetKeyForGamepad(state, Start + GAMEPAD_X, XINPUT_GAMEPAD_X);
            SetKeyForGamepad(state, Start + GAMEPAD_Y, XINPUT_GAMEPAD_Y);
            
            SetKeyForAxis(state->Gamepad.bRightTrigger, 200, Start + GAMEPAD_RT);
            SetKeyForAxis(state->Gamepad.bLeftTrigger, 200, Start + GAMEPAD_LT);

            SetKeyForAxis(state->Gamepad.sThumbLX, 15000, Start + GAMEPAD_LSTICK_RIGHT);
            SetKeyForAxis(state->Gamepad.sThumbLX, -15000, Start + GAMEPAD_LSTICK_LEFT);
            SetKeyForAxis(state->Gamepad.sThumbLY, 15000, Start + GAMEPAD_LSTICK_UP);
            SetKeyForAxis(state->Gamepad.sThumbLY, -15000, Start + GAMEPAD_LSTICK_DOWN);
            
            // If right axis isn't overridden, use it as a mouse pointer.
            if(m_Keys[Start + GAMEPAD_RSTICK_RIGHT].Key == 0 && m_Keys[Start + GAMEPAD_RSTICK_LEFT].Key == 0)
            {
                MoveMousePointer(state->Gamepad.sThumbRX, true);
            }
            else
            {
                SetKeyForAxis(state->Gamepad.sThumbRX, 15000, Start + GAMEPAD_RSTICK_RIGHT);
                SetKeyForAxis(state->Gamepad.sThumbRX, -15000, Start + GAMEPAD_RSTICK_LEFT);
            }
            if(m_Keys[Start + GAMEPAD_RSTICK_UP].Key == 0 && m_Keys[Start + GAMEPAD_RSTICK_DOWN].Key == 0)
            {
                MoveMousePointer(state->Gamepad.sThumbRY, false);
            }
            else
            {
                SetKeyForAxis(state->Gamepad.sThumbRY, 15000, Start + GAMEPAD_RSTICK_UP);
                SetKeyForAxis(state->Gamepad.sThumbRY, -15000, Start + GAMEPAD_RSTICK_DOWN);
            }
        }
    }

    void SetKeyForAxis(SHORT& Axis, int CompareValue, int i)
    {
		if(_SetKeyForAxis(Axis, CompareValue, i) == true) Axis = 0;
	}
	
    void SetKeyForAxis(BYTE& Axis, int CompareValue, int i)
    {
		if(_SetKeyForAxis(Axis, CompareValue, i) == true) Axis = 0;
	}

	//***************************************************************************************************
    // Sets the key up or down for the axis values
	//***************************************************************************************************
    bool _SetKeyForAxis(int Axis, int CompareValue, int i)
    {
		bool KeyExists = false;
		if(m_Keys[i].Key)
		{
			bool ConsiderDown = false;
			
			if(CompareValue > 0)
			{
				if(Axis > CompareValue) ConsiderDown = true;
			}
			else 
			{
				if(Axis < CompareValue) ConsiderDown = true;
			}
			
			if(ConsiderDown == true)
			{
				if(m_Keys[i].KeyDown == false)
				{
					m_Keys[i].KeyDown = true;
					send_key_or_mouse(i, KEYDOWN);
                    KeyExists = true;
				}
			}
			else
			{
				if(m_Keys[i].KeyDown == true)
				{
					m_Keys[i].KeyDown = false;
					send_key_or_mouse(i, KEYUP);
				}
			}
		}
		
		return KeyExists;
    }

	//***************************************************************************************************
    // Sets the key up or down for the wbutton values
	//***************************************************************************************************
    void SetKeyForGamepad(XINPUT_STATE* state, int i, int Mask)
    {
		if(m_Keys[i].Key)
		{
			if(state->Gamepad.wButtons & Mask)
			{
				if(m_Keys[i].KeyDown == false)
				{
					m_Keys[i].KeyDown = true;
					send_key_or_mouse(i, KEYDOWN);
                    // Clear the gamepad button.
                    state->Gamepad.wButtons &= ~(Mask);
				}
			}
			else
			{
				if(m_Keys[i].KeyDown == true)
				{
					m_Keys[i].KeyDown = false;
					send_key_or_mouse(i, KEYUP);
				}
			}
			
		}
    }
    
	//***************************************************************************************************
	// Stores the path and file location of the cvar.txt config file.
	//***************************************************************************************************
	void StoreConfigFileLocation(HANDLE handle) {
		wchar_t wide_path[MAX_PATH]{};
		if (GetModuleFileNameW((HMODULE)handle, wide_path, MAX_PATH)) {
			const auto path = std::filesystem::path(wide_path).parent_path() / "joytokey.txt";
			m_Path = path.string(); // change m_Path to a std::string
		}
	}	

	//***************************************************************************************************
	// Reads the config file cvars.txt and stores it in a linked list of CVAR_ITEMs.
	//***************************************************************************************************
    void ReadConfig(std::string ConfigFile) {
		std::string Line;
		
		int Length = 0;
		int i = 0;
        int LineNumber = 0;
		size_t Pos = 0;
		
        for(i = 0; i < TOTAL_BUTTONS; i++)
        {
            ZeroMemory(&m_Keys[i], sizeof(KEY_ITEM));
        }
        
		std::wstring_convert<std::codecvt_utf8<wchar_t>> Converter;
		
        
        API::get()->log_info("joytokey.dll: reading config file %s", ConfigFile.c_str());
		std::ifstream fileStream(ConfigFile.c_str());
		if(!fileStream.is_open()) {
			API::get()->log_error("joytokey.dll: %s cannot be opened or does not exist.", ConfigFile.c_str());
			return;
		}
		
			
		while (std::getline(fileStream, Line)) {
            LineNumber++;

            Length = static_cast<int>(Line.length());

			if(Line[0] == '#') continue;
			if(Line[0] == ';') continue;
			if(Line[0] == '[') continue;
			if(Line[0] == ' ') continue;
			if(Length < 3) continue;
			
			// Strip  spaces, carriage returns from line.
			Pos = Line.find_last_not_of(" \r\n");
			if(Pos != std::string::npos) {
				Line.erase(Pos + 1);
			}

			Pos = Line.find('=');
			if(Pos == std::string::npos) {
				API::get()->log_info("joytokey.dll: Invalid line, no = sign found or nothing after = found");   
				continue;
			}

			//API::get()->log_info("joytokey.dll: Line %d was %d long", LineNumber, Length);   
			API::get()->log_info("joytokey.dll: Processing config line: %s", Line.c_str());   

			// At this point, we are convinced we have a valid entry, find the key item to store it in.
            int KeyNumber = GetKeyNumber(Line);
            API::get()->log_info("joytokey.dll:  GetKeyNumber returned, key=%d", KeyNumber);
            if(KeyNumber >= 0 && KeyNumber < TOTAL_BUTTONS)
            {
                API::get()->log_info("joytokey.dll:  in if calling GetKeyFromLine");
                m_Keys[KeyNumber].Key = GetKeyFromLine(Line);
                API::get()->log_info("joytokey.dll:  in if returned GetKeyFromLine");
                if(m_Keys[KeyNumber].Key > 0 && m_Keys[KeyNumber].Key < 0x08) m_Keys[KeyNumber].Mouse = true;
                if(m_Keys[KeyNumber].Key == 0x0A || m_Keys[KeyNumber].Key == 0x0B) m_Keys[KeyNumber].Mouse = true;
                
                API::get()->log_info("joytokey.dll: Added entry: %ls, %d=0x%02F", Line.c_str(), KeyNumber, m_Keys[KeyNumber].Key);
            }
		}		
		
		fileStream.close();
	}


	unsigned char GetKeyFromLine(const std::string& Line) {
		size_t pos = Line.find("=");
		if (pos != std::string::npos) {
			// Extract the part after "="
			std::string value = Line.substr(pos + 1);
			
			// Check if value is empty after "="
			if (value.empty()) {
				return 0; // Return a default value or handle as necessary
			}

			// Check if value is a single character
			if (value.size() == 1) {
				char key = value[0];
				// Accept only A-Z and 0-9
				if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
					return static_cast<unsigned char>(key); // Return the character as virtual key code
				} else if (std::islower(key) && (key >= 'a' && key <= 'z')) {
					// Convert lowercase a-z to uppercase
					return static_cast<unsigned char>(std::toupper(key));
				}
				// Ignore all other single characters
				return 0;
			}

			// Handle hex values with optional "0x" prefix
			if (value.size() > 2 && value.substr(0, 2) == "0x") {
				value = value.substr(2); // Remove "0x" prefix if present
			}

			// Try converting the hex string to an integer
			try {
				int intValue = std::stoi(value, nullptr, 16);
				if (intValue >= 0 && intValue <= 255) { // Ensure within unsigned char range
					return static_cast<unsigned char>(intValue);
				} else {
					return 0; // Handle out of range values
				}
			} catch (const std::invalid_argument&) {
				return 0; // Handle invalid hex values
			} catch (const std::out_of_range&) {
				return 0; // Handle out of range values
			}
		}

		return 0; // Return a default value if no "=" is found
	}

	//***************************************************************************************************
	// Returns the number for the hex string as a char so "0x2F" returns 0x2F
	//***************************************************************************************************
    unsigned char GetKeyFromLineOld(const std::string& Line) {
        size_t pos = Line.find("=");
        if (pos != std::string::npos) {
            // Extract the part after "="
            std::string hexValue = Line.substr(pos + 1);
            
            // Convert the hex string to an integer (base 16) and then to unsigned char
            int intValue = std::stoi(hexValue, nullptr, 16);
            return static_cast<unsigned char>(intValue);
        } else {
            return 0;
        }
    }
    
	//***************************************************************************************************
	// Returns the number for the key string.
	//***************************************************************************************************
    int GetKeyNumber(const std::string& Line) {
        int KeyNumber = -1;
        
        if(Line.find("GAMEPAD_LB=") != std::string::npos) KeyNumber = 0;
        else if(Line.find("GAMEPAD_DPAD_UP=") != std::string::npos) KeyNumber = 1;
        else if(Line.find("GAMEPAD_DPAD_DOWN=") != std::string::npos) KeyNumber = 2;
        else if(Line.find("GAMEPAD_DPAD_LEFT=") != std::string::npos) KeyNumber = 3;
        else if(Line.find("GAMEPAD_DPAD_RIGHT=") != std::string::npos) KeyNumber = 4;
        else if(Line.find("GAMEPAD_START=") != std::string::npos) KeyNumber = 5;
        else if(Line.find("GAMEPAD_BACK=") != std::string::npos) KeyNumber = 6;
        else if(Line.find("GAMEPAD_L3=") != std::string::npos) KeyNumber = 7;
        else if(Line.find("GAMEPAD_R3=") != std::string::npos) KeyNumber = 8;
        else if(Line.find("GAMEPAD_RT=") != std::string::npos) KeyNumber = 9;
        else if(Line.find("GAMEPAD_RB=") != std::string::npos) KeyNumber = 10;
        else if(Line.find("GAMEPAD_A=") != std::string::npos) KeyNumber = 11;
        else if(Line.find("GAMEPAD_B=") != std::string::npos) KeyNumber = 12;
        else if(Line.find("GAMEPAD_X=") != std::string::npos) KeyNumber = 13;
        else if(Line.find("GAMEPAD_Y=") != std::string::npos) KeyNumber = 14;
        else if(Line.find("GAMEPAD_LSTICK_UP=") != std::string::npos) KeyNumber = 15;
        else if(Line.find("GAMEPAD_LSTICK_DOWN=") != std::string::npos) KeyNumber = 16;
        else if(Line.find("GAMEPAD_LSTICK_LEFT=") != std::string::npos) KeyNumber = 17;
        else if(Line.find("GAMEPAD_LSTICK_RIGHT=") != std::string::npos) KeyNumber = 18;
        else if(Line.find("GAMEPAD_RSTICK_UP=") != std::string::npos) KeyNumber = 19;
        else if(Line.find("GAMEPAD_RSTICK_DOWN=") != std::string::npos) KeyNumber = 20;
        else if(Line.find("GAMEPAD_RSTICK_LEFT=") != std::string::npos) KeyNumber = 21;
        else if(Line.find("GAMEPAD_RSTICK_RIGHT=") != std::string::npos) KeyNumber = 22;
        else if(Line.find("GAMEPAD_LT=") != std::string::npos) KeyNumber = 23;
        
        else if(Line.find("GAMEPAD_LB_DPAD_UP=") != std::string::npos) KeyNumber = 25;
        else if(Line.find("GAMEPAD_LB_DPAD_DOWN=") != std::string::npos) KeyNumber = 26;
        else if(Line.find("GAMEPAD_LB_DPAD_LEFT=") != std::string::npos) KeyNumber = 27;
        else if(Line.find("GAMEPAD_LB_DPAD_RIGHT=") != std::string::npos) KeyNumber = 28;
        else if(Line.find("GAMEPAD_LB_START=") != std::string::npos) KeyNumber = 29;
        else if(Line.find("GAMEPAD_LB_BACK=") != std::string::npos) KeyNumber = 30;
        else if(Line.find("GAMEPAD_LB_L3=") != std::string::npos) KeyNumber = 31;
        else if(Line.find("GAMEPAD_LB_R3=") != std::string::npos) KeyNumber = 32;
        else if(Line.find("GAMEPAD_LB_RT=") != std::string::npos) KeyNumber = 33;
        else if(Line.find("GAMEPAD_LB_RB=") != std::string::npos) KeyNumber = 34;
        else if(Line.find("GAMEPAD_LB_A=") != std::string::npos) KeyNumber = 35;
        else if(Line.find("GAMEPAD_LB_B=") != std::string::npos) KeyNumber = 36;
        else if(Line.find("GAMEPAD_LB_X=") != std::string::npos) KeyNumber = 37;
        else if(Line.find("GAMEPAD_LB_Y=") != std::string::npos) KeyNumber = 38;
        else if(Line.find("GAMEPAD_LB_LSTICK_UP=") != std::string::npos) KeyNumber = 39;
        else if(Line.find("GAMEPAD_LB_LSTICK_UP=") != std::string::npos) KeyNumber = 40;
        else if(Line.find("GAMEPAD_LB_LSTICK_UP=") != std::string::npos) KeyNumber = 41;
        else if(Line.find("GAMEPAD_LB_LSTICK_UP=") != std::string::npos) KeyNumber = 42;
        else if(Line.find("GAMEPAD_LB_RSTICK_UP=") != std::string::npos) KeyNumber = 43;
        else if(Line.find("GAMEPAD_LB_RSTICK_UP=") != std::string::npos) KeyNumber = 44;
        else if(Line.find("GAMEPAD_LB_RSTICK_UP=") != std::string::npos) KeyNumber = 45;
        else if(Line.find("GAMEPAD_LB_RSTICK_UP=") != std::string::npos) KeyNumber = 46;
        else if(Line.find("GAMEPAD_LB_LT=") != std::string::npos) KeyNumber = 47;
        
        return KeyNumber;
    }
    
};
// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<JoyToKey> g_plugin{new JoyToKey()};

void DebugPrint(char* Format, ...)
{
  char FormattedMessage[512];    
  va_list ArgPtr = NULL;  
  
  /* Generate the formatted debug message. */        
  va_start(ArgPtr, Format);        
  vsprintf(FormattedMessage, Format, ArgPtr);        
  va_end(ArgPtr); 

  OutputDebugString(FormattedMessage);
}

