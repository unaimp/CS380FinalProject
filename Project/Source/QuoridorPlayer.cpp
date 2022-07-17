
#include <Stdafx.h>

QuoridorPlayer::QuoridorPlayer(bool type, TileQ tile) : m_type(type), m_walls(5)
{
	m_tile.row = tile.row;
	m_tile.col = tile.col;
}

void QuoridorPlayer::SetPlayerInTile(int row, int col)
{
	m_tile.row = row;
	m_tile.col = col;
}

void QuoridorPlayer::SetTile(TileQ tile, bool ontile)
{
	DebugDrawingColor color = DEBUG_COLOR_WHITE;
	if (ontile)
	{
		// If this is the player
		if (m_type)
			color = DEBUG_COLOR_BLUE;
		// If this is the npc
		else
			color = DEBUG_COLOR_RED;
	}

	// Setting 2x2 tile
	g_terrain.SetColor(tile.row * 3, tile.col * 3, color);
	g_terrain.SetColor(tile.row * 3 + 1, tile.col * 3, color);
	g_terrain.SetColor(tile.row * 3, tile.col * 3 + 1, color);
	g_terrain.SetColor(tile.row * 3 + 1, tile.col * 3 + 1, color);
}

void QuoridorPlayer::SetTileTemp(TileQ tile)
{
	DebugDrawingColor color = DEBUG_COLOR_CYAN;

	// Setting 2x2 tile
	g_terrain.SetColor(tile.row * 3, tile.col * 3, color);
	g_terrain.SetColor(tile.row * 3 + 1, tile.col * 3, color);
	g_terrain.SetColor(tile.row * 3, tile.col * 3 + 1, color);
	g_terrain.SetColor(tile.row * 3 + 1, tile.col * 3 + 1, color);

	// Setting temp terrain
	g_terrain.SetInfluenceMapValue(tile.row * 3, tile.col * 3, 1.f);
	g_terrain.SetInfluenceMapValue(tile.row * 3 + 1, tile.col * 3, 1.f);
	g_terrain.SetInfluenceMapValue(tile.row * 3, tile.col * 3 + 1, 1.f);
	g_terrain.SetInfluenceMapValue(tile.row * 3 + 1, tile.col * 3 + 1, 1.f);

}

void QuoridorPlayer::SetWall(TileQ tile)
{
	DebugDrawingColor color = DEBUG_COLOR_BLACK;

	int width = g_terrain.GetWidth() / 3;

	// Setting 2x1 wall
	if (tile.half_row)
	{
		int offset = 0;
		if (tile.col == width)
			offset = -3;

		if (g_terrain.GetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset) == TILE_WALL)
			return;

		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset, color);
		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset + 1, color);
		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset + 2, color);
		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset + 3, color);
		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset + 4, color);

		// Setting tiles
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset, TILE_WALL);
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset + 1, TILE_WALL);
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset + 2, TILE_WALL);
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset + 3, TILE_WALL);
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset + 4, TILE_WALL);

		// Setting temp terrain
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset, -1.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset + 1, -1.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset + 2, -1.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset + 3, -1.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset + 4, -1.f);
	}

	// Setting 1x2 wall
	if (tile.half_col)
	{
		int offset = 0;
		if (tile.row == width)
			offset = -3;

		if (g_terrain.GetTerrainTile(tile.row * 3 + offset, tile.col * 3 + 2) == TILE_WALL)
			return;

		g_terrain.SetColor(tile.row * 3 + offset, tile.col * 3 + 2, color);
		g_terrain.SetColor(tile.row * 3 + offset + 1, tile.col * 3 + 2, color);
		g_terrain.SetColor(tile.row * 3 + offset + 2, tile.col * 3 + 2, color);
		g_terrain.SetColor(tile.row * 3 + offset + 3, tile.col * 3 + 2, color);
		g_terrain.SetColor(tile.row * 3 + offset + 4, tile.col * 3 + 2, color);

		// Setting tiles
		g_terrain.SetTerrainTile(tile.row * 3 + offset, tile.col * 3 + 2, TILE_WALL);
		g_terrain.SetTerrainTile(tile.row * 3 + offset + 1, tile.col * 3 + 2, TILE_WALL);
		g_terrain.SetTerrainTile(tile.row * 3 + offset + 2, tile.col * 3 + 2, TILE_WALL);
		g_terrain.SetTerrainTile(tile.row * 3 + offset + 3, tile.col * 3 + 2, TILE_WALL);
		g_terrain.SetTerrainTile(tile.row * 3 + offset + 4, tile.col * 3 + 2, TILE_WALL);

		// Setting temp terrain
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset, tile.col * 3 + 2, -1.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset + 1, tile.col * 3 + 2, -1.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset + 2, tile.col * 3 + 2, -1.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset + 3, tile.col * 3 + 2, -1.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset + 4, tile.col * 3 + 2, -1.f);
	}
}

