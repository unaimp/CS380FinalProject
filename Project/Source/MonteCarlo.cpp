#include <Stdafx.h>


namespace MonteCarlo {
	MonteCarloTree::MonteCarloTree(void) : 
		mRoot(nullptr),
		mCurrentIterations(0),
		mMinimumVisitedTimes(1),
		mUCTvar(2.f),
		mMaximumIterations(100),
		mSimulator(nullptr),
		mAIPlayer(nullptr)
	{
		mSimulator = new Simulator;
	}

	MonteCarloTree::~MonteCarloTree(void) {
		delete mSimulator;
	}


	void MonteCarloTree::Start(void) {
		mCurrentIterations = 0;

		if (!mAIPlayer) {
			mAIPlayer = &g_database.Find("NPC")->GetQuoridor();
			mHumanPlayer = &g_database.Find("Player")->GetQuoridor();
		}
		//reset stats
		mAIWinTimes = 0;
		mPlayerWinTimes = 0;

		//create root node from AI position
		mRoot = new Node();
		mRoot->mVisitedTimes = 1; //not to simulate from here, first create possible children
		mRoot->mState = new State(TileQ(mHumanPlayer->GetTile().row, mHumanPlayer->GetTile().col), false, false);
	}

	void MonteCarloTree::End(void) {
		//erase all tree
		Eraser(mRoot);

		mRoot = nullptr;
	}

	void MonteCarloTree::Eraser(Node* n) {
		for (auto& it : n->mChildren) {
			Eraser(it);
		}

		delete n;
	}

	void MonteCarloTree::Execute(void) {
		Start();

		srand(time(nullptr));

		while (mCurrentIterations < mMaximumIterations) {
			//clone actual map
			g_terrain.CloneMap();

			mCurrentIterations++;

			//Principle of operations
			Node* leafNode = Selection(mRoot);
			Node* expandedNode = Expansion(leafNode);
			bool simulationValue = Simulation(expandedNode);
			BackPropagation(expandedNode, simulationValue);
		}

		ActualMove();
	}

	void MonteCarloTree::ActualMove() {
		int higherValue = -10000;
		Node* betterOption = nullptr;

		std::cout << std::endl << std::endl;

		mStats.clear();
		std::stringstream ss;

		const TileQ& currentTile = mAIPlayer->GetTile();
		//Pick the better move
		for (auto& it : mRoot->mChildren) {
			//print out data
			if(it->mState->mWallPlacement)
				ss << "Placing a wall in:  " << it->mState->mTile.row << ", " << it->mState->mTile.col << "has a value of: " << it->mTotalSimulationReward << std::endl;
			else
				ss << "Moving to: " << it->mState->mTile.row << ", " << it->mState->mTile.col << " has value of: " << it->mTotalSimulationReward << std::endl;

			if (it->mTotalSimulationReward > higherValue) {
				higherValue = it->mTotalSimulationReward;
				betterOption = it;
			}
		}


		//print out data
		if (betterOption->mState->mWallPlacement) {
			ss << "Placing a wall in: "<< betterOption->mState->mTile.row << ", " << betterOption->mState->mTile.col  <<  std::endl;
			mAIPlayer->SetWall(betterOption->mState->mTile);
			mAIPlayer->m_walls--;
		}
		else {
			ss << std::endl << "AI moving to: " << betterOption->mState->mTile.row << ", " << betterOption->mState->mTile.col << std::endl;
			//actually move the AI player on the board
			//erase color from previous one
			mAIPlayer->SetTile(mAIPlayer->GetTile(), false);
			//set new tile position
			mAIPlayer->m_tile = betterOption->mState->mTile;
		}

		ss << "Simulation stats:" << std::endl;

		ss << "Player win rate = " << (static_cast<float>(mPlayerWinTimes) / mMaximumIterations) * 100.f << "%" << std::endl;
		ss << "AI     win rate = " << (static_cast<float>(mAIWinTimes) / mMaximumIterations) * 100.f << "%" << std::endl;

		mStats = ss.str();

		
		//erase the created tree
		End();
	}


	//PRINCIPLE OF OPERATIONS ----------------------------------------------------------------------------------------------------------------

