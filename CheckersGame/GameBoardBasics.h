#pragma once

#include "stdafx.h"

#include <vector>

static const int kBoardSize = 8;

//--------------------------------------------------------------------------------------
// Used to identify the two players.
enum EPlayer
{
	Player_Black,
	Player_Red,
	Player_None,
	
	PlayerCount
};

//--------------------------------------------------------------------------------------
// Represents a position on the board.
struct SPosition
{
	unsigned char m_x : 4;
	unsigned char m_y : 4;

	SPosition() : m_x(0), m_y(0) {}
	SPosition( unsigned int x, unsigned int y ) : m_x(x), m_y(y) { }

	int ToIndex() const { return m_x * kBoardSize + m_y; }

	int Compare( const SPosition& rhs ) const;
	bool IsValid() const { return ( m_x < kBoardSize && m_y < kBoardSize ); }

	bool operator == ( const SPosition& rhs ) const { return Compare( rhs ) == 0; }
	bool operator != ( const SPosition& rhs ) const { return Compare( rhs ) != 0; }
	bool operator <  ( const SPosition& rhs ) const { return Compare( rhs ) <  0; }
	bool operator <= ( const SPosition& rhs ) const { return Compare( rhs ) <= 0; }
	bool operator >  ( const SPosition& rhs ) const { return Compare( rhs ) >  0; }
	bool operator >= ( const SPosition& rhs ) const { return Compare( rhs ) >= 0; }
};

//--------------------------------------------------------------------------------------
// Represents a potential move.
class CMove
{
public:
	SPosition m_start;
	std::vector<SPosition> m_sequence;

	int Compare( const CMove& rhs ) const;

	bool operator == ( const CMove& rhs ) const { return Compare( rhs ) == 0; }
	bool operator != ( const CMove& rhs ) const { return Compare( rhs ) != 0; }
	bool operator <  ( const CMove& rhs ) const { return Compare( rhs ) <  0; }
	bool operator <= ( const CMove& rhs ) const { return Compare( rhs ) <= 0; }
	bool operator >  ( const CMove& rhs ) const { return Compare( rhs ) >  0; }
	bool operator >= ( const CMove& rhs ) const { return Compare( rhs ) >= 0; }
};


//--------------------------------------------------------------------------------------
inline int SPosition::Compare( const SPosition& rhs ) const
{
	int result = m_x - rhs.m_x;
	if( result )
		return result;
	return m_y - rhs.m_y;
}

//--------------------------------------------------------------------------------------
inline int CMove::Compare( const CMove& rhs ) const
{
	int result = m_start.Compare( rhs.m_start );
	if( result )
		return result;
	
	result = m_sequence.size() - rhs.m_sequence.size();
	if( result )
		return result;

	// NOTE: m_sequence.size() == rhs.m_sequence.size()
	return memcmp( &m_sequence[0], &rhs.m_sequence[0], sizeof( SPosition ) * m_sequence.size() );
}
