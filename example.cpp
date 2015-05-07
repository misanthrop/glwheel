#include "impl.hpp"	// include to one of your .cpp files
using namespace wheel;

int main()
{
	application app("glwheel example");	// UTF-8 strings are supported
	app.pressed = [&](key k) { if(k == key::f11) app.togglefullscreen(); };
	app.show();
	while(app.alive())	// while not closed
	{
		if(app.visible())	// draw only when visible
		{
			glClearColor(app.pointers[0].x/app.width, 0, app.pointers[0].y/app.height, 1);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			app.flip();	// flips pages
			app.process(0);	// processes events, argument is timeout
		}
		else app.process();	// default value is -1, means to wait for the next event
	}
	return 0;
}
