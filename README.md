# uevr-joytokey
Controller to keypress mapper for UEVR
Sample joytokey.txt config file below:

# All keys are use the hex code starting with 0x in the windows virtual key list below.
# Letters A-Z and 0-9 can be used in place of the hex codes. All others must be the hex code.
# Hex must start with 0x. Not 0X, nothing else except 0x
#
# The list is here: 
# https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
# 
# For example, to set to A, you can use A or 0x41. To set to 5, use 5 or 0x35. To set to DEL, use 0x2E
# The single keys can only be used for A-Z, 0-9. All other keys must use the hex code from the link above.
# Some common key codes are:
#   Backspace: 	0x08
#   Tab: 	0x09
#   Enter:	0x0D
#   Escape: 	0x1B
#   Space:	0x20
#   Ins:	0x2D
#   Del:	0x2E
#   Arrows: (left, up, right, down) 0x25, 0x26, 0x27, 0x28
#   Numpad 0 - Numpad 9: 0x60 - 0x69
#   F1 - F10:  0x70 - 0x79
# 
# Leaving a button unassigned or setting to 0 disables it.
#
# Mouse
# You can set mouse clicks using the same link above:
#   Left Button: 	0x01
#   Right Button: 	0x02
#   Middle Button:	0x04
#   XButton1: 		0x05
#   XButton2: 		0x06
#
# If left unassigned, the right stick maps to the mouse pointer. There is currently
# no sensitivity adjustments for this. You can assign the sticks to buttons or directly
# to 0 to disable the mouse. Leaving it empty defaults to right stick moving mouse.
#
# Shift Key
# GAMEPAD_LB can be shift so leave it unassigned if you wish to use it as such.
# This will allow for a second set of keys while holding shift
#
GAMEPAD_LB=0x46
GAMEPAD_DPAD_UP=W
GAMEPAD_DPAD_DOWN=S
GAMEPAD_DPAD_LEFT=A
GAMEPAD_DPAD_RIGHT=D
GAMEPAD_START=0x0D
GAMEPAD_BACK=0x1B
GAMEPAD_L3=0x09
GAMEPAD_R3=
GAMEPAD_RT=0x01
GAMEPAD_RB=0x04
GAMEPAD_A=0x49
GAMEPAD_B=0x45
GAMEPAD_X=0x20
GAMEPAD_Y=
GAMEPAD_LSTICK_UP=W
GAMEPAD_LSTICK_DOWN=S
GAMEPAD_LSTICK_LEFT=A
GAMEPAD_LSTICK_RIGHT=D
GAMEPAD_RSTICK_UP=
GAMEPAD_RSTICK_DOWN=
GAMEPAD_RSTICK_LEFT=
GAMEPAD_RSTICK_RIGHT=
GAMEPAD_LT=0x02

# If you want to use any of these, which are active while holding down LB,
# make sure you do not assign GAMEPAD_LB above.
GAMEPAD_LB_DPAD_UP=
GAMEPAD_LB_DPAD_DOWN=
GAMEPAD_LB_DPAD_LEFT=
GAMEPAD_LB_DPAD_RIGHT=
GAMEPAD_LB_START=
GAMEPAD_LB_BACK=
GAMEPAD_LB_L3=
GAMEPAD_LB_R3=
GAMEPAD_LB_RT=
GAMEPAD_LB_RB=
GAMEPAD_LB_A=
GAMEPAD_LB_B=
GAMEPAD_LB_X=
GAMEPAD_LB_Y=
GAMEPAD_LB_LSTICK_UP=
GAMEPAD_LB_LSTICK_DOWN=
GAMEPAD_LB_LSTICK_LEFT=
GAMEPAD_LB_LSTICK_RIGHT=
GAMEPAD_LB_RSTICK_UP=
GAMEPAD_LB_RSTICK_DOWN=
GAMEPAD_LB_RSTICK_LEFT=
GAMEPAD_LB_RSTICK_RIGHT=
GAMEPAD_LB_LT=