void QuoridorPlayer::SetWallTemp(TileQ tile)
{
	DebugDrawingColor color = DEBUG_COLOR_YELLOW;

	int width = g_terrain.GetWidth() / 3;

	// Setting 2x1 wall
	if (tile.half_row)
	{
		int offset = 0;
		if (tile.col == width)
			offset = -3;

		if (g_terrain.GetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset) == TILE_WALL)
			return;

		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset, color);
		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset + 1, color);
		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset + 2, color);
		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset + 3, color);
		g_terrain.SetColor(tile.row * 3 + 2, tile.col * 3 + offset + 4, color);

		// Setting tiles
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset, TILE_EMPTY);
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset + 1, TILE_EMPTY);
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset + 2, TILE_EMPTY);
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset + 3, TILE_EMPTY);
		g_terrain.SetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset + 4, TILE_EMPTY);

		// Setting temp terrain
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset, 2.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset + 1, 2.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset + 2, 2.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset + 3, 2.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + 2, tile.col * 3 + offset + 4, 2.f);
	}

	// Setting 1x2 wall
	if (tile.half_col)
	{
		int offset = 0;
		if (tile.row == width)
			offset = -3;

		if (g_terrain.GetTerrainTile(tile.row * 3 + offset, tile.col * 3 + 2) == TILE_WALL)
			return;

		g_terrain.SetColor(tile.row * 3 + offset, tile.col * 3 + 2, color);
		g_terrain.SetColor(tile.row * 3 + offset + 1, tile.col * 3 + 2, color);
		g_terrain.SetColor(tile.row * 3 + offset + 2, tile.col * 3 + 2, color);
		g_terrain.SetColor(tile.row * 3 + offset + 3, tile.col * 3 + 2, color);
		g_terrain.SetColor(tile.row * 3 + offset + 4, tile.col * 3 + 2, color);

		// Setting tiles
		g_terrain.SetTerrainTile(tile.row * 3 + offset, tile.col * 3 + 2, TILE_EMPTY);
		g_terrain.SetTerrainTile(tile.row * 3 + offset + 1, tile.col * 3 + 2, TILE_EMPTY);
		g_terrain.SetTerrainTile(tile.row * 3 + offset + 2, tile.col * 3 + 2, TILE_EMPTY);
		g_terrain.SetTerrainTile(tile.row * 3 + offset + 3, tile.col * 3 + 2, TILE_EMPTY);
		g_terrain.SetTerrainTile(tile.row * 3 + offset + 4, tile.col * 3 + 2, TILE_EMPTY);

		// Setting temp terrain
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset, tile.col * 3 + 2, 2.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset + 1, tile.col * 3 + 2, 2.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset + 2, tile.col * 3 + 2, 2.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset + 3, tile.col * 3 + 2, 2.f);
		g_terrain.SetInfluenceMapValue(tile.row * 3 + offset + 4, tile.col * 3 + 2, 2.f);
	}
}

void QuoridorPlayer::ResetTemp()
{
	for (int r = 0; r < g_terrain.GetWidth(); r++)
	{
		for (int c = 0; c < g_terrain.GetWidth(); c++)
		{
			if (g_terrain.GetInfluenceMapValue(r, c) == 1.f)
				g_terrain.SetColor(r, c, DEBUG_COLOR_WHITE);
			if (g_terrain.GetInfluenceMapValue(r, c) == 2.f)
			{
				g_terrain.SetColor(r, c, DEBUG_COLOR_GRAY);
				g_terrain.SetTerrainTile(r, c, TILE_IN_BETWEEN);
			}
			g_terrain.SetInfluenceMapValue(r, c, 0.f);
		}
	}
}

