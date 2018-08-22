//--------------------------------------------------------
// musket/include/musket/example/list_box.cpp
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
			spirea::rect_t{ spirea::point_t{ 0, 0 }, { 320, 240 } },
			"list box",
		};

		auto const rc = wnd.client_area_size();

		musket::widget< musket::auto_resizer< musket::list_box< char > > > list = {
			spirea::rect_t{ spirea::point_t{ 20.0f, 15.0f }, { rc.width() - 40.0f, rc.height() - 30.0f } }, 
			20.0f
		};
		for( char c = 'a'; c <= 'z'; ++c ) {
			list->push_back( std::string( ( ( c - 'a' ) % 10 ) + 1, c ), c );
		}

		wnd.attach_widget( list );

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