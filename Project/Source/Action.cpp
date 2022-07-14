#include <Stdafx.h>

using namespace MonteCarlo;


State::State(TileQ tile, bool AI, bool wallplacement)
	: mAI(AI), mTile(tile), mWallPlacement(wallplacement) {
	
}

bool State::IsTerminal() {
	//need to differentiate between win or lose to award points to the play
	if (mAI) {
		if (mTile.row == 0)
			return true;


		return false;
	}
	else {
		if (mTile.row == 12)
			return true;


		return false;
	}

}