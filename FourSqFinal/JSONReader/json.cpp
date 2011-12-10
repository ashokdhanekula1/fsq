#include <iostream>
#include <string>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>

void reader()
{
	using namespace boost::property_tree;

	ptree pt;

	try
	{
		json_parser::read_json( "output.txt", pt );
	}
	catch( std::exception & e )
	{
		std::cerr << e.what() << std::endl;
	}

	if( const auto result = pt.get_optional< std::string >( "���{��" ) )
	{
		std::cout << result.get() << std::endl;
	}
}

void writer()
{
	using namespace boost::property_tree;

	wptree pt;
	wptree child;

	pt.put( L"�������������{��\"", L"�k�C���J�T" );

	//std::cout << pt.get< std::string >( "\"�������������{��" ) << std::endl;

	try
	{
		write_json( "ts.json", pt );
	}
	catch( std::exception & e )
	{
		std::cout << e.what() << std::endl;
	}


}


int main()
{
	
	writer();
	reader();
	return 0;
}