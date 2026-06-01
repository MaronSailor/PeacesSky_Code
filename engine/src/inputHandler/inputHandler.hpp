struct Keys
{
	bool space = false;
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;
	bool v = false;
	bool r = false;
	bool c = false;
	bool esc = false;
	bool lshift = false;
	bool lctrl = false;
	bool one = false;
	bool two = false;
	bool three = false;
	bool four = false;
	bool five = false;
};

struct Mouse
{
	float x = 0;
	float y = 0;
	bool lMouseButton = false;
	bool rMouseButton = false;
	float scroll = 0;
};

class InputHandler
{
public:
	Keys keys;
	Mouse mouse;

	void updateKey(int keyCode, bool value);
	void updateMousePosition(float x, float y);
	void updateMouseButton(int keyCode, bool value);
	void updateScrollWheel(float y);
};