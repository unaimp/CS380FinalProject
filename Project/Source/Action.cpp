#include <Stdafx.h>

using namespace MonteCarlo;


State::State(TileQ tile, bool AI, bool wallplacement)
	: mAI(AI), mRow(tile.row), mColumn(tile.col), mWallPlacement(wallplacement) {
	
}

bool State::IsTerminal() {
	//need to differentiate between win or lose to award points to the play
	if (mAI) {
		if (mRow == 0)
			return true;


		return false;
	}
	else {
		if (mRow == 12)
			return true;


		return false;
	}

}