bool QuoridorPlayer::IsLegalMove(TileQ origin_tile, TileQ& tile)
{
	TileQ bottom_left_tile(tile.row * 3, tile.col * 3);

	int width = g_terrain.GetWidth() / 3;

	// Moving left check
	if (origin_tile.col != 0)
	{
		if (origin_tile.row == tile.row && origin_tile.col - 1 == tile.col &&
			!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 2))
		{
			if(!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col))
			{
				// Normal left skip
				if (tile.col != 0 && !g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 1))   // No second wall
					tile.col -= 1;
				// Diagonal left skip
				else
				{
					if (tile.row == width ||
						(g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col) &&
						!g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col))) // There's a wall up
						tile.row -= 1;
					else if (tile.row == 0 ||
						(g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col) &&
						!g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col))) // There's a wall down
						tile.row += 1;
					// If this is the player
					else if (m_type)
					{
						D3DXVECTOR3 temp = g_terrain.GetMousePos();
						TileQ target;
						g_terrain.GetRowColumn(&temp, &target.row, &target.col);

						// Bounce down
						if (target.row % 3 == 0)
							tile.row -= 1;
						// Bounce up
						if (target.row % 3 == 1)
							tile.row += 1;
					}
					// If this is the npc
					else
					{
						srand(time(NULL));
						if(rand() % 2 == 0)
							tile.row -= 1;
						else
							tile.row += 1;
					}
				}
			}
			return true;
		}
	}
	// Moving right check
	if (origin_tile.col != width)
	{
		if (origin_tile.row == tile.row && origin_tile.col+1 == tile.col &&
			!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col-1))
		{
			if (!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col))
			{
				// Normal right skip
				if (tile.col != width && !g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 2))   // No second wall
					tile.col += 1;
				// Diagonal right skip
				else
				{
					if (tile.row == width ||
						(g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col) &&
						!g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col))) // There's a wall up
						tile.row -= 1;
					else if (tile.row == 0 ||
						(g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col) &&
						!g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col))) // There's a wall down
						tile.row += 1;
					// If this is the player
					else if (m_type)
					{
						D3DXVECTOR3 temp = g_terrain.GetMousePos();
						TileQ target;
						g_terrain.GetRowColumn(&temp, &target.row, &target.col);

						// Bounce down
						if (target.row % 3 == 0)
							tile.row -= 1;
						// Bounce up
						if (target.row % 3 == 1)
							tile.row += 1;
					}
					// If this is the npc
					else
					{
						srand(time(NULL));
						if (rand() % 2 == 0)
							tile.row -= 1;
						else
							tile.row += 1;
					}
				}
			}
			return true;
		}
	}
	// Moving down check
	if (origin_tile.row != 0)
	{
		if (origin_tile.row-1 == tile.row && origin_tile.col == tile.col &&
			!g_terrain.IsWallClone(bottom_left_tile.row+2, bottom_left_tile.col))
		{
			if (!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col))
			{
				// Normal down skip
				if (tile.row != 0 && !g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col))   // No second wall
					tile.row -= 1;
				// Diagonal down skip
				else
				{
					if (tile.col == width ||
						(g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 2) &&
						!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 1))) // There's a wall right
						tile.col -= 1;
					else if (tile.col == 0 ||
						(g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 1) &&
						!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 2))) // There's a wall left
						tile.col += 1;
					// If this is the player
					else if (m_type)
					{
						D3DXVECTOR3 temp = g_terrain.GetMousePos();
						TileQ target;
						g_terrain.GetRowColumn(&temp, &target.row, &target.col);

						// Bounce down
						if (target.col % 3 == 0)
							tile.col -= 1;
						// Bounce up
						if (target.row % 3 == 1)
							tile.col += 1;
					}
					// If this is the npc
					else
					{
						srand(time(NULL));
						if (rand() % 2 == 0)
							tile.col -= 1;
						else
							tile.col += 1;
					}
				}
			}
			return true;
		}
	}
	// Moving up check
	if (origin_tile.row != width)
	{
		if (origin_tile.row+1 == tile.row && origin_tile.col == tile.col &&
			!g_terrain.IsWallClone(bottom_left_tile.row-1, bottom_left_tile.col))
		{
			if (!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col))
			{
				// Normal up skip
				if (tile.row != width && !g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col))   // No second wall
					tile.row += 1;
				// Diagonal up skip
				else
				{
					if (tile.col == width ||
						(g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 2) &&
						!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 1))) // There's a wall right
						tile.col -= 1;
					else if (tile.col == 0 ||
						(g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 1) &&
						!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 2))) // There's a wall left
						tile.col += 1;
					// If this is the player
					else if (m_type)
					{
						D3DXVECTOR3 temp = g_terrain.GetMousePos();
						TileQ target;
						g_terrain.GetRowColumn(&temp, &target.row, &target.col);

						// Bounce down
						if (target.col % 3 == 0)
							tile.col -= 1;
						// Bounce up
						if (target.row % 3 == 1)
							tile.col += 1;
					}
					// If this is the npc
					else
					{
						srand(time(NULL));
						if (rand() % 2 == 0)
							tile.col -= 1;
						else
							tile.col += 1;
					}
				}
			}
			return true;
		}
	}



	// Moving left over player without obstacle check
	if (origin_tile.col > 1)
	{
		if (origin_tile.row == tile.row && origin_tile.col-2 == tile.col &&    // Target is this case
			!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col+5) && // No first wall
			!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col+2) && // No second wall
			!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col+3))  // There's a player to skip over
			return true;
	}
	// Moving right over player without obstacle check
	if (origin_tile.col < (width-1))
	{
		if (origin_tile.row == tile.row && origin_tile.col+2 == tile.col &&    // Target is this case
			!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col-4) && // No first wall
			!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col-1) && // No second wall
			!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col-2))  // There's a player to skip over
			return true;
	}
	// Moving down over player without obstacle check
	if (origin_tile.row > 1)
	{
		if (origin_tile.row-2 == tile.row && origin_tile.col == tile.col &&    // Target is this case
			!g_terrain.IsWallClone(bottom_left_tile.row+5, bottom_left_tile.col) && // No first wall
			!g_terrain.IsWallClone(bottom_left_tile.row+2, bottom_left_tile.col) && // No second wall
			!g_terrain.IsBlank(bottom_left_tile.row+3, bottom_left_tile.col))  // There's a player to skip over
			return true;
	}
	// Moving up over player without obstacle check
	if (origin_tile.row < (width-1))
	{
		if (origin_tile.row+2 == tile.row && origin_tile.col == tile.col &&    // Target is this case
			!g_terrain.IsWallClone(bottom_left_tile.row-4, bottom_left_tile.col) && // No first wall
			!g_terrain.IsWallClone(bottom_left_tile.row-1, bottom_left_tile.col) && // No second wall
			!g_terrain.IsBlank(bottom_left_tile.row-2, bottom_left_tile.col))  // There's a player to skip over
			return true;
	}

	bottom_left_tile.row = origin_tile.row * 3;
	bottom_left_tile.col = origin_tile.col * 3;

	// Moving left over player with obstacle check
	if (origin_tile.col != 0)
	{
		if (abs(origin_tile.row - tile.row) == 1 && origin_tile.col - 1 == tile.col &&             // Target is of this case
			!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 1) &&                   // No first wall
			(tile.col == 0 || g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 4)) && // Yes second wall
			!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col - 2))                    // There's a player to skip over

		{
			// Case for rebound down
			if (origin_tile.row != 0 && origin_tile.row - 1 == tile.row && !g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col - 1))
				return true;
			// Case for rebound up
			if (origin_tile.row != width && origin_tile.row + 1 == tile.row && !g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col - 1))
				return true;
		}
	}
	// Moving right over player with obstacle check
	if (origin_tile.col != width)
	{
		if (abs(origin_tile.row - tile.row) == 1 && origin_tile.col + 1 == tile.col &&                 // Target is of this case
			!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 2) &&                       // No first wall
			(tile.col == width || g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 5)) && // Yes second wall
			!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col + 3))                        // There's a player to skip over

		{
			// Case for rebound down
			if (origin_tile.row != 0 && origin_tile.row - 1 == tile.row && !g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col + 3))
				return true;
			// Case for rebound up
			if (origin_tile.row != width && origin_tile.row + 1 == tile.row && !g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col + 3))
				return true;
		}
	}
	// Moving down over player with obstacle check
	if (origin_tile.row != 0)
	{
		if (abs(origin_tile.col - tile.col) == 1 && origin_tile.row - 1 == tile.row &&             // Target is of this case
			!g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col) &&                   // No first wall
			(tile.row == 0 || g_terrain.IsWallClone(bottom_left_tile.row - 4, bottom_left_tile.col)) && // Yes second wall
			!g_terrain.IsBlank(bottom_left_tile.row - 2, bottom_left_tile.col))                    // There's a player to skip over

		{
			// Case for rebound left
			if (origin_tile.col != 0 && origin_tile.col - 1 == tile.col && !g_terrain.IsWallClone(bottom_left_tile.row - 2, bottom_left_tile.col - 1))
				return true;
			// Case for rebound right
			if (origin_tile.col != width && origin_tile.col + 1 == tile.col && !g_terrain.IsWallClone(bottom_left_tile.row - 2, bottom_left_tile.col + 2))
				return true;
		}
	}
	// Moving up over player with obstacle check
	if (origin_tile.row != width)
	{
		if (abs(origin_tile.col - tile.col) == 1 && origin_tile.row + 1 == tile.row &&                 // Target is of this case
			!g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col) &&                       // No first wall
			(tile.row == width || g_terrain.IsWallClone(bottom_left_tile.row + 5, bottom_left_tile.col)) && // Yes second wall
			!g_terrain.IsBlank(bottom_left_tile.row + 3, bottom_left_tile.col))                        // There's a player to skip over

		{
			// Case for rebound left
			if (origin_tile.col != 0 && origin_tile.col - 1 == tile.col && !g_terrain.IsWallClone(bottom_left_tile.row + 3, bottom_left_tile.col - 1))
				return true;
			// Case for rebound right
			if (origin_tile.col != width && origin_tile.col + 1 == tile.col && !g_terrain.IsWallClone(bottom_left_tile.row + 3, bottom_left_tile.col + 2))
				return true;
		}
	}

	return false;
}

