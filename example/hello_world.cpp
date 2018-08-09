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
			spirea::rect_t{ spirea::point_t{ 0, 0 }, { 320, 240 } },
			musket::rgba_color_t{ 0.2f, 0.2f, 0.2f, 0.0f },
		};

		musket::font_format const font = {
			"Yu Gothic", 17.0f,
			spirea::dwrite::font_weight::normal,
			spirea::dwrite::font_style::normal,
			spirea::dwrite::font_stretch::normal,
			spirea::dwrite::text_alignment::center,
			spirea::dwrite::paragraph_alignment::center,
		};

		musket::widget< musket::button > btn = {
			spirea::rect_t{ spirea::point_t{ 110.0f, 150.0f }, { 100.0f, 30.0f } }, 
			"Push", font,
			musket::button_style{ 
				{ 0.4f, 0.4f, 0.4f, 1.0f }, 
				musket::edge_property{ { 0.1f, 0.1f, 0.1f, 1.0f } }, 
				{ 1.0f, 1.0f, 1.0f, 1.0f } 
			},
			musket::button_style{ 
				{ 0.5f, 0.5f, 0.5f, 1.0f }, 
				musket::edge_property{ { 0.1f, 0.1f, 0.1f, 1.0f } }, 
				{ 1.0f, 1.0f, 1.0f, 1.0f } 
			},
			musket::button_style{ 
				{ 0.7f, 0.7f, 0.7f, 1.0f }, 
				musket::edge_property{ { 1.0f, 1.0f, 0.0f, 1.0f } }, 
				{ 1.0f, 1.0f, 1.0f, 1.0f } 
			},
		};

		musket::widget< musket::label > lbl = {
			spirea::rect_t{ spirea::point_t{ 60.0f, 60.0f }, { 200.0f, 30.0f } }, 
			"hello, world!", font,
			musket::label_style{
				{},
				{},
				{ 1.0f, 1.0f, 1.0f, 1.0f },
			}
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