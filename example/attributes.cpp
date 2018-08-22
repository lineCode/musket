//--------------------------------------------------------
// musket/example/attributes/attributes.cpp
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
			"attributes",
		};

		musket::text_format tf;
		tf.size = 18.0f;

		auto const rc = wnd.client_area_size();

		musket::widget< musket::auto_resizer< musket::label > > lbl = {
			spirea::rect_t{ spirea::point_t{ 20.0f, 30.0f }, { rc.width() - 50.0f, rc.height() - 60.0f } }, 
			"AutoResize", 
			musket::label_property{ 
				tf,
				musket::label_style{ 
					musket::rgba_color_t{ 0.2f, 0.2f, 0.2f, 1.0f }, 
					musket::edge_property{ { 0.95f, 0.95f, 0.0f, 1.0f } }, 
					{ 1.0f, 1.0f, 1.0f, 1.0f } 
				},
			}
		};

		musket::widget< musket::auto_scaling_scroll_bar< musket::axis_flag::vertical > > scroll_bar = {
			spirea::rect_t{ spirea::point_t{ rc.right - 20.0f, 0.0f }, { 20.0f, rc.bottom } },
			5u, 100u
		};

		wnd.attach_widget( lbl );
		wnd.attach_widget( scroll_bar );

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