	/*Start from root R and select successive child nodes until a leaf node L is reached. 
	The root is the current game state and a leaf is any node that has a potential child from which
	no simulation (playout) has yet been initiated.
	Upper Confidence Bound applied to trees or shortly UCT*/
	MonteCarloTree::Node* MonteCarloTree::Selection(Node* currentNode) {
		//get to a leaf node
		while (currentNode->mChildren.empty() == false) {
			//Use UCT formula
			float bestUCT = -10000.f;
			Node* betterOption = nullptr;
			//Use UCT formula to determine where to explore the tree from
			for (auto& it : currentNode->mChildren) {
				//if never visited UCT calculation will be +inf
				if (it->mVisitedTimes == 0) {
					betterOption = it;
					break;
				}

				// balance between expansion and exploration
				float uct_value = it->mTotalSimulationReward / it->mVisitedTimes + 
					mUCTvar * sqrt(log(currentNode->mVisitedTimes) / it->mVisitedTimes);

				if (uct_value > bestUCT) {
					bestUCT = uct_value;
					betterOption = it;
				}
			}

			//move onel level downwards in the tree by selecting the child with better UCT value
			currentNode = betterOption;
		}

		//better UCT leaf node
		return currentNode;
	}

	/*create one (or more) child nodes and choose node C from one of them. Child nodes are any valid moves 
	from the game position defined by L.*/
	MonteCarloTree::Node* MonteCarloTree::Expansion(Node* leafNode) {
		//if the leaf node has not been visited yet
		if (leafNode->mVisitedTimes == 0) {
			//rollout from this node
			return leafNode;
		}
		else {
			//create this node children actions and rollout from one of them
			if(leafNode->mAI)
				leafNode->CreateChildren(mHumanPlayer);
			else
				leafNode->CreateChildren(mAIPlayer);

			return leafNode->mChildren[0];
		}
	}


	/* Complete one random playout from node C
	A playout may be as simple as choosing uniform random moves until the game is decided */
	bool MonteCarloTree::Simulation(Node* node) {
		//get player and AI position for simulating
		TileQ playerTile, aiTile;
		//depending on which's  turn it is, the current node will be whose turn is, 
		// and the parent of this node will be the others
		//if no parent it will be current board position
		if (node->mAI) {
			aiTile = node->mState->mTile;
			if (node->mParent)
				playerTile = node->mParent->mState->mTile;
			else
				playerTile = mHumanPlayer->GetTile();
		}
		else {
			playerTile = node->mState->mTile;
				if (node->mParent)
					aiTile = node->mParent->mState->mTile;
			else
				aiTile = mAIPlayer->GetTile();
		}

		//place the wall if this move does that
		if (node->mState->mWallPlacement) {
			if (node->mAI) {
				TileQ wall = mSimulator->PlaceWall(playerTile, true);
				mAIPlayer->SetWallClone(wall);
				node->mState->mTile = wall;
			}
			else {
				TileQ wall = mSimulator->PlaceWall(aiTile, false);
				mHumanPlayer->SetWallClone(wall);
				node->mState->mTile = wall;
			}
		}


		bool simValue = mSimulator->Simulate(aiTile, playerTile, mAIPlayer, mHumanPlayer, node->mAI);

		//stats
		if (simValue)
			mAIWinTimes++;
		else
			mPlayerWinTimes++;

		return simValue;
	}

	/*Use the result of the playout to update information in the nodes on the path from C to R.*/
	void MonteCarloTree::BackPropagation(Node* node, bool aiWon) {
		if (node == nullptr)		return; //root node

		//update values
		if(aiWon)
			node->mTotalSimulationReward ++;

		node->mVisitedTimes++;

		BackPropagation(node->mParent, aiWon);
	}


	bool Simulator::Simulate(const TileQ& AITile, const TileQ& playerTile, QuoridorPlayer* AI, QuoridorPlayer* player, bool AIStarts) {
		State AIState = State(AITile, true, false);
		State playerState = State(playerTile, false, false);

		//During simulation the moves are chosen with respect to a function called rollout policy function (which has some biased stats)
		while (1) {
			if (AIStarts) {
				AIStarts = true;
				//AI simulation turn
				if (AIState.IsTerminal()) {
					return true; //AI WIN
				}
				AIState = RollOut(AIState, AI, player, true);
				//set new position on clone map
				if(mAIRow >= 0)
					AI->SetTileClone(TileQ(mAIRow, mAIColumn), false);
				mAIRow = AIState.mTile.row;		mAIColumn = AIState.mTile.col;
				AI->SetTileClone(TileQ(mAIRow, mAIColumn), true);
			}

			//player simulation turn
			if (playerState.IsTerminal()) {
				return false; //Human WIN
			}
			playerState = RollOut(playerState, player, AI, false);
			//set new position on clone map
			if (mPlayerRow >= 0)
				AI->SetTileClone(TileQ(mPlayerRow, mPlayerColumn), false);
			mPlayerRow = playerState.mTile.row;		mPlayerColumn = playerState.mTile.col;
			AI->SetTileClone(TileQ(mPlayerRow, mPlayerColumn), true);
		}
	}

