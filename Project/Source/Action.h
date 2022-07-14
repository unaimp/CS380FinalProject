

namespace MonteCarlo {

	enum Moves {
		M_NONE,
		M_MOVE_FWD,
		M_PLACE_WALL,
		M_MOVE_RIGHT,
		M_MOVE_LEFT,
		M_MOVE_BACKWD
	};


	struct State {
		State(TileQ tile, bool AI, bool wallplacement);
		//State of the player from where take following actions
		TileQ mTile;
		bool mWallPlacement;

		bool IsTerminal();
		bool mAI;
	};
}