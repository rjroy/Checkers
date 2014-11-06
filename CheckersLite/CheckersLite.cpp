// CheckersLite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Display.h"
#include "ComputerPlayer.h"

#include <time.h>
#include <iostream>
using namespace std;

bool UserMove( CCheckersBoard& board, const CDisplay& display, EPlayer userPlayer );

int _tmain(int argc, _TCHAR* argv[])
{
	srand( (unsigned int)time(NULL) );

	CDisplay display;
	CCheckersBoard board;

	CComputerPlayer p1( Player_Red );
	CComputerPlayer p2( Player_Black );
	//EPlayer p2 = CCheckersBoard::GetOpponent( p1.GetPlayer() );

	display.Show( cout, board );
	cout << endl;
	for( ;; )
	{
		if( !p1.Move( board, 0 ) )
		{
			display.Show( cout, board );
			break;
		}

		display.Show( cout, board );
		cout << "P1: " << board.CalculatePlayerScore( p1.GetPlayer() ) << endl << endl;

		// if( !UserMove( board, display, p2 ) ) break;

		if( !p2.Move( board, 4 ) )
		{
			display.Show( cout, board );
			break;
		}

		display.Show( cout, board );
		cout << "P2: " << board.CalculatePlayerScore( p2.GetPlayer() ) << endl << endl;
	}

	cout << "DONE" << endl;
	cin.ignore();
	cin.get();

	return 0;
}

bool UserMove( CCheckersBoard& board, const CDisplay& display, EPlayer userPlayer)
{
	std::vector<CMove> moves;
	if( !board.GetMoves( userPlayer, moves ) )
		return false;
	if( moves.empty() )
		return false;

	unsigned int selection;
	do {
		display.ShowMoves( cout, moves );
		cout << "Please select a move: ";
		cin >> selection;
	} while( selection >= moves.size() );
	if( !board.MakeMoveIfValid( userPlayer, moves[selection] ) )
	{
		cout << "failed to make the valid move: " << selection << endl;
		return false;
	}

	return true;
}