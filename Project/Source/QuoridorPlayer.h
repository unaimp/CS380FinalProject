#pragma once

struct TileQ
{
	int row, col;
	bool half_row, half_col;
	TileQ(): row(0), col(0), half_row(false), half_col(false) {}
	TileQ(int r, int c): row(r), col(c), half_row(false), half_col(false) {}
	TileQ(int r, int c, bool hr, bool hc): row(r), col(c), half_row(hr), half_col(hc) {}
};

class QuoridorPlayer
{
public:

	QuoridorPlayer(bool type, TileQ tile);

	void Update();

	void SetPlayerInTile(int row, int col);

	void SetTile(TileQ tile, bool ontile);
	void SetTileTemp(TileQ tile);

	void SetWall(TileQ tile);
	void SetWallTemp(TileQ tile);

	void ResetTemp();

	bool IsLegalMove(TileQ origin_tile, TileQ& tile);
	bool IsLegalWall(TileQ origin_tile, TileQ tile);

	bool IsLegalWallAfterPathfinding(TileQ origin_tile, TileQ tile);
	void SetTileClone(TileQ tile, bool ontile);
	void SetWallClone(const TileQ& tile);

	TileQ GetTile() { return m_tile; }
	unsigned GetWalls() { return m_walls; }

	// Pathfinding stuff
	struct a_node_data
	{
		int parentRow; // Parent node (-1 is start node)
		int parentCol; // Parent node (-1 is start node)

		float cost;    // Cost to get this node, g(x)

		float total;   // Total cost, g(x) + h(x)

		bool onOpen;   // On Open List
		bool onClosed; // On Closed List

		a_node_data() : parentRow(-1), parentCol(-1), cost(INFINITY), total(INFINITY), onOpen(false), onClosed(false) {}
	};

	bool PathCheck(int r0, int c0, int r1, int r2);
	bool IsClearPath(int r0, int c0, int r1, int c1);
	float HeuristicCost(int fr, int fc, int r, int c);
	int GetCheapestNode(int* parentR, int* parentC);
	void IdToCoords(int id, int width, int* r, int* c);
	float AdjacentChildCost(int or , int oc, int r, int c);
	bool IsDiagonal(int or , int oc, int r, int c);
	void AFillWaypoint(int r, int c);
	bool WallCheck(int or , int oc, int r, int c);
	bool WallCheck2(int or , int oc, int r, int c);

	TileQ m_tile;
	unsigned m_walls;
private:
	bool m_type;

	// Pathfinding variables
	int m_width;
	std::vector<a_node_data> m_nodes;

	std::vector<int> m_open_list;
	std::vector<int> m_closed_list;
	std::list<D3DXVECTOR3> m_waypointList;
};
