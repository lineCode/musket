#include <iostream>
#include <sstream>
#include <musket.hpp>

int main()
{
	try {
		musket::window wnd = {
			"scroll bar",
			spirea::rect_t{ spirea::point_t{ 0, 0 }, { 320, 240 } },
			musket::rgba_color_t{ 0.3f, 0.3f, 0.3f, 0.0f }
		};

		musket::font_format const font = {
			"Yu Gothic",
			spirea::dwrite::font_weight::normal,
			spirea::dwrite::font_style::normal,
			spirea::dwrite::font_stretch::normal,
			spirea::dwrite::text_alignment::center,
		};

		auto const client_rc = wnd.client_area_size();

		musket::widget< musket::label > lbl = {
			spirea::rect_t{ spirea::point_t{ 40.0f, 80.f }, { 200.0f, 30.0f } },
			"0, 5", font,
			musket::label_style{ {}, {}, { 1.0f, 1.0f, 1.0f, 1.0f } }
		};

		musket::widget< musket::scroll_bar<> > scroll = {
			spirea::rect_t{ spirea::point_t{ client_rc.right - 20.0f, 0.0f }, { 20.0f, client_rc.bottom } },
			musket::scroll_bar_style{ { 0.4f, 0.4f, 0.4f, 1.0f }, {}, 5u, 100u, 10.0f },
			musket::scroll_bar_thumb_style{ { 0.65f, 0.65f, 0.65f, 1.0f }, {} },
			musket::scroll_bar_thumb_style{ { 0.75f, 0.75f, 0.75f, 1.0f }, {} },
			musket::scroll_bar_thumb_style{ { 0.9f, 0.9f, 0.9f, 1.0f }, {} },
		};

		scroll->connect( musket::scroll_bar_event::scroll{}, [lbl](std::uint32_t lower, std::uint32_t upper) mutable {
			std::ostringstream oss;
			oss << lower << ", " << upper << std::ends;
			lbl->set_text( oss.str() );
		} );

		wnd.attach_widget( lbl );
		wnd.attach_widget( scroll );

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