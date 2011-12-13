#include <iostream>
#include <math.h>
#include <fstream>
#include <numeric>
#include <vector>
#include <array>
#include <map>
#include <time.h>

#include <boost/format.hpp>
#include <boost/optional.hpp>

#define DEBUGMODE_FLAG 0

enum enum_elements
{
	E_ELEM_SHOP,
	E_ELEM_ENTERTAINMENT,
	E_ELEM_FOOD,
	E_MAX,
};

class database
{
public:
	typedef unsigned int day;
	typedef unsigned int time;

	unsigned int value( const std::string & category ) const
	{
		const auto ptr = category_.find( category );

		if( ptr != category_.end() )
			return ptr->second;
		else
			return 0;
	}

	unsigned int sum() const
	{
		return static_cast< unsigned >\
			( std::accumulate( category_.begin(), category_.end(), 0, \
			[]( const unsigned int value, const std::pair< const std::string, time > & v )
			{
				return  v.second + value;
			}
		) );
	}


	void add( const std::string & category, const time time_value, const day day_value )
	{
		{
			const auto it = std::find( black_list_.begin(), black_list_.end(), category );

			if( it != black_list_.end() )
				return;
		}


		const auto it = history_.find( category );

		const unsigned int border = 3;

		if( it == history_.end() || \
			abs( static_cast< long >( it->second - day_value ) ) < border )
			category_[ category ] += time_value;
		else
		{
			black_list_.push_back( category );
		}
	}

	void sub( const std::string & category, const unsigned int value )
	{
		category_[ category ] -= value;
	}

	struct spot
	{
		std::string name_;
		std::string category_;
		std::string city_;
		double latitude_;
		double longitude_;

		spot( const std::string & name, const std::string & category, const std::string & city, \
			const double latitude, const double longitude ) : \
			name_( name ), category_( category ), latitude_( latitude ), longitude_( longitude )
		{}

		spot(){}

	};

private:
	std::map< std::string, time > category_;
	std::map< std::string, day > history_;
	std::vector< std::string > black_list_;
	//�ǉ�����Ė���������Avalue = 0�Ƃ��ĕԂ�
};

struct profile
{
	std::map< std::string, database > city_history_;
	unsigned int id_;
};



//< shpp, station, entertainment, .... >
//�������Ƃ̖K���(�������S�̂̌��������Ŋ������l = ����
//�S�̂̕��ꂩ�����I�ɍs���Ă���ꏊ�̐��͌��炷
//< 

//R k,l (k,l���[�U) = K * L / ( | K | * | L | )
// = ��i ( K i * L i ) ^ 2 / ( ( sqrt( ��i( K i ) ^ 2 ) * ( sqrt( ��i( L i ) ^ 3 ) ) 
//R = ���֌W��
double calc( const std::vector< double > & k, const std::vector< double > & l )
{
	assert( k.size() == l.size() );

	double x = 0;
	double ky = 0;
	double ly = 0;

	for( std::size_t i = 0, size = k.size(); i < size; ++i )
	{
		x += k[ i ] * l[ i ];//pow( k[ i ] * l[ i ], 2 );
		ky += pow( k[ i ], 2 );
		ly += pow( l[ i ], 2 );
	}
	
	const double relation = x / ( sqrt( ky ) * sqrt( ly ) );

#if DEBUGMODE_FLAG
	std::cout << boost::format( "x : %0.3lf, a : %0.3lf, b : %0.3lf, R : %0.3lf\n" ) % x % ky % ly % relation;
#endif

	return relation;
}

//city�̋L��
//city���������������ł܂����ŋL�^���Ă����ꍇ�͒n��
//�s���͈͂Ɋ�Â��ގ��x�v�Z
boost::optional< std::vector< int > > get_time( const std::vector< int > & times )
{
	std::vector< int > result;

	if( times.size() <= 1 )
		return boost::optional< std::vector< int > >();

	//�e�ό��n�́A�e����(�����ّ�)�̍s�����Ԃ̕��ς��Ƃ��Ă����̂����肩��
	//�s���͈͂��L�� == ���鎞�Ԃ��Z���āA�����̑����𓾂Ă���?

	//�}���R�t�A���I�Ȃ��̂����߂Ă��Ȃ��̂ŏ��Ԃ͌��Ȃ�

	for( std::size_t i = 0, size = times.size(); i < size - 1; ++i )
	{
		result.push_back( times[ i ] - times[ i + 1 ] );
	}

	return boost::optional< std::vector< int > >( result );
}


int main()
{
	std::vector< double > user_a( 5 );
	std::vector< double > user_b( 5 );

	for( int i = 0; i < 5; ++i )
	{
		user_a[ i ] = rand() % 10;
		user_b[ i ] = rand() % 10;
	}

	std::cout << calc( user_a, user_b ) << std::endl;


	return 0;
}