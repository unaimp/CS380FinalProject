#include <Stdafx.h>


namespace MonteCarlo {
	MonteCarloTree::MonteCarloTree(void) : 
		mRoot(nullptr),
		mCurrentIterations(0),
		mMinimumVisitedTimes(1),
		mUCTvar(2.f),
		mMaximumIterations(4000),
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

			//set how many walls each player has before simulation
			mHumanPlayer->m_Simulation_walls = mHumanPlayer->m_walls;
			mAIPlayer->m_Simulation_walls = mAIPlayer->m_walls;

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
				ss << "Placing a wall in:  " << it->mState->mTile.row << ", " << it->mState->mTile.col << " has a value of: " << it->mTotalSimulationReward
				<< ". Visited times: " << it->mVisitedTimes << std::endl;
			else
				ss << "Moving to: " << it->mState->mTile.row << ", " << it->mState->mTile.col << " has value of: " << it->mTotalSimulationReward 
				<<". Visited times: " << it->mVisitedTimes << std::endl;

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
				TileQ wall;
				do {
					wall = mSimulator->PlaceWallRandom(playerTile, mAIPlayer);
				} while (wall.row == -1);

				mAIPlayer->SetWallClone(wall);
				mAIPlayer->m_Simulation_walls--;
			}
			else {
				TileQ wall;
				do {
					wall = mSimulator->PlaceWallRandom(playerTile, mHumanPlayer);
				} while (wall.row == -1);

				mHumanPlayer->SetWallClone(wall);
				mHumanPlayer->m_Simulation_walls--;
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
			node->mTotalSimulationReward++;

		node->mVisitedTimes++;

		BackPropagation(node->mParent, aiWon);
	}


	Simulator::Simulator() :
		mAIRow(-1), mAIColumn(-1),
		mPlayerRow(-1), mPlayerColumn(-1) {}

	bool Simulator::Simulate(const TileQ& AITile, const TileQ& playerTile, QuoridorPlayer* AI, QuoridorPlayer* player, bool AIStarts) {
		State AIState = State(AITile, true, false);
		State playerState = State(playerTile, false, false);

		//During simulation the moves are chosen with respect to a function called rollout policy function (which has some biased stats)
		while (1) {
			if (AIStarts) {
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
				player->SetTileClone(TileQ(mPlayerRow, mPlayerColumn), false);
			mPlayerRow = playerState.mTile.row;		mPlayerColumn = playerState.mTile.col;
			player->SetTileClone(TileQ(mPlayerRow, mPlayerColumn), true);

			AIStarts = true;
		}
	}

	State Simulator::RollOut(const State currentState, QuoridorPlayer* movingQuoridor, QuoridorPlayer* otherPlayer, 
		bool AITurn, std::vector<Moves>& posibleMoves) {
		//moves are not created yet
		if (posibleMoves.empty()) {
			//BIASED ACTIONS
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_FWD);
			if (movingQuoridor->m_Simulation_walls > 0) {
				//posibleMoves.push_back(Moves::M_PLACE_WALL);
				//posibleMoves.push_back(Moves::M_PLACE_WALL);
				//posibleMoves.push_back(Moves::M_PLACE_WALL);
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

		bool wall_check = false;
		TileQ wall;

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
			do {
				std::cout << "trying to place a wall in simulation" << std::endl;
				TileQ origin = movingQuoridor->GetTile();
				wall = PlaceWallRandom(origin, movingQuoridor);
			} while (wall.row == -1);

			movingQuoridor->SetWallClone(wall);
			movingQuoridor->m_Simulation_walls--;
			return State(currentState.mTile, AITurn, true);

		case Moves::M_WALL_FWD_A:
			wall_check = true;
			if (AITurn)
				destinationTile = TileQ(mPlayerRow, mPlayerColumn, true, false);
			else
				destinationTile = TileQ(mAIRow - 1, mAIColumn, true, false);

			break;
		case Moves::M_WALL_FWD_B:
			wall_check = true;
			if (AITurn)
				destinationTile = TileQ(mPlayerRow, mPlayerColumn - 1, true, false);
			else
				destinationTile = TileQ(mAIRow - 1, mAIColumn - 1, true, false);

			break;
		case Moves::M_WALL_RIGHT_A:
			wall_check = true;
			if (AITurn)
				destinationTile = TileQ(mPlayerRow, mPlayerColumn, false, true);
			else
				destinationTile = TileQ(mAIRow - 1, mAIColumn, false, true);

			break;
		case Moves::M_WALL_RIGHT_B:
			wall_check = true;
			if (AITurn)
				destinationTile = TileQ(mPlayerRow - 1, mPlayerColumn, false, true);
			else
				destinationTile = TileQ(mAIRow, mAIColumn, false, true);
		case Moves::M_WALL_LEFT_A:
			wall_check = true;
			if (AITurn)
				destinationTile = TileQ(mPlayerRow, mPlayerColumn - 1, false, true);
			else
				destinationTile = TileQ(mAIRow - 1, mAIColumn - 1, false, true);

			break;
		case Moves::M_WALL_LEFT_B:
			wall_check = true;
			if (AITurn)
				destinationTile = TileQ(mPlayerRow - 1, mPlayerColumn - 1, false, true);
			else
				destinationTile = TileQ(mAIRow, mAIColumn - 1, false, true);

			break;
		}

		if (!wall_check)
		{
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
		else
		{
			if (movingQuoridor->IsLegalWall(currentState.mTile, destinationTile)) {
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
	}

	/*void Simulator::InfluenceUp(TileQ tile, float influence, int separation_value, bool last_was_separation)
	{
		int row = tile.row * 3, col = tile.col * 3, width = g_terrain.GetWidth();
		if (row+2 == g_terrain.GetWidth() && influence <= 0.f)
			return;

		if (!g_terrain.IsWallClone(row + 2, col))
		{
			g_terrain.SetInfluenceCloneMapValue(row + 2, col + 0, influence);
			g_terrain.SetInfluenceCloneMapValue(row + 2, col + 1, influence);
			
			//InfluenceUp(TileQ(tile.row+1, tile.col), influence - 0.1f, separation_value, false);
		}
		else if (separation_value != 0 && !last_was_separation)
		{
			InfluenceLeft(TileQ(tile.row, tile.col), influence, separation_value - 1, true, true);
			InfluenceRight(TileQ(tile.row, tile.col), influence, separation_value - 1, true, true);
		}
	}

	void Simulator::InfluenceDown(TileQ tile, float influence, int separation_value, bool last_was_separation)
	{
		int row = tile.row * 3, col = tile.col * 3, width = g_terrain.GetWidth();
		if (row == 0 && influence <= 0.f)
			return;

		if (!g_terrain.IsWallClone(row - 1, col))
		{
			g_terrain.SetInfluenceCloneMapValue(row - 1, col + 0, influence);
			g_terrain.SetInfluenceCloneMapValue(row - 1, col + 1, influence);

			InfluenceDown(TileQ(tile.row - 1, tile.col), influence - 0.1f, separation_value, false);
		}
		else if (separation_value != 0 && !last_was_separation)
		{
			InfluenceLeft(TileQ(tile.row, tile.col), influence, separation_value - 1, true, false);
			InfluenceRight(TileQ(tile.row, tile.col), influence, separation_value - 1, true, false);
		}
	}

	void Simulator::InfluenceLeft(TileQ tile, float influence, int separation_value, bool last_was_separation, bool go_up)
	{
		if (influence <= 0.f)
			return;

		int row = tile.row * 3, col = tile.col * 3, width = g_terrain.GetWidth();

		if (col != 0 && !g_terrain.IsWallClone(row, col - 1))
		{
			g_terrain.SetInfluenceCloneMapValue(row + 0, col - 1, influence);
			g_terrain.SetInfluenceCloneMapValue(row + 1, col - 1, influence);

			InfluenceLeft(TileQ(tile.row, tile.col - 1), influence - 0.1f, separation_value, false, go_up);
		}
		else if (separation_value != 0 && !last_was_separation)
		{
			if (go_up)
				InfluenceUp(TileQ(tile.row, tile.col), influence, separation_value - 1, true);
			else
				InfluenceDown(TileQ(tile.row, tile.col), influence, separation_value - 1, true);
		}
	}

	void Simulator::InfluenceRight(TileQ tile, float influence, int separation_value, bool last_was_separation, bool go_up)
	{
		if (influence <= 0.f)
			return;

		int row = tile.row * 3, col = tile.col * 3, width = g_terrain.GetWidth();

		if (col != 0 && !g_terrain.IsWallClone(row, col + 2))
		{
			g_terrain.SetInfluenceCloneMapValue(row + 0, col + 2, influence);
			g_terrain.SetInfluenceCloneMapValue(row + 1, col + 2, influence);

			InfluenceRight(TileQ(tile.row, tile.col + 1), influence - 0.1f, separation_value, false, go_up);
		}
		else if (separation_value != 0 && !last_was_separation)
		{
			if (go_up)
				InfluenceUp(TileQ(tile.row, tile.col), influence, separation_value - 1, true);
			else
				InfluenceDown(TileQ(tile.row, tile.col), influence, separation_value - 1, true);
		}
	}*/

	//Places a wall using propagation logic
	TileQ Simulator::PlaceWall(const TileQ& origin, bool propagation_up, float decay, float growing)
	{
		int width = g_terrain.GetWidth();

		QuoridorPlayer quoridor(g_database.Find("NPC")->GetQuoridor());

		if (!propagation_up)
			quoridor = g_database.Find("Player")->GetQuoridor();

		int row = origin.row * 3, col = origin.col * 3;

		if (propagation_up)
		{
			// Trying up wall
			if (quoridor.IsLegalWall(origin, TileQ(origin.row, origin.col, true, false)))
				return TileQ(origin.row, origin.col, true, false);
			if (quoridor.IsLegalWall(origin, TileQ(origin.row, origin.col-1, true, false)))
				return TileQ(origin.row, origin.col - 1, true, false);
		}
		else
		{
			// Trying down wall
			if (quoridor.IsLegalWall(origin, TileQ(origin.row-1, origin.col, true, false)))
				return TileQ(origin.row - 1, origin.col, true, false);
			if (quoridor.IsLegalWall(origin, TileQ(origin.row-1, origin.col-1, true, false)))
				return TileQ(origin.row - 1, origin.col - 1, true, false);
		}

		// Trying left wall
		if (quoridor.IsLegalWall(origin, TileQ(origin.row, origin.col-1, false, true)))
			return TileQ(origin.row, origin.col - 1, false, true);
		if (quoridor.IsLegalWall(origin, TileQ(origin.row-1, origin.col-1, false, true)))
			return TileQ(origin.row - 1, origin.col - 1, false, true);

		// Trying right wall
		if (quoridor.IsLegalWall(origin, TileQ(origin.row, origin.col, false, true)))
			return TileQ(origin.row, origin.col, false, true);
		if (quoridor.IsLegalWall(origin, TileQ(origin.row-1, origin.col, false, true)))
			return TileQ(origin.row - 1, origin.col, false, true);

		// THIS IS A NON VALID WALL PLACEMENT
		return TileQ();

		//std::vector<float> m_terrainTempMap; // row * with + col
		//m_terrainTempMap.resize(width * width);

		//if (propagation_up)
		//	InfluenceUp(origin, 1.f, 0, false);
		//else
		//	InfluenceDown(origin, 1.f, 0, false);

		//// Setting the influence values into the map
		//for (int row = 0; row < width; row++)
		//{
		//	for (int col = 0; col < width; col++)
		//	{
		//		g_terrain.SetInfluenceCloneMapValue(row, col, m_terrainTempMap[row * width + col]);
		//	}
		//}

		/*float max_influence = 0.f;
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
							//else if (quoridor_tile.row != 0)
							//{
							//	quoridor_tile.col--;
							//	if (quoridor.IsLegalWall(quoridor.GetTile(), quoridor_tile))
							//	{
							//		max_influence = influence;
							//		wall_tile = quoridor_tile;
							//		continue;
							//	}
							//}
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
							//else if (quoridor_tile.col != 0)
							//{
							//	quoridor_tile.row--;
							//	if (quoridor.IsLegalWall(quoridor.GetTile(), quoridor_tile))
							//	{
							//		max_influence = influence;
							//		wall_tile = quoridor_tile;
							//		continue;
							//	}
							//}
						}

					}
				}
			}
		}

		return wall_tile;*/

	}

	TileQ Simulator::PlaceWallRandom(const TileQ& origin, QuoridorPlayer* q)
	{
		srand(time(NULL));

		TileQ random_tile;
		random_tile.row = rand() % (8 - 0 + 1) + 0;
		random_tile.col = rand() % (8 - 0 + 1) + 0;
		if (rand() % 2 == 0)
			random_tile.half_row = true;
		else
			random_tile.half_col = true;

		if (q->IsLegalWall(origin, random_tile))
			return random_tile;
		else
			return TileQ();
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
		//
		if (q->IsLegalMove(TileQ(previousState.mTile.row, previousState.mTile.col), down))
			new Node(this, down, false, previousState.mAI);

		//wall placement
		if(q->GetWalls() > 0)
			new Node(this, previousState.mTile, true, previousState.mAI);

		if (q->IsLegalMove(TileQ(previousState.mTile.row, previousState.mTile.col), right))
			new Node(this, right, false, previousState.mAI);
		//
		if (q->IsLegalMove(TileQ(previousState.mTile.row, previousState.mTile.col), left))
			new Node(this, left, false, previousState.mAI);
	}
}