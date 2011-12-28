#include <iostream>
#include <string>
#include <fstream>
#include <map>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/progress.hpp>

#include <boost/asio/ssl.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "stx.h"

//icuuc
#include <Windows.h>

#pragma comment(lib, "libeay32MDd.lib")
#pragma comment(lib, "ssleay32MDd.lib")

namespace search
{
	bool judge_is_json_format( const std::string & str )
	{


		return true;
	}


	std::map< string_map::stx::string_symbol, std::string > mp;
	std::string key;
	boost::optional< std::string > search_ssl( const std::string & url )
	{
		try
		{
			const auto index = url.find( "/" );

			if( index == std::string::npos )
				return boost::optional< std::string >();

			const std::string uri = url.substr( 0, index );
			const std::string host = uri;
			const std::string site = url.substr( index );

			boost::asio::io_service io_service;

			boost::asio::ssl::context context( io_service, boost::asio::ssl::context::sslv3_client );
			boost::asio::ssl::stream< boost::asio::ip::tcp::socket > ssl_stream( io_service, context );

			//resolve���g���ăT�C�g�����݂���̂��m�F�A�����Đڑ�
			ssl_stream.lowest_layer().connect( * boost::asio::ip::tcp::resolver( io_service ).resolve\
				( boost::asio::ip::tcp::resolver::query( uri, "https" ) ) );

			ssl_stream.handshake( boost::asio::ssl::stream_base::client );

			//request���쐬����-------------------------------
			boost::asio::streambuf request;
			std::ostream request_stream( & request );

			request_stream << "GET /" << site << " HTTP/1.0\r\n";
			request_stream << "Host: " << host << "\r\n";
			request_stream << "\r\n";
			//------------------------------------------------

			//���N�G�X�g�̑��M
			boost::asio::write( ssl_stream, request );

			//���N�G�X�g�̎�M�ƕ\��
			boost::asio::streambuf res;
			boost::system::error_code error;
			std::string log_str;
			while( boost::asio::read( ssl_stream, res, boost::asio::transfer_at_least(1), error ) )
			{
				std::ostringstream ss;
				//std::string str = boost::asio::buffer_cast< const char * >( res.data() ); 
				//std::cout << str;
				ss << & res;
				log_str += ss.str();

				if( log_str.find( "options['checkinId']" ) != std::string::npos )
					break;
			}

			const auto debug_e_mes = error.message();
			const std::string venuestr = "options['venue']";
			const auto venue = log_str.find( venuestr );
			const auto checkin = log_str.find( "options['checkinId']" );

			//std::cout << log_str;
			if(  venue != std::string::npos && checkin != std::string::npos )
			{
				std::string tmp_str = log_str.substr( venue + venuestr.size() + 2, checkin - ( venue + venuestr.size() + 2 ) - 2 );

				//�����ʂ̐��̊m�F - json�Ƃ��Ă͊��S�Ȃ͂�
				if( std::count( tmp_str.begin(), tmp_str.end(), '{' ) != \
					std::count( tmp_str.begin(), tmp_str.end(), '}' ) )
				{
					return boost::optional< std::string >();//json�Ƃ��Ă͕s���S
				}

				boost::algorithm::replace_all( tmp_str, "\n", "" );
				boost::algorithm::replace_all( tmp_str, "\t", "" );
				boost::algorithm::replace_all( tmp_str, " ", "" );
				boost::algorithm::replace_all( tmp_str, "\\", "" );//�G�X�P�[�v�����폜
				
				//name�̒��� " " ����������A����͒u��
				const auto namefind = tmp_str.find( "\"name\":" );

				//name�̒��� "" ������
				if( namefind != std::string::npos )
				{
					const auto contactfind = tmp_str.find( ",\"contact\":" );

					if( contactfind != std::string::npos )
					{
						auto namestr = tmp_str.substr( namefind + 9, contactfind - namefind - 12 ); 

						boost::algorithm::replace_all( namestr, "\"", "" );

						tmp_str = tmp_str.substr( 0, namefind + 9 ) + namestr + \
							tmp_str.substr( contactfind - 3 ); 
					}
				}
				tmp_str.pop_back();
				//tmp_str = tmp_str.substr( 1 );
				mp.insert( std::map< std::string, std::string >::value_type( key, tmp_str ) );

				return boost::optional< std::string >( tmp_str );
			}
			return boost::optional< std::string >();
		}
		catch( std::exception & e )
		{
			std::cerr << e.what() << std::endl;
			return boost::optional< std::string >();
		}
	}



