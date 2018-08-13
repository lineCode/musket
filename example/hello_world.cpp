//--------------------------------------------------------
// musket/example/hello_world/hello_world.cpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#include <iostream>
#include <musket.hpp>

int main()
{
	try {
		musket::window wnd = {
			"hello world",
			spirea::rect_t{ spirea::point_t{ 0, 0 }, { 320, 240 } }
		};

		musket::widget< musket::button > btn = {
			spirea::rect_t{ spirea::point_t{ 110.0f, 150.0f }, { 100.0f, 30.0f } }, 
			"Push" 
		};

		musket::widget< musket::label > lbl = {
			spirea::rect_t{ spirea::point_t{ 60.0f, 60.0f }, { 200.0f, 30.0f } }, 
			"hello, world!"
		};
		lbl->hide();

		btn->connect( musket::button_event::pressed{}, [lbl](spirea::point_t< std::int32_t > const&) mutable {
			musket::switch_visibility( lbl );
		} );

		wnd.attach_widget( btn );
		wnd.attach_widget( lbl );

		wnd.show();

		return musket::loop();
	}
	catch( std::exception const& e ) {
		std::cerr << e.what() << std::endl;
	}
	catch( ... ) {
		std::cerr << "unknown error" << std::endl;
	}
}