bool QuoridorPlayer::IsLegalWall(TileQ origin_tile, TileQ tile)
{
	if (tile.half_row && tile.half_col)
		return false;

	TileQ bottom_left_tile(tile.row * 3, tile.col * 3, tile.half_row, tile.half_col);
	if (tile.half_row)
		bottom_left_tile.row += 2;
	if (tile.half_col)
		bottom_left_tile.col += 2;

	int width = g_terrain.GetWidth() / 3;

	// If it's an horizontal wall
	if (tile.half_row)
	{
		// If not in the end of the board
		if (tile.col != width)
		{
			// If there's room for the wall
			if (!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col) &&
				!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 2) &&
				!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col + 3))
			{
				SetWallClone(tile);
				if (IsLegalWallAfterPathfinding(origin_tile, tile))
				{
					RemoveWallClone(tile);
					return true;
				}
				RemoveWallClone(tile);
			}
		}
		// If in the end of the board
		else
		{
			if (!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col) &&
				!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 1) &&
				!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col - 2))
			{
				SetWallClone(tile);
				if (IsLegalWallAfterPathfinding(origin_tile, tile))
				{
					RemoveWallClone(tile);
					return true;
				}
				RemoveWallClone(tile);
			}
		}
	}
	// If it's an horizontal wall
	if (tile.half_col)
	{
		// If not in the end of the board
		if (tile.row != width)
		{
			// If there's room for the wall
			if (!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col) &&
				!g_terrain.IsWallClone(bottom_left_tile.row + 2, bottom_left_tile.col) &&
				!g_terrain.IsWallClone(bottom_left_tile.row + 3, bottom_left_tile.col))
			{
				SetWallClone(tile);
				if (IsLegalWallAfterPathfinding(origin_tile, tile))
				{
					RemoveWallClone(tile);
					return true;
				}
				RemoveWallClone(tile);
			}
		}
		// If in the end of the board
		else
		{
			if (!g_terrain.IsWallClone(bottom_left_tile.row, bottom_left_tile.col) &&
				!g_terrain.IsWallClone(bottom_left_tile.row - 1, bottom_left_tile.col) &&
				!g_terrain.IsWallClone(bottom_left_tile.row - 2, bottom_left_tile.col))
			{
				SetWallClone(tile);
				if (IsLegalWallAfterPathfinding(origin_tile, tile))
				{
					RemoveWallClone(tile);
					return true;
				}
				RemoveWallClone(tile);
			}
		}
	}
	return false;
}

