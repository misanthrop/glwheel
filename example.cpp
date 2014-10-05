#include "impl.hpp"
using namespace wheel;

struct example : widget
{
	void resize() { set(0,0,parent->width(),parent->height()); }

	void press(uint8_t k)
	{
		switch(k)
		{
			case key::lbutton: break; // left mouse button or touch screen
			case key::f11: app.togglefullscreen(); break;
			case key::esc: app.close(); break;
		}
	}

	void draw()
	{
		glClearColor((float)pointer().x/width(), 0, (float)pointer().y/height(), 1);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}
};

int main()
{
	app.init("glwheel");
	example wnd;
	app.add(&wnd);
	app.show(true);
	while(app.alive())
	{
		if(app)			// if window is visible
			app.draw();	// clear, draw children, swap buffers
		app.process(100);
	}
	return 0;
}
