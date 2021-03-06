//--------------------------------------------------------
// musket/example/scroll_bar/scroll_bar.cpp
//
// Copyright (C) 2018 LNSEAB
//
// released under the MIT License.
// https://opensource.org/licenses/MIT
//--------------------------------------------------------

#include <iostream>
#include <sstream>
#include <musket.hpp>

int main()
{
	try {
		musket::window wnd = {
			spirea::rect_t< float >{ { 0, 0 }, { 320, 240 } },
			"scroll bar",
		};

		musket::text_format tf;
		tf.size = 24.0f;

		auto const client_rc = wnd.client_area_size();

		musket::widget< musket::label > lbl = {
			spirea::rect_t< float >{ { 40.0f, 50.f }, { 200.0f, 30.0f } },
			"0, 3", musket::label_property{ tf }
		};
		musket::widget< musket::label > hlbl = {
			spirea::rect_t< float >{ { 40.0f, 100.f }, { 200.0f, 30.0f } },
			"0, 3", musket::label_property{ tf }
		};

		musket::widget< musket::scroll_bar< musket::axis_flag::vertical > > scroll = {
			spirea::rect_t< float >{ { client_rc.right - 20.0f, 0.0f }, { 20.0f, client_rc.bottom } },
			3u, 100u
		};
		musket::widget< musket::scroll_bar< musket::axis_flag::horizontal > > hscroll = {
			spirea::rect_t< float >{ { 0.0f, client_rc.bottom - 20.0f }, { client_rc.right - 22.0f, 20.0f } },
			3u, 100u
		};

		scroll->connect( musket::scroll_bar_event::scroll{}, [lbl](std::uint32_t lower, std::uint32_t upper) mutable {
			std::ostringstream oss;
			oss << lower << ", " << upper << std::ends;
			lbl->set_text( oss.str() );
		} );

		hscroll->connect( musket::scroll_bar_event::scroll{}, [hlbl](std::uint32_t lower, std::uint32_t upper) mutable {
			std::ostringstream oss;
			oss << lower << ", " << upper << std::ends;
			hlbl->set_text( oss.str() );
		} );

		wnd.attach_widget( lbl );
		wnd.attach_widget( hlbl );
		wnd.attach_widget( scroll );
		wnd.attach_widget( hscroll );

		wnd.show();

		return musket::loop();
	}
	catch( std::exception const& e ) {
		std::cerr << e.what() << std::endl;
	}
	catch( ... ) {
		std::cerr << "unknown exception" << std::endl;
	}
}