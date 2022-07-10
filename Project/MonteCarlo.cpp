#include "MonteCarlo.h"


namespace MonteCarlo {
	MonteCarloTree::MonteCarloTree(void) : 
		mRoot(nullptr),
		mCurrentIterations(0),
		mMinimumVisitedTimes(1),
		mUCTvar(2.f),
		mMaximumIterations(200),
		mSimulator(nullptr)
	{
		mSimulator = new Simulator;
	}

	MonteCarloTree::~MonteCarloTree(void) {
		delete mSimulator;
	}


	void MonteCarloTree::Start(void) {
		mCurrentIterations = 0;

		//create root node from AI position
		int row = 3; int col = 6; //get from board dataaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

		mRoot = new Node();
		mRoot->mState = new State(Moves::M_NONE, row, col, true);
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

		//Pick the better move
		for (auto& it : mRoot->mChildren) {
			if (it->mTotalSimulationReward > higherValue) {
				higherValue = it->mTotalSimulationReward;
				betterOption = it;
			}
		}

		//actually move the AI player on the board
		//...
		
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
			leafNode->CreateChildren();
			return leafNode->mChildren[0];
		}
	}


	/* Complete one random playout from node C
	A playout may be as simple as choosing uniform random moves until the game is decided */
	double MonteCarloTree::Simulation(Node* node) {
		return mSimulator->Simulate(node);
	}

	/*Use the result of the playout to update information in the nodes on the path from C to R.*/
	void MonteCarloTree::BackPropagation(Node* node, double simValue) {
		if (node == nullptr)		return; //root node

		//update values
		node->mTotalSimulationReward += simValue;
		node->mVisitedTimes++;

		BackPropagation(node->mParent, simValue);
	}


	double Simulator::Simulate(MonteCarloTree::Node* startingPoint) {
		int numberOfActions = 0;
		State AIState = *startingPoint->mState;
		State playerState = State(Moves::M_NONE, 0, 0/*playerPos*/);

		double AIWIN = 0;
		//During simulation the moves are chosen with respect to a function called rollout policy function (which has some biased stats)
		while (1) {
			numberOfActions++;

			//AI simulation turn
			if (AIState.IsTerminal()) {
				AIWIN = 1.;
				break;
			}
			AIState = RollOut(AIState, true);
			mAIRow = AIState.mRow;		mAIColumn = AIState.mColumn;

			//player simulation turn
			if (playerState.IsTerminal()) {
				AIWIN = -1.;
				break;
			}
			playerState = RollOut(playerState, false);
			mPlayerRow = playerState.mRow;		mPlayerColumn = playerState.mColumn;
		}

		//set the value of the simulation
		return AIWIN / numberOfActions;
	}

	State Simulator::RollOut(const State currentState, bool AITurn) {
		//biased randomizer for next move
		int r = rand() % 10;

		//NEED TO CHECK IF POSSIBLEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
		if (r < 6)
			return State(Moves::M_MOVE_UP, currentState.mRow, currentState.mColumn, AITurn);
		if (r >= 6 && r < 8)
			return State(Moves::M_MOVE_RIGHT, currentState.mRow, currentState.mColumn, AITurn);
		if (r >= 8 && r < 10)
			return State(Moves::M_MOVE_LEFT, currentState.mRow, currentState.mColumn, AITurn);
		if (r == 10)
			return State(Moves::M_MOVE_DOWN, currentState.mRow, currentState.mColumn, AITurn);
		//if (r < 5)
		//	return State(Moves::M_MOVE_UP, currentState.mRow, currentState.mColumn);
	}

	MonteCarloTree::Node::Node() : mParent(nullptr), mState(nullptr) {

	}

	MonteCarloTree::Node::Node(Node* parent, Moves m) : mVisitedTimes(0u), mTotalSimulationReward(0.),
	mState(new State(m, parent->mState->mRow, parent->mState->mColumn)){
		//add parent as parent pointer
		mParent = parent;

		//add this node to the children vector of parent
		parent->mChildren.push_back(this);
	}

	MonteCarloTree::Node::~Node() {
		delete mState;
	}

	void MonteCarloTree::Node::CreateChildren() {
		//create all possible moves
		//if possible
		new Node(this, Moves::M_MOVE_UP);
		//if possible
		new Node(this, Moves::M_MOVE_RIGHT);
		//if possible
		new Node(this, Moves::M_MOVE_LEFT);
		//if possible
		new Node(this, Moves::M_MOVE_DOWN);
		//if possible
		//new Node(this, Moves::M_MOVE_WALL);
	}
}