bool QuoridorPlayer::IsLegalWallAfterPathfinding(TileQ origin_tile, TileQ tile)
{
	TileQ player_tile = g_database.Find("Player")->GetQuoridor().m_tile;
	TileQ npc_tile = g_database.Find("NPC")->GetQuoridor().m_tile;

	int width = g_terrain.GetWidth();

	for (int i = 0; i < width; i += 3)
	{
		if (!PathCheck(player_tile.row*3, player_tile.col*3, width - 1, i) || !PathCheck(npc_tile.row*3, npc_tile.col*3, 0, i))
			return false;
	}
	return true;
}

void QuoridorPlayer::SetTileClone(TileQ tile, bool ontile)
{
	DebugDrawingColor color = DEBUG_COLOR_WHITE;
	if (ontile)
	{
		// If this is the player
		if (m_type)
			color = DEBUG_COLOR_BLUE;
		// If this is the npc
		else
			color = DEBUG_COLOR_RED;
	}

	// Setting 2x2 tile
	g_terrain.SetCloneColor(tile.row * 3, tile.col * 3, color);
	g_terrain.SetCloneColor(tile.row * 3 + 1, tile.col * 3, color);
	g_terrain.SetCloneColor(tile.row * 3, tile.col * 3 + 1, color);
	g_terrain.SetCloneColor(tile.row * 3 + 1, tile.col * 3 + 1, color);
}

void QuoridorPlayer::SetWallClone(const TileQ tile)
{
	DebugDrawingColor color = DEBUG_COLOR_YELLOW;

	int width = g_terrain.GetWidth() / 3;

	// Setting 2x1 wall
	if (tile.half_row)
	{
		int offset = 0;
		if (tile.col == width)
			offset = -3;

		if (g_terrain.GetTerrainTile(tile.row * 3 + 2, tile.col * 3 + offset) == TILE_WALL)
			return;

		Map clone_map = g_terrain.GetCloneMap();

		// Setting tiles
		clone_map.PlaceWall(tile.row * 3 + 2, tile.col * 3 + offset + 0);
		clone_map.PlaceWall(tile.row * 3 + 2, tile.col * 3 + offset + 1);
		clone_map.PlaceWall(tile.row * 3 + 2, tile.col * 3 + offset + 2);
		clone_map.PlaceWall(tile.row * 3 + 2, tile.col * 3 + offset + 3);
		clone_map.PlaceWall(tile.row * 3 + 2, tile.col * 3 + offset + 4);
	}

	// Setting 1x2 wall
	if (tile.half_col)
	{
		int offset = 0;
		if (tile.row == width)
			offset = -3;

		if (g_terrain.GetTerrainTile(tile.row * 3 + offset, tile.col * 3 + 2) == TILE_WALL)
			return;

		Map clone_map = g_terrain.GetCloneMap();

		// Setting tiles
		clone_map.PlaceWall(tile.row * 3 + offset + 0, tile.col * 3 + 2);
		clone_map.PlaceWall(tile.row * 3 + offset + 1, tile.col * 3 + 2);
		clone_map.PlaceWall(tile.row * 3 + offset + 2, tile.col * 3 + 2);
		clone_map.PlaceWall(tile.row * 3 + offset + 3, tile.col * 3 + 2);
		clone_map.PlaceWall(tile.row * 3 + offset + 4, tile.col * 3 + 2);
	}
}

