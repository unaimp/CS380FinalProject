#include <Stdafx.h>


namespace MonteCarlo {
	MonteCarloTree::MonteCarloTree(void) : 
		mRoot(nullptr),
		mCurrentIterations(0),
		mMinimumVisitedTimes(1),
		mUCTvar(2.f),
		mSimulator(nullptr),
		mAIPlayer(nullptr)
	{
		mSimulator = new Simulator;
	}

	MonteCarloTree::~MonteCarloTree(void) {
		delete mSimulator;
	}


	void MonteCarloTree::Start(void) {
		mMaximumIterations = 10000;

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
		mRoot->mState = new State(mHumanPlayer->GetTile(), false, false);
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

			mHumanPlayer->m_simulation_walls = mHumanPlayer->m_walls;
			mAIPlayer->m_simulation_walls = mAIPlayer->m_walls;

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

		mStats.clear();
		std::stringstream ss;

		const TileQ& currentTile = mAIPlayer->GetTile();
		//Pick the better move
		for (auto& it : mRoot->mChildren) {
			//print out data
			if(it->mState->mWallPlacement)
				ss << "Placing a wall in:  " << it->mState->mWall.row << ", " << it->mState->mWall.col << " has a value of: " << it->mTotalSimulationReward
				<< ". Visited times: " << it->mVisitedTimes << std::endl;
			else
				ss << "Moving to: " << it->mState->mTile.row << ", " << it->mState->mTile.col << " has value of: " << it->mTotalSimulationReward 
				<<". Visited times: " << it->mVisitedTimes << std::endl;

			if (it->mTotalSimulationReward > higherValue) {
				higherValue = it->mTotalSimulationReward;
				betterOption = it;
			}
		}

		//actual move
		if (betterOption->mState->mWallPlacement) {
			ss << "Placing a wall in: "<< betterOption->mState->mWall.row << ", " << betterOption->mState->mWall.col  <<  std::endl;
			mAIPlayer->SetWall(betterOption->mState->mWall);
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
				//set walls in the map if this node is wall placement
				if (currentNode->mState->mWallPlacement) {
					if (currentNode->mAI) {
						mAIPlayer->SetWallClone(currentNode->mState->mWall);
						mAIPlayer->m_simulation_walls--;
					}
					else {
						mHumanPlayer->SetWallClone(currentNode->mState->mWall);
						mHumanPlayer->m_simulation_walls--;
					}
				}

				//if never visited UCT calculation will be +inf
				if (it->mVisitedTimes == 0) {
					betterOption = it;
					break;
				}

				//compute the winrate of the node
				float winRate = static_cast<float>(it->mTotalSimulationReward) / it->mVisitedTimes;
				//if this is the player move this will be our loserate
				if (!it->mAI)
					winRate = 1.f - winRate; 

				// balance between expansion and exploration
				float uct_value = winRate + mUCTvar * sqrt(log(currentNode->mVisitedTimes) / it->mVisitedTimes);

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
		//if the leaf node has not been visited yet or it is a terminal state
		if (leafNode->mVisitedTimes == 0 || leafNode->mState->IsTerminal()) {
			//rollout from this node
			return leafNode;
		}
		else {
			//create this node children actions and rollout from one of them
			leafNode->CreateChildren(mAIPlayer, mHumanPlayer);

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

		mSimulator->mAIRow = aiTile.row;				mSimulator->mAIColumn = aiTile.col;
		mSimulator->mPlayerRow = playerTile.row;		mSimulator->mPlayerColumn = playerTile.col;

		//place the wall if this move does that
		if (node->mState->mWallPlacement) {
			if (node->mAI) {
				mAIPlayer->SetWallClone(node->mState->mWall);
				mAIPlayer->m_simulation_walls--;
			}
			else {
				mHumanPlayer->SetWallClone(node->mState->mWall);
				mHumanPlayer->m_simulation_walls--;
			}
		}

		bool simValue = mSimulator->Simulate(aiTile, playerTile, mAIPlayer, mHumanPlayer, !node->mAI);

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
		if(aiWon /*&& node->mAI	||		!aiWon && !node->mAI*/)
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

				//set new position on clone map if not wall placement
				AI->SetTileClone(TileQ(mAIRow, mAIColumn), false);
				mAIRow = AIState.mTile.row;		mAIColumn = AIState.mTile.col;
				AI->SetTileClone(TileQ(mAIRow, mAIColumn), true);
			}

			//player simulation turn
			if (playerState.IsTerminal()) {
				return false; //Human WIN
			}
			playerState = RollOut(playerState, player, AI, false);

			//set new position on clone map if not wall placement
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
			//BIASED ACTIONS, highly expected to move forward if possible
			for(int i = 0; i < 20; i++)
				posibleMoves.push_back(Moves::M_MOVE_FWD);
			posibleMoves.push_back(Moves::M_MOVE_RIGHT);
			posibleMoves.push_back(Moves::M_MOVE_RIGHT);
			posibleMoves.push_back(Moves::M_MOVE_LEFT);
			posibleMoves.push_back(Moves::M_MOVE_LEFT);
			posibleMoves.push_back(Moves::M_MOVE_BACKWD);
			
		}

		srand(time(nullptr));
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

	MonteCarloTree::Node::Node() : mParent(nullptr), mState(nullptr), mVisitedTimes(0u), mTotalSimulationReward(0), mAI(false) {

	}

	MonteCarloTree::Node::Node(Node* parent, TileQ tile, bool wallPlacement, bool AI, TileQ wall) : mVisitedTimes(0u), mTotalSimulationReward(0), mAI(AI),
	mState(new State(tile, AI, wallPlacement, wall)){
		//add parent as parent pointer
		mParent = parent;

		//add this node to the children vector of parent
		parent->mChildren.push_back(this);
	}

	MonteCarloTree::Node::~Node() {
		delete mState;
	}

	void MonteCarloTree::Node::CreateChildren(QuoridorPlayer* AI, QuoridorPlayer* player) {
		//get current tree node position for both players
		TileQ aiTile, humanTile;
		//if this node has a parent, meaning this is not the first move
		if (this->mParent) {
			if (this->mAI) {
				aiTile = this->mState->mTile;
				humanTile = this->mParent->mState->mTile;
			}
			else {
				 humanTile = this->mState->mTile;
				 aiTile = this->mParent->mState->mTile;
			}
		}
		else {
			//this is first move, need to actually take board position
			humanTile = this->mState->mTile;
			aiTile = AI->GetTile();
		}

		TileQ previousTile = this->mAI ? humanTile : aiTile;
		QuoridorPlayer* q = this->mAI ? player : AI;

		TileQ up(previousTile.row + 1, previousTile.col);
		TileQ right(previousTile.row, previousTile.col + 1);
		TileQ left(previousTile.row, previousTile.col - 1);
		TileQ down(previousTile.row - 1, previousTile.col);


		//create all possible moves
		if (q->IsLegalMove(previousTile, down))
			new Node(this, down, false, !this->mAI);
		if (q->IsLegalMove(previousTile, up))
			new Node(this, up, false, !this->mAI);
		if (q->IsLegalMove(previousTile, right))
			new Node(this, right, false, !this->mAI);
		if (q->IsLegalMove(previousTile, left))
			new Node(this, left, false, !this->mAI);

		//create all wall placement
		if (q->GetWalls() > 0) {
			if (this->mAI) {
				//if (q->IsLegalWall(previousTile, TileQ(aiTile.row - 1, aiTile.col, true, false))) {
				//	new Node(this, previousTile, true, !this->mAI, TileQ(aiTile.row - 1, aiTile.col, true, false));
				//}
				//if (q->IsLegalWall(previousTile, TileQ(aiTile.row - 1, aiTile.col - 1, true, false))){
				//	new Node(this, previousTile, true, !this->mAI, TileQ(aiTile.row - 1, aiTile.col - 1, true, false));
				//}
				//if (q->IsLegalWall(previousTile, TileQ(aiTile.row - 1, aiTile.col, false, true))) {
				//	new Node(this, previousTile, true, !this->mAI, TileQ(aiTile.row - 1, aiTile.col, false, true));
				//}
				//if (q->IsLegalWall(previousTile, TileQ(aiTile.row, aiTile.col, false, true))) {
				//}
				//if (q->IsLegalWall(previousTile, TileQ(aiTile.row - 1, aiTile.col - 1, false, true))){
				//	new Node(this, previousTile, true, !this->mAI, TileQ(aiTile.row - 1, aiTile.col - 1, false, true));
				//}
				//if ( q->IsLegalWall(previousTile, TileQ(aiTile.row, aiTile.col - 1, false, true)))
				//	new Node(this, previousTile, true, !this->mAI, TileQ(aiTile.row, aiTile.col - 1, false, true));
			}
			else {
				if (q->IsLegalWall(previousTile, TileQ(humanTile.row, humanTile.col, true, false))){
					new Node(this, previousTile, true, !this->mAI, TileQ(humanTile.row, humanTile.col, true, false));
				}
				else if (q->IsLegalWall(previousTile, TileQ(humanTile.row, humanTile.col - 1, true, false))){
					new Node(this, previousTile, true, !this->mAI, TileQ(humanTile.row, humanTile.col - 1, true, false));
				}
				if (q->IsLegalWall(previousTile, TileQ(humanTile.row, humanTile.col, false, true))){
					new Node(this, previousTile, true, !this->mAI, TileQ(humanTile.row , humanTile.col, false, true));
				}
				else if (q->IsLegalWall(previousTile, TileQ(humanTile.row - 1, humanTile.col, false, true))) {
					new Node(this, previousTile, true, !this->mAI, TileQ(humanTile.row - 1, humanTile.col, false, true));
				}
				//if (q->IsLegalWall(previousTile, TileQ(humanTile.row, humanTile.col - 1, false, true))){
				//	new Node(this, previousTile, true, !this->mAI, TileQ(humanTile.row , humanTile.col - 1, false, true));
				//}
				//else if (q->IsLegalWall(previousTile, TileQ(humanTile.row - 1, humanTile.col - 1, false, true)))
				//	new Node(this, previousTile, true, !this->mAI, TileQ(humanTile.row - 1, humanTile.col - 1, false, true));
			}
		}
	}
}