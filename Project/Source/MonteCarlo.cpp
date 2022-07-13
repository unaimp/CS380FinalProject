#include <Stdafx.h>


namespace MonteCarlo {
	MonteCarloTree::MonteCarloTree(void) : 
		mRoot(nullptr),
		mCurrentIterations(0),
		mMinimumVisitedTimes(1),
		mUCTvar(2.f),
		mMaximumIterations(1000),
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
		//create root node from AI position

		mRoot = new Node();
		mRoot->mState = new State(TileQ(mAIPlayer->GetTile().row, mAIPlayer->GetTile().col), true, false);
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
			double simulationValue = Simulation(expandedNode);
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


		//std::cout << "AI move to:" << betterOption->mState->mRow << ", " << betterOption->mState->mColumn << std::endl;
		//actually move the AI player on the board
		//erase color from previous one
		mAIPlayer->SetTile(TileQ(mRoot->mState->mRow, mRoot->mState->mColumn), false);
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
			leafNode->CreateChildren(mAIPlayer);
			return leafNode->mChildren[0];
		}
	}


	/* Complete one random playout from node C
	A playout may be as simple as choosing uniform random moves until the game is decided */
	double MonteCarloTree::Simulation(Node* node) {
		return mSimulator->Simulate(node, mAIPlayer, mHumanPlayer);
	}

	/*Use the result of the playout to update information in the nodes on the path from C to R.*/
	void MonteCarloTree::BackPropagation(Node* node, double simValue) {
		if (node == nullptr)		return; //root node

		//update values
		node->mTotalSimulationReward += simValue;
		node->mVisitedTimes++;

		BackPropagation(node->mParent, simValue);
	}


	double Simulator::Simulate(MonteCarloTree::Node* startingPoint, QuoridorPlayer* AI, QuoridorPlayer* player) {
		int numberOfActions = 0;
		State AIState = *startingPoint->mState;
		State playerState = State(TileQ(player->m_tile.row, player->m_tile.col), false, false);

		double AIWIN = 0;
		//During simulation the moves are chosen with respect to a function called rollout policy function (which has some biased stats)
		while (1) {
			numberOfActions++;

			//AI simulation turn
			if (AIState.IsTerminal()) {
				AIWIN = 1.;
				break;
			}
			AIState = RollOut(AIState, AI, true);
			mAIRow = AIState.mRow;		mAIColumn = AIState.mColumn;

			//player simulation turn
			if (playerState.IsTerminal()) {
				AIWIN = -1.;
				break;
			}
			playerState = RollOut(playerState, player, false);
			mPlayerRow = playerState.mRow;		mPlayerColumn = playerState.mColumn;
		}

		//set the value of the simulation
		return AIWIN / numberOfActions;
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

	MonteCarloTree::Node::Node() : mParent(nullptr), mState(nullptr) {

	}

	MonteCarloTree::Node::Node(Node* parent, TileQ tile, bool wallPlacement) : mVisitedTimes(0u), mTotalSimulationReward(0.),
	mState(new State(tile, true, wallPlacement)){
		//add parent as parent pointer
		mParent = parent;

		//add this node to the children vector of parent
		parent->mChildren.push_back(this);
	}

	MonteCarloTree::Node::~Node() {
		delete mState;
	}

	void MonteCarloTree::Node::CreateChildren(QuoridorPlayer* q) {
		TileQ fwd(mState->mRow - 1, mState->mColumn);
		TileQ right(mState->mRow, mState->mColumn + 1);
		TileQ left(mState->mRow, mState->mColumn - 1);
		TileQ backwd(mState->mRow + 1, mState->mColumn);

		//create all possible moves
		if (q->IsLegalMove(TileQ(mState->mRow, mState->mColumn), fwd))
			new Node(this, fwd, false);

		//wall placement
		//new Node(this, TileQ(this->mState->mRow, this->mState->mColumn), true);

		if (q->IsLegalMove(TileQ(mState->mRow, mState->mColumn), right))
			new Node(this, right, false);

		if (q->IsLegalMove(TileQ(mState->mRow, mState->mColumn), left))
			new Node(this, left, false);

		if (q->IsLegalMove(TileQ(mState->mRow, mState->mColumn), backwd))
			new Node(this, backwd, false);
	}
}