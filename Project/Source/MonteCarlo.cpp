#include <Stdafx.h>


namespace MonteCarlo {
	MonteCarloTree::MonteCarloTree(void) : 
		mRoot(nullptr),
		mCurrentIterations(0),
		mMinimumVisitedTimes(1),
		mUCTvar(2.f),
		mMaximumIterations(3000),
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

		//clone current map
		g_terrain.CloneMap();
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
		double higherValue = -10000.;
		Node* betterOption = nullptr;

		std::cout << std::endl << std::endl;

		mStats.clear();
		std::stringstream ss;

		//Pick the better move
		for (auto& it : mRoot->mChildren) {
			//print out data
			ss << "Moving to: " << it->mState->mRow << ", " << it->mState->mColumn << " has value of: " << it->mTotalSimulationReward << std::endl;
			if (it->mTotalSimulationReward > higherValue) {
				higherValue = it->mTotalSimulationReward;
				betterOption = it;
			}
		}
		ss << std::endl<< "AI moving to: " << betterOption->mState->mRow << ", " << betterOption->mState->mColumn << std::endl;

		ss << "Simulation stats:" << std::endl;

		ss << "Player win rate = " << (static_cast<float>(mPlayerWinTimes) / mMaximumIterations) * 100.f << "%" << std::endl;
		ss << "AI     win rate = " << (static_cast<float>(mAIWinTimes) / mMaximumIterations) * 100.f << "%" << std::endl;

		mStats = ss.str();

