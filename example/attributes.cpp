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
			"attributes",
			spirea::rect_t{ spirea::point_t{ 0, 0 }, { 320, 240 } },
			musket::rgba_color_t{ 0.2f, 0.2f, 0.2f, 0.0f },
		};

		musket::font_format const font = {
			"Yu Gothic", 18.0f,
			spirea::dwrite::font_weight::normal,
			spirea::dwrite::font_style::normal,
			spirea::dwrite::font_stretch::normal,
			spirea::dwrite::text_alignment::center,
			spirea::dwrite::paragraph_alignment::center,
		};

		auto const rc = wnd.client_area_size();

		musket::widget< musket::auto_resizer< musket::label > > lbl = {
			spirea::rect_t{ spirea::point_t{ 20.0f, 30.0f }, { rc.width() - 50.0f, rc.height() - 60.0f } }, 
			"AutoResize", font,
			musket::label_style{ 
				musket::rgba_color_t{ 0.2f, 0.2f, 0.2f, 1.0f }, 
				musket::edge_property{ { 0.95f, 0.95f, 0.0f, 1.0f } }, 
				{ 1.0f, 1.0f, 1.0f, 1.0f } 
			},
		};

		musket::widget< musket::auto_scaling_scroll_bar< musket::axis_flag::vertical > > scroll_bar = {
			spirea::rect_t{ spirea::point_t{ rc.right - 20.0f, 0.0f }, { 20.0f, rc.bottom } },
			musket::scroll_bar_style{ { 0.4f, 0.4f, 0.4f, 1.0f }, {}, 5u, 100u, 10.0f },
			musket::scroll_bar_thumb_style{ { 0.65f, 0.65f, 0.65f, 1.0f }, {} },
			musket::scroll_bar_thumb_style{ { 0.75f, 0.75f, 0.75f, 1.0f }, {} },
			musket::scroll_bar_thumb_style{ { 0.9f, 0.9f, 0.9f, 1.0f }, {} },
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