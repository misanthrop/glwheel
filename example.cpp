#include "window.hpp"

using namespace wheel;

struct example : window
{
	float x = 0, y = 0;

	example(application& app) : window(app, "glwheel") {}

	void press(uint8_t k)
	{
		switch(k)
		{
			case key::lbutton:
				x = 2.0*pointer().x/width() - 1;
				y = 1 - 2.0*pointer().y/height();
				break;
			case key::f11: togglefullscreen(); break;
			case key::esc: close(); break;
		}
	}

	void draw()
	{
		glClearColor((float)pointer().x/width(), 0, (float)pointer().y/height(), 1);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glViewport(0,0,width(),height());
		flip();
	}
};

int main()
{
	application app;
	example wnd(app);
	wnd.show(true);
	while(app)						// while have windows
	{
		wnd.update();				// does nothing by default
		if(wnd.show())				// if window is visible
			wnd.draw();				// clear, draw children, swap buffers
		app.process(1000);
	}
	return 0;
}