void QuoridorPlayer::RemoveWallClone(const TileQ tile)
{
	DebugDrawingColor color = DEBUG_COLOR_YELLOW;

	int width = g_terrain.GetWidth() / 3;

	// Setting 2x1 wall
	if (tile.half_row)
	{
		int offset = 0;
		if (tile.col == width)
			offset = -3;

		Map clone_map = g_terrain.GetCloneMap();

		// Setting tiles
		clone_map.RemoveWall(tile.row * 3 + 2, tile.col * 3 + offset + 0);
		clone_map.RemoveWall(tile.row * 3 + 2, tile.col * 3 + offset + 1);
		clone_map.RemoveWall(tile.row * 3 + 2, tile.col * 3 + offset + 2);
		clone_map.RemoveWall(tile.row * 3 + 2, tile.col * 3 + offset + 3);
		clone_map.RemoveWall(tile.row * 3 + 2, tile.col * 3 + offset + 4);
	}

	// Setting 1x2 wall
	if (tile.half_col)
	{
		int offset = 0;
		if (tile.row == width)
			offset = -3;

		Map clone_map = g_terrain.GetCloneMap();

		// Setting tiles
		clone_map.RemoveWall(tile.row * 3 + offset + 0, tile.col * 3 + 2);
		clone_map.RemoveWall(tile.row * 3 + offset + 1, tile.col * 3 + 2);
		clone_map.RemoveWall(tile.row * 3 + offset + 2, tile.col * 3 + 2);
		clone_map.RemoveWall(tile.row * 3 + offset + 3, tile.col * 3 + 2);
		clone_map.RemoveWall(tile.row * 3 + offset + 4, tile.col * 3 + 2);
	}
}

void QuoridorPlayer::Update()
{
	SetTile(m_tile, true);

	if (g_terrain.IsEnd())
		return;

	if (m_type && g_terrain.IsPlayerTurn())
	{
		ResetTemp();

		D3DXVECTOR3 temp = g_terrain.GetMousePos();

		TileQ target;
		g_terrain.GetRowColumn(&temp, &target.row, &target.col);

		TileQ tile_target(target.row / 3, target.col / 3);

		//if (g_terrain.IsMousClick())
		//	IsLegalMove(m_tile, tile_target);
				
		g_terrain.CloneMap();

		switch (g_terrain.GetPlayerQuoridorMode())
		{
		case 0: // Move
			if (target.row % 3 != 2 && target.col % 3 != 2)
			{
				if (IsLegalMove(m_tile, tile_target))
				{
					SetTileTemp(tile_target);
					if (g_terrain.IsMousClick())
					{
						SetTile(m_tile, false);
						m_tile = tile_target;
						if (m_tile.row != g_terrain.GetWidth() / 3)
							g_terrain.ChangeTurn();
						else
							g_terrain.SetEnd(false);
					}
				}
			}
			break;
		case 1: // Wall
			if (m_walls && (target.row % 3 == 2 || target.col % 3 == 2))
			{
				if (target.row % 3 == 2)
					tile_target.half_row = true;
				if (target.col % 3 == 2)
					tile_target.half_col = true;

				if (IsLegalWall(m_tile, tile_target))
				{
					SetWallTemp(tile_target);
					if (g_terrain.IsMousClick())
					{
						SetWall(tile_target);
						m_walls--;
						g_terrain.ChangeTurn();
					}
				}
			}
			break;
		case 2: // Stair
			break;
		}
	}
	else if(!m_type && !g_terrain.IsPlayerTurn()) {
		g_database.mAILogic->Execute();
		if (m_tile.row != 0)
			g_terrain.ChangeTurn();
		else
			g_terrain.SetEnd(true);
	}
}

//TileQ QuoridorPlayer::BiasedNextPos(const TileQ& current_tile)
//{
//	int width = g_terrain.GetWidth();
//	int row = 0;
//	if (m_type)
//		row = width - 1;
//	unsigned count = 10000;
//	TileQ finish = current_tile;
//	for (int col = 0; col < width; col += 3)
//	{
//		if (PathCheck(current_tile.row * 3, current_tile.col * 3, row, col))
//		{
//			if (m_waypointList.size() < count)
//			{
//				count = m_waypointList.size();
//				std::list<D3DXVECTOR3>::iterator temp = m_waypointList.begin();
//				TileQ listed;
//				g_terrain.GetRowColumn(&(*temp), &listed.row, &listed.col);
//				listed.row *= 3;
//				listed.col *= 3;
//				while (temp != m_waypointList.end() && listed.row == current_tile.row && listed.col == current_tile.col)
//				{
//					temp++;
//					g_terrain.GetRowColumn(&(*temp), &listed.row, &listed.col);
//					listed.row *= 3;
//					listed.col *= 3;
//				}
//				finish = listed;
//			}
//		}
//	}
//	finish.row /= 3;
//	finish.col /= 3;
//
//	return finish;
//}

