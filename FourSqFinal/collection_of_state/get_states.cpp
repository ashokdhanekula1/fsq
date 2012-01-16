#include <iostream>
#include <string>
#include <fstream>
#include <vector>

void write( std::ofstream & outfile, const std::vector< std::string > & strs )
{
	for( unsigned int i = 0; i < strs.size(); ++i )
	{
		outfile << strs[ i ] << std::endl;
	}
}

void read_and_write_files( std::ifstream & infile, const std::string & word )
{
	using namespace std;
	const string filename = "seached.txt";
	ofstream outfile( filename );

	if( ! outfile.fail() )
	{
		vector< string > data;
		bool find_flag = false;

		while( ! infile.eof() )
		{
			std::string input;
			getline( infile, input );

			//�ꍇ����
			if( input.size() > 0 )
			{
				if( input[ 0 ] == '{' && input.size() == 1 )
				{
					data.clear();
					find_flag = false;
				}

				data.push_back( input );

				if( input.find( word ) != string::npos )
				{
					find_flag = true;
				}

				if( input[ 0 ] == '}' && find_flag )
				{
					write( outfile, data );
				}
			}
		}
		write( outfile, data );
	}

}



int main()
{
	using namespace std;

	const string filename = "input.txt";

	ifstream infile( filename );

	if( ! infile.fail() )
	{
		read_and_write_files( infile, "\"���s" );
	}
	else
	{
		cerr << "bad input file" << endl;
	}

	return 0;
}