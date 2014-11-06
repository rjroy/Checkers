// CheckersLite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Display.h"
#include "ComputerPlayer.h"

#include <time.h>
#include <iostream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	srand( (unsigned int)time(NULL) );

	CDisplay display;
	CCheckersBoard board;

	CComputerPlayer p1( Player_Red );
	EPlayer p2 = CCheckersBoard::GetOpponent( p1.GetPlayer() );

	for( ;; )
	{
		display.ClearScreen( cout );

		if( !p1.Move( board ) )
		{
			display.Show( cout, board );
			break;
		}

		display.Show( cout, board );

		std::vector<CMove> moves;
		if( !board.GetMoves( p2, moves ) )
			break;
		if( moves.empty() )
			break;

		unsigned int selection;
		do {
			display.ShowMoves( cout, moves );
			cout << "Please select a move: ";
			cin >> selection;
		} while( selection >= moves.size() );
		if( !board.MakeMoveIfValid( p2, moves[selection] ) )
		{
			cout << "failed to make the valid move: " << selection << endl;
			break;
		}

	}

	cout << "DONE" << endl;
	cin.ignore();
	cin.get();

	return 0;
}