bool QuoridorPlayer::PlayerHasShortestPath(const TileQ& player_tile, const TileQ& ai_tile, bool aiTurn)
{
	int width = g_terrain.GetWidth();
	size_t size_player = 100000, size_ai = 100000;
	for (int col = 0; col < width; col += 3)
	{
		if (PathCheck(player_tile.row * 3, player_tile.col * 3, width - 1, col))
		{
			if (m_waypointList.size() < size_player)
				size_player = m_waypointList.size();
		}
		if (PathCheck(ai_tile.row * 3, ai_tile.col * 3, 0, col))
		{
			if (m_waypointList.size() < size_ai)
				size_ai = m_waypointList.size();
		}
	}

	//if same size, winner will be the one that starts moving
	if (size_player == size_ai)
		return !aiTurn;

	return (size_player < size_ai);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool QuoridorPlayer::IsClearPath(int r0, int c0, int r1, int c1)
{
	int max_r = max(r0, r1);
	int min_r = min(r0, r1);
	int max_c = max(c0, c1);
	int min_c = min(c0, c1);

	// Check for if successful Straight Line
	bool straight = true;

	// Loop through the rows
	for (int i = min_r; i <= max_r; i++)
	{
		// Loop through the columns
		for (int j = min_c; j <= max_c; j++)
		{
			// If there's a wall, cannot be straight line
			if (g_terrain.IsWallClone(i, j))
			{
				straight = false;
				break;
			}
		}
		// Breaking when can't be striahgt line
		if (!straight)
			break;
	}

	return straight;
}

float QuoridorPlayer::HeuristicCost(int fr, int fc, int r, int c)
{
	float heuristicWeight = 1.f;

	// Getting the differences between the current and final slots
	int rdiff = abs(fr - r);
	int cdiff = abs(fc - c);

	// Euclidean Heuristic Calculation
	return sqrtf(rdiff * rdiff + cdiff * cdiff) * heuristicWeight;
}

bool QuoridorPlayer::IsDiagonal(int or , int oc, int r, int c)
{
	// If they either share a row or column, they aren't diagonal
	if (or == r || oc == c)
		return false;
	return true;
}

float QuoridorPlayer::AdjacentChildCost(int or , int oc, int r, int c)
{
	// If child number is odd, its a diagonal adjacent child
	if (IsDiagonal(or , oc, r, c))
		return sqrtf(2.f);
	else
		return 1;
}

void QuoridorPlayer::IdToCoords(int id, int width, int* r, int* c)
{
	// Getting the coords from the id of a node
	*r = id % width;
	*c = id / width;
}

int QuoridorPlayer::GetCheapestNode(int* parentR, int* parentC)
{
	float min = INFINITY;
	int id = -1;
	size_t index = -1;
	// Go through the list
	for (size_t i = 0; i < m_open_list.size(); i++)
	{
		int temp = m_open_list[i];
		// If this node is the cheapest, store it
		if (m_nodes[temp].total < min)
		{
			id = temp;
			min = m_nodes[temp].total;
			index = i;
		}
	}
	// Getting the coords of the node
	IdToCoords(id, m_width, parentR, parentC);

	// Return cheapest node by index
	return index;
}

void QuoridorPlayer::AFillWaypoint(int r, int c)
{
	a_node_data node = m_nodes[r + c * m_width];
	// If reached the first node, don't add it, it's already there
	if (node.parentRow == -1 && node.parentCol == -1)
		return;
	// Add the parent node to the waypoint
	AFillWaypoint(node.parentRow, node.parentCol);
	// Add the node to the waypoint
	m_waypointList.push_back(g_terrain.GetCoordinates(r, c));
}

bool QuoridorPlayer::WallCheck(int or , int oc, int r, int c)
{
	// If it's a wall already, return true
	if (g_terrain.IsWallClone(r, c))
		return true;
	// If both to the sides have walls, this also has wall
	if (g_terrain.IsWallClone(r - 1, c) && g_terrain.IsWallClone(r + 1, c))
		return true;
	if (g_terrain.IsWallClone(r, c - 1) && g_terrain.IsWallClone(r, c + 1))
		return true;
	// If it's a diagonal
	if (IsDiagonal(or , oc, r, c))
	{
		return true;
		// If the diagonal is on the right
		if (or < r)
		{
			// If the right from the parent also a wall, can't make that diagonal
			if (g_terrain.IsWallClone(or +1, oc))
				return true;
		}
		// If the diagonal is on the left
		if (or > r)
		{
			// If the left from the parent also a wall, can't make that diagonal
			if (g_terrain.IsWallClone(or -1, oc))
				return true;
		}
		// If the diagonal is up
		if (oc < c)
		{
			// If the up from the parent also a wall, can't make that diagonal
			if (g_terrain.IsWallClone(or , oc + 1))
				return true;
		}
		// If the diagonal is down
		if (oc > c)
		{
			// If the down from the parent also a wall, can't make that diagonal
			if (g_terrain.IsWallClone(or , oc - 1))
				return true;
		}
	}
	return false;
}

bool QuoridorPlayer::WallCheck2(int or , int oc, int r, int c)
{
	// If it's a wall already, return true
	if (g_terrain.IsWallClone(or , oc))
		return true;
	// If it's a diagonal
	if (IsDiagonal(or , oc, r, c))
	{
		// If the diagonal is on the right top
		if (or < r && oc < c)
		{
			// If the right from the parent also a wall, can't make that diagonal
			if (g_terrain.IsWallClone(or +1, oc) && g_terrain.IsWallClone(or , oc + 1))
				return true;
		}
		// If the diagonal is on the left top
		if (or > r && oc < c)
		{
			// If the left from the parent also a wall, can't make that diagonal
			if (g_terrain.IsWallClone(or -1, oc) && g_terrain.IsWallClone(or , oc + 1))
				return true;
		}
		// If the diagonal is on the right down
		if (or < r && oc > c)
		{
			// If the right from the parent also a wall, can't make that diagonal
			if (g_terrain.IsWallClone(or +1, oc) && g_terrain.IsWallClone(or , oc - 1))
				return true;
		}
		// If the diagonal is on the left down
		if (or > r && oc > c)
		{
			// If the left from the parent also a wall, can't make that diagonal
			if (g_terrain.IsWallClone(or -1, oc) && g_terrain.IsWallClone(or , oc - 1))
				return true;
		}
	}
	return false;
}

bool QuoridorPlayer::PathCheck(int r0, int c0, int r, int c)
{


	// Getting values of the current location of Tiny
	int curR = r0, curC = c0;
	D3DXVECTOR3 cur = g_terrain.GetCoordinates(r0, c0);


	m_width = g_terrain.GetWidth();
	//int rdiff = curR - r, cdiff = curC - c;

	
	// Clearing the waypoint list and adding the current node as the first node
	m_waypointList.clear();
	m_waypointList.push_back(cur);

	// Straight Line Optimization
	{
		// If it can be a straight line
		if (IsClearPath(curR, curC, r, c))
		{
			// Adding the last node
			m_waypointList.push_back(g_terrain.GetCoordinates(r, c));

			return true;
		}
	}

	/// A* algorithm beginning
	// First, empty the lists
	m_nodes.clear();
	m_nodes = std::vector<a_node_data>(m_width * m_width);
	m_open_list.clear();
	m_closed_list.clear();

	// Pushing Start Node to Open List
	a_node_data& start = m_nodes[curR + curC * m_width];
	start.cost = 0;
	start.total = HeuristicCost(r, c, curR, curC);
	start.onOpen = true;
	m_open_list.push_back(curR + curC * m_width); // Pushing the id of the node

	//while (1)
	{
		/// A* algorithm
		// While Open List isn't empty
		while (!m_open_list.empty())
		{
			// Pop cheapest node from Open List to act as parent
			int parentR, parentC;
			// Here we get the id on the open list of the cheapest, and then offset an iterator to that node
			m_open_list.erase(m_open_list.begin() + GetCheapestNode(&parentR, &parentC));

			// TOCOMPLETE If node is the goal, return GOAL(?)
			if (parentR == r && parentC == c)
			{
				m_closed_list.push_back(parentR + parentC * m_width);

				AFillWaypoint(parentR, parentC);

				break;
			}

			a_node_data& parent_node = m_nodes[parentR + parentC * m_width];

			// For all neighbouring nodes
			int count_children = 0;
			for (int i = max(parentR - 1, 0); i <= min(parentR + 1, m_width - 1); i++)
			{
				for (int j = max(parentC - 1, 0); j <= min(parentC + 1, m_width - 1); j++)
				{
					int id = i + j * m_width;

					count_children++;
					// Skip parent
					if (i == parentR && j == parentC)
						continue;
					// Skip if wall
					if (WallCheck(parentR, parentC, i, j))
						continue;

					a_node_data& child_node = m_nodes[id];

					// Computing the cost
					float cost = parent_node.cost + AdjacentChildCost(parentR, parentC, i, j);
					float total_cost = cost + HeuristicCost(r, c, i, j);

					// If child node isn't on open or closed list, put it on open list
					if (!child_node.onOpen && !child_node.onClosed)
					{
						child_node.onOpen = true;

						child_node.parentRow = parentR;
						child_node.parentCol = parentC;

						child_node.cost = cost;
						child_node.total = total_cost;

						m_open_list.push_back(id);
					}
					// If on open/closed list and cost is cheaper, remove previous and add new one
					else if (total_cost < child_node.total)
					{
						child_node.parentRow = parentR;
						child_node.parentCol = parentC;

						child_node.cost = cost;
						child_node.total = total_cost;

						child_node.onOpen = true;

						// If this was on the closed list
						if (child_node.onClosed)
						{
							// Remove it from there
							child_node.onClosed = false;

							// Remove the node from the closed list from its id
							for (size_t l = 0; l < m_closed_list.size(); l++)
							{
								if (m_closed_list[l] == (id))
								{
									m_closed_list.erase(m_closed_list.begin() + l);
									break;
								}
							}

							// Add it to the open list
							m_open_list.push_back(id);
						}
					}
				}
			}
			// Placing parent node on the closed list
			m_closed_list.push_back(parentR + parentC * m_width);

			if (m_open_list.empty())
				return false;
		}

		// If reached this point, path hasn't been found
		return true;
	}
}