	boost::optional< std::string > search_http( const std::string & httpurl )
	{
		try
		{
			using namespace boost::asio;
			// www.google.co.jp �� http �T�[�r�X�ɐڑ�

			const std::string url = httpurl.substr( httpurl.find( "//" ) + 2 ); 

			const auto index = url.find( "/" );

			if( index == std::string::npos )
				return boost::optional< std::string >();

			const std::string uri = url.substr( 0, index );
			const std::string host = uri;
			const std::string site = url.substr( index );

			ip::tcp::iostream s( uri, "http" );

			// ���M
			s << "GET /" << site <<" HTTP/1.0\r\n";
			s << "Host: " << host << "\r\n";
			s << "\r\n";
			s << std::flush;  // �o�b�t�@�ɗ��߂��Ɋm���Ƀl�b�g���[�N���M

			// ��M
			std::string line;
			std::string result;
			while( getline(s, line) )
			{
				result += line+ "\n";
				//std::cout << line << std::endl;
			}

			return boost::optional< std::string >( result );
		}
		catch( std::exception & e )
		{
			std::cerr << e.what() << std::endl;
			return boost::optional< std::string >();
		}
	}
}

void search_main()
{
	using namespace search;
	using namespace boost::property_tree;

	boost::io::ios_rdbuf_saver saver( std::cout );
	std::string output;
	std::ifstream ifs( "input.txt" );
	int counter = 0;

	enum STATE
	{
		E_NULL,
		E_SEARCH_START,
		E_SEARCH_USER_NAME,
		E_SEARCH_FIRST_DATE,
		E_SEARCH_DATE,
		E_SEARCH_DATA_OR_DATE_OR_END,
		E_END
	};

	int state = E_SEARCH_START;

	output += "{\n\"data\": [\n";

	while( ! ifs.eof() )
	{
		std::cout << ++counter << std::endl;

		std::string str;

		std::getline( ifs, str );

		boost::algorithm::replace_all( str, "\t", "" );

		if( state == E_END || state == E_SEARCH_START )
		{
			if( str.find( "start" ) == 0 )
			{
				state = E_SEARCH_USER_NAME;
				output += "{\n";
				continue;
			}
		}

		if( state == E_SEARCH_USER_NAME )
		{
			//���[�U�����L�^
			output += "\"user\":\"" + str + "\",\n";
			state = E_SEARCH_FIRST_DATE;
			continue;
		}

		if( state == E_SEARCH_FIRST_DATE )
		{
			if( str.find("d") == 0 )
			{
				//���߂Č��������t������
				output += "\"days\" : \n\t\t[\n";
				output += ( std::string )"\t\t\t{\n" + "\t\t\t\t\"day\":\"" + str.substr( 1 ) + "\",";
				output += "\n\t\t\t\t\"log\":[\n\t\t\t\t\t\n";
				state = E_SEARCH_DATA_OR_DATE_OR_END;
				continue;
			}
		}

		if( state == E_SEARCH_DATA_OR_DATE_OR_END || state == E_SEARCH_DATA_OR_DATE_OR_END )
		{
			if( str.find("d") == 0 )
			{
				output.pop_back();
				output.pop_back();
				//���t������
				output += "\n\t\n]\n},\n";//���O�̏I���
				output += ( std::string )"\t\t\t{\n" + "\"day\":\"" + str.substr( 1 ) + "\",";
				output += "\n\t\t\t\t\"log\":[\n\t\t\t\t\t\n";
				
				continue;
			}

			if( str.find( "http://" ) != std::string::npos )
			{
				//key����
				const auto imatf = str.find("http://" );

				const auto imat = str.substr( 0, imatf );
				std::cout << "searching : " << imat << std::endl;

				if( imatf != std::string::npos )
				{
					const auto itr = mp.find(  imat );

					if( itr != mp.end() )
					{
						const auto str_for_time = str.substr( str.find( "http://" ) );
						const auto time = str_for_time.substr( str_for_time.find( "{" ) + 1, 8 );
						std::cout << time << std::endl;
						output += ( std::string )"{ \"time\":" + "\"" + time + "\"" + ",\n \"info\":[";
						output += itr->second + "]},\n";
						//��������
						std::cout << "hit" << std::endl;		
						continue;
					}
				}
				//�V�K�ꏊ�Ȃ̂ŃL�[�ɋL��
				key = imat;
				//�T�C�g������
				if( str.find( "http://t.co" ) != std::string::npos )
				{
					const auto tco_url = str.substr( str.find( "http" ), str.find( "{" ) - str.find( "http" ) - 1 );
					const auto tco = search_http( tco_url );

					if( tco )
					{
						const auto http_index = tco->find( "http" );
						if( http_index == std::string::npos )
							continue;
						const auto url_4sq = tco->substr( http_index, tco->find( "Cache" ) - http_index - 2 );
						//4sq���o -2��\n�؂�

						const auto fsq = search_http( url_4sq );

						if( fsq )
						{
							const auto s = * fsq;
							//���_�C���N�g����
							if( fsq->find( "href=\"https://foursquare" ) != std::string::npos )
							{
								const auto hrefsub = fsq->substr( fsq->find( "href=\"https://foursquare" ) );
								const auto href_index = fsq->find( "\">" );
								const auto len = hrefsub.find( "\">" );
								const auto url_fsq = hrefsub.substr( 14, len - 14 );
								search_ssl( url_fsq );
								if( const auto s = search_ssl( url_fsq ) )
								{
									const auto str_for_time = str.substr( str.find( "http://" ) );
									const auto time = str_for_time.substr( str_for_time.find( "{" ) + 1, 8 ); 
									std::cout << time << std::endl;
									output += ( std::string )"{ \"time\":" + "\"" + time + "\"" + ",\n \"info\":[";
									output += s.get() + "]},\n";
								}
							}
						}
					}
				}
				if( str.find( "http://4sq" ) != std::string::npos )
				{
					const auto fsu = str.substr( str.find( "http" ) );
					const auto fsq_url = fsu.substr( 0, fsu.find( " " ) );
					const auto fsq = search_http( fsq_url );

					if( fsq )
					{
						const auto fsq_href_index = fsq->find( "href=" );
						//���_�C���N�g����
						if( fsq->find( "href=" ) != std::string::npos )
						{
							ptree log;
							const auto hrefsub = fsq->substr( fsq->find( "href=" ) );
							const auto href_index = fsq->find( "\">" );
							const auto len = hrefsub.find( "\">" );
							const auto url_fsq = hrefsub.substr( 14, len - 14 );
							if( const auto s = search_ssl( url_fsq ) )
							{
								const auto str_for_time = str.substr( str.find( "http://" ) );
								const auto time = str_for_time.substr( str_for_time.find( "{" ) + 1, 8 );
								std::cout << time << std::endl;

								output += ( std::string )"{ \"time\":" + "\"" + time + "\"" + ",\n \"info\":[";
								output += s.get() + "]},\n";
							}
						}
					}
					continue;
				}
			}
		}
		if( str.find( "end" ) == 0 )
		{
			//�O�̓��O
			output.pop_back();
			output.pop_back();
			output += "\n]\n} \n]\n},\n";

			state = E_END;
			continue;
		}
	}

	output.pop_back();
	output.pop_back();

	output += "\n]\n}\n";
	std::ofstream ofs( "output.txt" );
	ofs << output;

}



int main()
{
	boost::progress_timer t;
	search_main();
	return 0;
}