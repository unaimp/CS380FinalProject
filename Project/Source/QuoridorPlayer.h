#pragma once

struct TileQ
{
	int row, col;
	TileQ(): row(0), col(0) {}
	TileQ(int r, int c): row(r), col(c) {}
};

class QuoridorPlayer
{
public:

	QuoridorPlayer(bool type, TileQ tile);

	void Update();

	void SetPlayerInTile(int row, int col);

	void SetTile(TileQ tile, bool ontile);
	void SetTileTemp(TileQ tile);

	void ResetTemp();

	bool IsLegalMove(TileQ origin_tile, TileQ tile);

	TileQ GetTile() { return m_tile; }
	unsigned GetWalls() { return m_walls; }
	unsigned GetStairs() { return m_stairs; }

	TileQ m_tile;
private:
	bool m_type;
	unsigned m_walls;
	unsigned m_stairs;

};
