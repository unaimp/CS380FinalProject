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
		//Pick the better move
		for (auto& it : mRoot->mChildren) {
			//print out data
			std::cout << "Moving to:" << it->mState->mRow << ", " << it->mState->mColumn << " has value of: " << it->mTotalSimulationReward << std::endl;
			if (it->mTotalSimulationReward > higherValue) {
				higherValue = it->mTotalSimulationReward;
				betterOption = it;
			}
		}
		std::cout << std::endl<< "AI moving to: " << betterOption->mState->mRow << ", " << betterOption->mState->mColumn << std::endl;

		std::cout << "Simulation stats:" << std::endl;

		std::cout << "Player win rate = " << static_cast<float>(mPlayerWinTimes) / mMaximumIterations << std::endl;
		std::cout << "AI     win rate = " << static_cast<float>(mAIWinTimes) / mMaximumIterations << std::endl;

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
				//if(mAIRow >= 0)
				//	AI->SetTileClone(TileQ(mAIRow, mAIColumn), false);
				mAIRow = AIState.mRow;		mAIColumn = AIState.mColumn;
				//AI->SetTileClone(TileQ(mAIRow, mAIColumn), true);
			}

			//player simulation turn
			if (playerState.IsTerminal()) {
				return false; //Human WIN
			}
			playerState = RollOut(playerState, player, false);
			//set new position on clone map
			//if (mPlayerRow >= 0)
			//	AI->SetTileClone(TileQ(mPlayerRow, mPlayerColumn), false);
			mPlayerRow = playerState.mRow;		mPlayerColumn = playerState.mColumn;
			//AI->SetTileClone(TileQ(mPlayerRow, mPlayerColumn), true);
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
			for (auto it = posibleMoves.begin(); it != posibleMoves.end(); ) {
				if (*it == posibleMoves[randomNumber]) {
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
}