	State& Simulator::RollOut(const State currentState, QuoridorPlayer* movingQuoridor, QuoridorPlayer* otherPlayer, 
		bool AITurn, std::vector<Moves>& posibleMoves) {
		//moves are not created yet
		if (posibleMoves.empty()) {
			//BIASED ACTIONS
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			if (movingQuoridor->GetWalls() > 0) {
				posibleMoves.push_back(Moves::M_PLACE_WALL);
				posibleMoves.push_back(Moves::M_PLACE_WALL);
				posibleMoves.push_back(Moves::M_PLACE_WALL);
			}
			posibleMoves.push_back(Moves::M_MOVE_RIGHT);
			posibleMoves.push_back(Moves::M_MOVE_RIGHT);
			posibleMoves.push_back(Moves::M_MOVE_LEFT);
			posibleMoves.push_back(Moves::M_MOVE_LEFT);
			posibleMoves.push_back(Moves::M_MOVE_BACKWD);
		}

		//randomizer for next move
		int randomNumber = rand() % posibleMoves.size();


		TileQ destinationTile;

		switch (posibleMoves[randomNumber]) {
		case Moves::M_MOVE_FWD:
			if (AITurn)
				destinationTile = TileQ(currentState.mTile.row - 1, currentState.mTile.col);
			else
				destinationTile = TileQ(currentState.mTile.row + 1, currentState.mTile.col);
			break;
		case Moves::M_MOVE_RIGHT:
			destinationTile = TileQ(currentState.mTile.row, currentState.mTile.col + 1);
			break;
		case Moves::M_MOVE_LEFT:
			destinationTile = TileQ(currentState.mTile.row, currentState.mTile.col - 1);
			break;
		case Moves::M_MOVE_BACKWD:
			if (AITurn)
				destinationTile = TileQ(currentState.mTile.row + 1, currentState.mTile.col);
			else
				destinationTile = TileQ(currentState.mTile.row - 1, currentState.mTile.col);

			break;
		case Moves::M_PLACE_WALL:
			TileQ otherPlayerTile = otherPlayer->GetTile();
			movingQuoridor->SetWallClone(PlaceWall(otherPlayerTile, AITurn));
			return State(currentState.mTile, AITurn, true);
		}

		if (movingQuoridor->IsLegalMove(currentState.mTile, destinationTile)) {
			return State(destinationTile, AITurn, false);
		}
		else {
			//erase this possibility from the vector
			Moves movetoDelete = posibleMoves[randomNumber];
			for (auto it = posibleMoves.begin(); it != posibleMoves.end(); ) {
				if (*it == movetoDelete) {
					it = posibleMoves.erase(it);
				}
				else {
					it++;
				}
			}
			return RollOut(currentState, movingQuoridor, otherPlayer, AITurn, posibleMoves);
		}
	}