		//actually move the AI player on the board
		//erase color from previous one
		mAIPlayer->SetTile(mAIPlayer->GetTile(), false);
		//set new tile position
		mAIPlayer->m_tile = TileQ(betterOption->mState->mRow, betterOption->mState->mColumn);
		
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
			aiTile = TileQ(node->mState->mRow, node->mState->mColumn);
			if (node->mParent)
				playerTile = TileQ(node->mParent->mState->mRow, node->mParent->mState->mColumn);
			else
				playerTile = mHumanPlayer->GetTile();
		}
		else {
			playerTile = TileQ(node->mState->mRow, node->mState->mColumn);
			if (node->mParent)
				aiTile = TileQ(node->mParent->mState->mRow, node->mParent->mState->mColumn);
			else
				aiTile = mAIPlayer->GetTile();
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


	bool Simulator::Simulate(TileQ& AITile, TileQ& playerTile, QuoridorPlayer* AI, QuoridorPlayer* player, bool AIStarts) {
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
				AIState = RollOut(AIState, AI, true);
				//set new position on clone map
				if(mAIRow >= 0)
					AI->SetTileClone(TileQ(mAIRow, mAIColumn), false);
				mAIRow = AIState.mRow;		mAIColumn = AIState.mColumn;
				AI->SetTileClone(TileQ(mAIRow, mAIColumn), true);
			}

			//player simulation turn
			if (playerState.IsTerminal()) {
				return false; //Human WIN
			}
			playerState = RollOut(playerState, player, false);
			//set new position on clone map
			if (mPlayerRow >= 0)
				AI->SetTileClone(TileQ(mPlayerRow, mPlayerColumn), false);
			mPlayerRow = playerState.mRow;		mPlayerColumn = playerState.mColumn;
			AI->SetTileClone(TileQ(mPlayerRow, mPlayerColumn), true);
		}
	}

	State& Simulator::RollOut(const State currentState, QuoridorPlayer* q,  bool AITurn, std::vector<Moves>& posibleMoves) {
		//moves are not created yet
		if (posibleMoves.empty()) {
			//BIASED ACTIONS
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			//posibleMoves.push_back(Moves::M_PLACE_WALL);
			//posibleMoves.push_back(Moves::M_PLACE_WALL);
			//posibleMoves.push_back(Moves::M_PLACE_WALL);
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
				destinationTile = TileQ(currentState.mRow - 1, currentState.mColumn);
			else
				destinationTile = TileQ(currentState.mRow + 1, currentState.mColumn);
			break;
		case Moves::M_MOVE_RIGHT:
			destinationTile = TileQ(currentState.mRow, currentState.mColumn + 1);
			break;
		case Moves::M_MOVE_LEFT:
			destinationTile = TileQ(currentState.mRow, currentState.mColumn - 1);
			break;
		case Moves::M_MOVE_BACKWD:
			if (AITurn)
				destinationTile = TileQ(currentState.mRow + 1, currentState.mColumn);
			else
				destinationTile = TileQ(currentState.mRow - 1, currentState.mColumn);

			break;
		case Moves::M_PLACE_WALL:
			PlaceWall();
			return State(TileQ(currentState.mRow, currentState.mColumn), AITurn, true);
		}

		if (q->IsLegalMove(TileQ(currentState.mRow, currentState.mColumn), destinationTile)) {
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
			return RollOut(currentState, q, AITurn, posibleMoves);
		}
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

		TileQ up(previousState.mRow + 1, previousState.mColumn);
		TileQ right(previousState.mRow, previousState.mColumn + 1);
		TileQ left(previousState.mRow, previousState.mColumn - 1);
		TileQ down(previousState.mRow - 1, previousState.mColumn);

		//create all possible moves
		if (q->IsLegalMove(TileQ(previousState.mRow, previousState.mColumn), up))
			new Node(this, up, false, previousState.mAI);

		//wall placement
		//new Node(this, TileQ(this->mState->mRow, this->mState->mColumn), true, , !this->mAI);

		if (q->IsLegalMove(TileQ(previousState.mRow, previousState.mColumn), right))
			new Node(this, right, false, previousState.mAI);
		//
		if (q->IsLegalMove(TileQ(previousState.mRow, previousState.mColumn), left))
			new Node(this, left, false, previousState.mAI);
		//
		if (q->IsLegalMove(TileQ(previousState.mRow, previousState.mColumn), down))
			new Node(this, down, false, previousState.mAI);
	}

	TileQ MonteCarloTree::Propagation(TileQ origin, float decay, float growing)
	{
		int width = g_terrain.GetWidth();

		g_terrain.GetCloneMap().SetCentralInfluence(origin.row, origin.col);
	
		std::vector<float> m_terrainTempMap; // row * with + col
		m_terrainTempMap.resize(width * width);
	
		// Iterate through all tiles
		for (int row = 0; row < width; row++)
		{
			for (int col = 0; col < width; col++)
			{
				float N = 0.f;
				if (g_terrain.GetInfluenceMapValue(row,col) < 0.f)
				{
					m_terrainTempMap[row * width + col] = g_terrain.GetInfluenceMapValue(row,col);
					continue;
				}
				// Checking the neighbours
				for (int r = row; r <= min(row + 1, width - 1); r++)
				{
					for (int c = max(col - 1, 0); c <= min(col + 1, width - 1); c++)
					{
						// Wall in the way, skip
						if (mAIPlayer->WallCheck2(row, col, r, c))
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
					if(row % 3 == 2)
						g_terrain.SetInfluenceCloneMapValue(row, col, m_terrainTempMap[row * width + col]);
					if (col % 3 == 2)
						g_terrain.SetInfluenceCloneMapValue(row, col, m_terrainTempMap[row * width + col] - m_terrainTempMap[row * width + col]/1000.f);
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
							if (mAIPlayer->IsLegalWall(mAIPlayer->GetTile(), quoridor_tile))
							{
								max_influence = influence;
								wall_tile = quoridor_tile;
								continue;
							}
							else if (row != 0)
							{
								quoridor_tile.row--;
								if (mAIPlayer->IsLegalWall(mAIPlayer->GetTile(), quoridor_tile))
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
							if (mAIPlayer->IsLegalWall(mAIPlayer->GetTile(), quoridor_tile))
							{
								max_influence = influence;
								wall_tile = quoridor_tile;
								continue;
							}
							else if (col != 0)
							{
								quoridor_tile.col--;
								if (mAIPlayer->IsLegalWall(mAIPlayer->GetTile(), quoridor_tile))
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
}