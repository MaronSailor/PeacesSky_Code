#include "inputHandler.hpp"

void InputHandler::updateKey(int keyCode, bool value)
{
	switch (keyCode)
	{
		case 87:
		{
			keys.w = value;
			break;
		}
		case 65:
		{
			keys.a = value;
			break;
		}
		case 83:
		{
			keys.s = value;
			break;
		}
		case 68:
		{
			keys.d = value;
			break;
		}
		case 32:
		{
			keys.space = value;
			break;
		}
		case 256:
		{
			keys.esc = value;
			break;
		}
		case 340:
		{
			keys.lshift = value;
			break;
		}
		case 341:
		{
			keys.lctrl = value;
			break;
		}
		case 86:
		{
			keys.v = value;
			break;
		}
		case 82:
		{
			keys.r = value;
			break;
		}
		case 67:
		{
			keys.c = value;
			break;
		}
		case 49:
		{
			keys.one = value;
			break;
		}
		case 50:
		{
			keys.two = value;
			break;
		}
		case 51:
		{
			keys.three = value;
			break;
		}
		case 52:
		{
			keys.four = value;
			break;
		}
		case 53:
		{
			keys.five = value;
			break;
		}
	}
}

void InputHandler::updateMousePosition(float x, float y)
{
	mouse.x = x;
	mouse.y = y;
}

void InputHandler::updateMouseButton(int keyCode, bool value)
{
	switch (keyCode)
	{
		case 0:
		{
			mouse.lMouseButton = value;
			break;
		}
		case 1:
		{
			mouse.rMouseButton = value;
			break;
		}
	}
}

void InputHandler::updateScrollWheel(float y)
{
	mouse.scroll -= y;
}