	//Places a wall using propagation logic
	const TileQ& Simulator::PlaceWall(const TileQ& origin, bool propagation_up, float decay, float growing)
	{
		int width = g_terrain.GetWidth();

		g_terrain.GetCloneMap().SetCentralInfluence(origin.row, origin.col);

		QuoridorPlayer quoridor(g_database.Find("NPC")->GetQuoridor());

		if (!propagation_up)
			quoridor = g_database.Find("Player")->GetQuoridor();

		std::vector<float> m_terrainTempMap; // row * with + col
		m_terrainTempMap.resize(width * width);

		// Iterate through all tiles
		for (int row = 0; row < width; row++)
		{
			int rmin, rmax;
			if (propagation_up)
			{
				rmin = row;
				rmax = min(row + 1, width - 1);
			}
			else
			{
				rmin = max(row - 1, 0);
				rmax = row;
			}

			for (int col = 0; col < width; col++)
			{
				float N = 0.f;
				if (g_terrain.GetInfluenceMapValue(row, col) < 0.f)
				{
					m_terrainTempMap[row * width + col] = g_terrain.GetInfluenceMapValue(row, col);
					continue;
				}
				// Checking the neighbours
				for (int r = rmin; r <= rmax; r++)
				{
					for (int c = max(col - 1, 0); c <= min(col + 1, width - 1); c++)
					{
						// Wall in the way, skip
						if (quoridor.WallCheck2(row, col, r, c))
							break;
						// If center skip
						if (r == row && c == col)
							continue;
						// Getting the absolute of the neighbour
						float dist = sqrtf((r - row) * (r - row) + (c - col) * (c - col));
						float n = g_terrain.GetInfluenceMapValue(r, c) * exp(-dist * decay);
						// Updating the highest absolute of all neighbour influence
						if (abs(n) > abs(N))
							N = n;
					}
				}
				// Update temp layer
				m_terrainTempMap[row * width + col] = g_terrain.Lerp(g_terrain.GetInfluenceMapValue(row, col), N, growing);
			}
		}

		// Setting the influence values into the map
		for (int row = 0; row < width; row++)
		{
			for (int col = 0; col < width; col++)
			{
				if (!(row % 3 == 2 && col % 3 == 2))
				{
					if (row % 3 == 2)
						g_terrain.SetInfluenceCloneMapValue(row, col, m_terrainTempMap[row * width + col]);
					if (col % 3 == 2)
						g_terrain.SetInfluenceCloneMapValue(row, col, m_terrainTempMap[row * width + col] - m_terrainTempMap[row * width + col] / 1000.f);
				}
			}
		}

		float max_influence = 0.f;
		TileQ wall_tile;

		// Setting the influence values into the map
		for (int row = 0; row < width; row++)
		{
			for (int col = 0; col < width; col++)
			{
				float influence = g_terrain.GetCloneMap().GetInfluence(row, col);
				if (influence != 0.f && influence != 1.f)
				{
					if (influence > max_influence)
					{
						TileQ quoridor_tile(row / 3, col / 3);
						if (row % 3 == 2)
						{
							quoridor_tile.half_row = true;
							if (quoridor.IsLegalWall(quoridor.GetTile(), quoridor_tile))
							{
								max_influence = influence;
								wall_tile = quoridor_tile;
								continue;
							}
							else if (quoridor_tile.row != 0)
							{
								quoridor_tile.row--;
								if (quoridor.IsLegalWall(quoridor.GetTile(), quoridor_tile))
								{
									max_influence = influence;
									wall_tile = quoridor_tile;
									continue;
								}
							}
						}
						if (col % 3 == 2)
						{
							quoridor_tile.half_col = true;
							if (quoridor.IsLegalWall(quoridor.GetTile(), quoridor_tile))
							{
								max_influence = influence;
								wall_tile = quoridor_tile;
								continue;
							}
							else if (quoridor_tile.col != 0)
							{
								quoridor_tile.col--;
								if (quoridor.IsLegalWall(quoridor.GetTile(), quoridor_tile))
								{
									max_influence = influence;
									wall_tile = quoridor_tile;
									continue;
								}
							}
						}

					}
				}
			}
		}

		return wall_tile;

	}

	MonteCarloTree::Node::Node() : mParent(nullptr), mState(nullptr), mVisitedTimes(0u), mTotalSimulationReward(0.), mAI(false) {

	}

	MonteCarloTree::Node::Node(Node* parent, TileQ tile, bool wallPlacement, bool AI) : mVisitedTimes(0u), mTotalSimulationReward(0.), mAI(AI),
	mState(new State(tile, mAI, wallPlacement)){
		//add parent as parent pointer
		mParent = parent;

		//add this node to the children vector of parent
		parent->mChildren.push_back(this);
	}

	MonteCarloTree::Node::~Node() {
		delete mState;
	}

	void MonteCarloTree::Node::CreateChildren(QuoridorPlayer* q) {
		//get grandparent node (the previous move for the plyaer)
		Node* grandParent = this->mParent ? this->mParent : nullptr;
		//if not grandparent get current board position state
		State previousState = grandParent ? *grandParent->mState 
			: State(q->GetTile(), !this->mAI, false);

		TileQ up(previousState.mTile.row + 1, previousState.mTile.col);
		TileQ right(previousState.mTile.row, previousState.mTile.col + 1);
		TileQ left(previousState.mTile.row, previousState.mTile.col - 1);
		TileQ down(previousState.mTile.row - 1, previousState.mTile.col);

		//create all possible moves
		if (q->IsLegalMove(TileQ(previousState.mTile.row, previousState.mTile.col), up))
			new Node(this, up, false, previousState.mAI);

		//wall placement
		if(q->GetWalls() > 0)
			new Node(this, TileQ(previousState.mTile.row, previousState.mTile.col), true, previousState.mAI);

		if (q->IsLegalMove(TileQ(previousState.mTile.row, previousState.mTile.col), right))
			new Node(this, right, false, previousState.mAI);
		//
		if (q->IsLegalMove(TileQ(previousState.mTile.row, previousState.mTile.col), left))
			new Node(this, left, false, previousState.mAI);
		//
		if (q->IsLegalMove(TileQ(previousState.mTile.row, previousState.mTile.col), down))
			new Node(this, down, false, previousState.mAI);
	}
}