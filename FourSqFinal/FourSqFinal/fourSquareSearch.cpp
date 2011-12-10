#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <codecvt>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/progress.hpp>

#include <boost/asio/ssl.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

//icuuc
#include <Windows.h>

#pragma comment(lib, "libeay32MDd.lib")
#pragma comment(lib, "ssleay32MDd.lib")

namespace search
{
	std::map< std::string, std::string > mp;
	std::string key;
	boost::optional< std::string > search_ssl( const std::string & url, boost::property_tree::ptree & log )
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

			//resolveを使ってサイトが存在するのか確認、そして接続
			ssl_stream.lowest_layer().connect( * boost::asio::ip::tcp::resolver( io_service ).resolve\
				( boost::asio::ip::tcp::resolver::query( uri, "https" ) ) );

			ssl_stream.handshake( boost::asio::ssl::stream_base::client );

			//requestを作成する-------------------------------
			boost::asio::streambuf request;
			std::ostream request_stream( & request );

			request_stream << "GET /" << site << " HTTP/1.0\r\n";
			request_stream << "Host: " << host << "\r\n";
			request_stream << "\r\n";
			//------------------------------------------------

			//リクエストの送信
			boost::asio::write( ssl_stream, request );

			//リクエストの受信と表示
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

			const auto e_mes = error.message();
			const std::string venuestr = "options['venue']";
			const auto venue = log_str.find( venuestr );
			const auto checkin = log_str.find( "options['checkinId']" );
			//std::cout << log_str;
			if(  venue != std::string::npos && checkin != std::string::npos )
			{
				std::string tmp_str = log_str.substr( venue + venuestr.size() + 2, checkin - ( venue + venuestr.size() + 2 ) - 2 );
				boost::algorithm::replace_all( tmp_str, "\n", "" );
				boost::algorithm::replace_all( tmp_str, "\t", "" );
				boost::algorithm::replace_all( tmp_str, " ", "" );
				boost::algorithm::replace_all( tmp_str, "\\", "" );//エスケープ文字削除

				tmp_str.pop_back();
				//tmp_str.pop_back();

				//tmp_str = tmp_str.substr( 1 );

				boost::property_tree::ptree p;

				std::stringstream s;
				s << tmp_str;

				std::cout << s.str();

				std::cout << s << std::endl;

				boost::property_tree::json_parser::read_json( s, p );

				log.push_back( std::make_pair( "", p ) );

				mp.insert( std::map< std::string, std::string >::value_type( key, tmp_str ) );
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
			// www.google.co.jp の http サービスに接続

			const std::string url = httpurl.substr( httpurl.find( "//" ) + 2 ); 

			const auto index = url.find( "/" );

			if( index == std::string::npos )
				return boost::optional< std::string >();

			const std::string uri = url.substr( 0, index );
			const std::string host = uri;
			const std::string site = url.substr( index );

			ip::tcp::iostream s( uri, "http" );

			// 送信
			s << "GET /" << site <<" HTTP/1.0\r\n";
			s << "Host: " << host << "\r\n";
			s << "\r\n";
			s << std::flush;  // バッファに溜めずに確実にネットワーク送信

			// 受信
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

	std::ifstream ifs( "input.txt" );

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

	wptree json;
	ptree users;
	ptree days;
	ptree logs;
	ptree user;
	ptree day;



	while( ! ifs.eof() )
	{
		std::string str;

		std::getline( ifs, str );

		boost::algorithm::replace_all( str, "\t", "" );

		if( state == E_END || state == E_SEARCH_START )
		{
			if( str.find( "start" ) == 0 )
			{
				state = E_SEARCH_USER_NAME;
				user = ptree();
				days = ptree();
				day = ptree();
				logs = ptree();

				continue;
			}
		}

		if( state == E_SEARCH_USER_NAME )
		{
			//ユーザ名を記録
			user.put( "user", str );
			state = E_SEARCH_FIRST_DATE;
			continue;
		}

		if( state == E_SEARCH_FIRST_DATE )
		{
			if( str.find("d") == 0 )
			{
				//日付だった
				day.put( "day", str.substr( 1 ) );
				state = E_SEARCH_DATA_OR_DATE_OR_END;
				continue;
			}
		}

		if( state == E_SEARCH_DATA_OR_DATE_OR_END || state == E_SEARCH_DATA_OR_DATE_OR_END )
		{
			if( str.find("d") == 0 )
			{
				//日付だった
				day.add_child( "logs", logs );
				days.push_back( std::make_pair( "", day ) );
				day = ptree();
				day.put( "day", str.substr( 1 ) );
				continue;
			}

			if( str.find( "http://" ) != std::string::npos )
			{
				//key検索
				const auto imatf = str.find("http://" );

				const auto imat = str.substr( imatf );

				if( imatf != std::string::npos )
				{
					const auto itr = mp.find( imat );

					if( itr != mp.end() )
					{
						//見つかった
						ptree log;
						log.put( "info", itr->second );
						logs.push_back( std::make_pair( "", log ) );
						continue;
					}
				}
				//新規場所なのでキーに記憶
				key = imat;
				//サイトだった
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
						//4sq抽出 -2は\n切り

						const auto fsq = search_http( url_4sq );

						if( fsq )
						{
							const auto s = * fsq;
							//リダイレクト処理
							if( fsq->find( "href=\"https://foursquare" ) != std::string::npos )
							{
								ptree log;
								const auto hrefsub = fsq->substr( fsq->find( "href=\"https://foursquare" ) );
								const auto href_index = fsq->find( "\">" );
								const auto len = hrefsub.find( "\">" );
								const auto url_fsq = hrefsub.substr( 14, len - 14 );
								search_ssl( url_fsq, log );

								logs.push_back( std::make_pair( "", log ) );	
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
						//リダイレクト処理
						if( fsq->find( "href=" ) != std::string::npos )
						{
							ptree log;
							const auto hrefsub = fsq->substr( fsq->find( "href=" ) );
							const auto href_index = fsq->find( "\">" );
							const auto len = hrefsub.find( "\">" );
							const auto url_fsq = hrefsub.substr( 14, len - 14 );
							search_ssl( url_fsq, log );
							logs.push_back( std::make_pair( "", log ) );					

						}
					}

					continue;
				}
			}

		}
		if( str.find( "end" ) == 0 )
		{
			user.add_child( "day", days );
			users.push_back( std::make_pair( "", user ) );

			state = E_END;
			continue;
		}
	}
	json.add_child( L"users", users );
	write_json( "output.json", json );
}



int main()
{
	boost::progress_timer t;
	search_main();
	return 0;
}