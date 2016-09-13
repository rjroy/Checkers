GameBoardBasics - Two player game agnostic board types (EPlayer, SPosition, CMove, TScoredMove)

ComputerPlayer - Uses a generic board type to perform Alpha Beta Pruning to determine the best move with current information.
Requires that the board implement: IsValidMove, GetMoves, MakeMoveIfValid, CalculatePlayerScore, GetOpponent.

LearningCache - A hash map with a maximum cache size.  It pushes all recently visted nodes to the end of a doublely linked list. 
The cache will be cleared from the front of the cache as needed.

CheckersBoard - Checkers board implementation which can be used by a ComputerPlayer to find potential moves and score them.
Will also validate moves using American Checkers rules.

PerfTimer - Used to time various functions to find performance hot spots.
