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

	bool IsLegalMove(TileQ origin_tile, TileQ tile);
	bool IsLegalWall(TileQ origin_tile, TileQ tile);

	TileQ GetTile() { return m_tile; }
	unsigned GetWalls() { return m_walls; }
	unsigned GetStairs() { return m_stairs; }

private:
	bool m_type;
	TileQ m_tile;
	unsigned m_walls;
	unsigned m_stairs;

};
