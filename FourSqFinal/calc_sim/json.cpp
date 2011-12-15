#include <iostream>
#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace json
{
	using namespace boost::property_tree;
	class json_reader
	{
	public:
		void read_file( const std::string & filename );
		wptree pt() const{ return pt_; }

	private:
		wptree pt_;
	};

	struct data
	{
		std::string city_name_;
		std::string state_name_;
		int day_;
		int time_;
	};

	void show_all( const json_reader & jr )
	{
		auto pt = jr.pt();

		if( const auto data = pt.get_child_optional( "data" ) )
		{
			if( auto x = data->begin()->second.get_optional< std::wstring >( "user" ) )
				std::cout << x.get() << std::endl;
			if( const auto days = pt.get_child_optional( "days" ) )
			{/*
				std::cout << "day : " << days->get< std::string >( "day" ) << std::endl;

				if( const auto log = days->get_child_optional( "log" ) )
				{
					std::cout << "time : " << log->get< std::string >( "time" ) << std::endl;
				}
			*/}
		}
	}

	void json_reader::read_file( const std::string & filename )
	{
		wptree pt;
		try
		{
			json_parser::read_json( filename, pt );
		}
		catch( std::exception & e )
		{
			std::cerr << e.what() << std::endl;
			return;
		}
		pt_ = std::move( pt );
	}
}

/*

data.child

	{
		-user
		days.child
			{
				-day
				log.child
				{
					-time
					info.child
					{
						-venue.location.lat
						-venue.location.lng
						-venue.city
						-venue.state
						-venue.categories.name ( shortName, puralName )
					}
				}
			}
		}
	}
}

*/

int main()
{
	json::json_reader js;
	js.read_file( "outputs.txt" );
	show_all( js );

	return 0;
}

