#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <array>
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
void place()
{

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