
#include <Stdafx.h>

QuoridorPlayer::QuoridorPlayer(bool type, TileQ tile) : m_type(type), m_walls(10), m_stairs(1)
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

void QuoridorPlayer::ResetTemp()
{
	for (int r = 0; r < g_terrain.GetWidth(); r++)
	{
		for (int c = 0; c < g_terrain.GetWidth(); c++)
		{
			if (g_terrain.GetInfluenceMapValue(r, c) == 1.f)
			{
				g_terrain.SetInfluenceMapValue(r, c, 0.f);
				g_terrain.SetColor(r, c, DEBUG_COLOR_WHITE);
			}
		}
	}
}

bool QuoridorPlayer::IsLegalMove(TileQ tile)
{
	TileQ bottom_left_tile(tile.row * 3, tile.col * 3);

	int width = g_terrain.GetWidth() / 3;

	// Moving left check
	if (m_tile.col != 0)
	{
		if (m_tile.row == tile.row && m_tile.col-1 == tile.col &&
			!g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col+2) &&
			g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col) )
			return true;
	}
	// Moving right check
	if (m_tile.col != width)
	{
		if (m_tile.row == tile.row && m_tile.col+1 == tile.col &&
			!g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col-1) &&
			g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col))
			return true;
	}
	// Moving down check
	if (m_tile.row != 0)
	{
		if (m_tile.row-1 == tile.row && m_tile.col == tile.col &&
			!g_terrain.IsWall(bottom_left_tile.row+2, bottom_left_tile.col) &&
			g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col))
			return true;
	}
	// Moving up check
	if (m_tile.row != width)
	{
		if (m_tile.row+1 == tile.row && m_tile.col == tile.col &&
			!g_terrain.IsWall(bottom_left_tile.row-1, bottom_left_tile.col) &&
			g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col))
			return true;
	}



	// Moving left over player without obstacle check
	if (m_tile.col > 1)
	{
		if (m_tile.row == tile.row && m_tile.col-2 == tile.col &&              // Target is this case
			!g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col+5) && // No first wall
			!g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col+2) && // No second wall
			!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col+3))  // There's a player to skip over
			return true;
	}
	// Moving right over player without obstacle check
	if (m_tile.col < (width-1))
	{
		if (m_tile.row == tile.row && m_tile.col+2 == tile.col &&              // Target is this case
			!g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col-4) && // No first wall
			!g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col-1) && // No second wall
			!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col-2))  // There's a player to skip over
			return true;
	}
	// Moving down over player without obstacle check
	if (m_tile.row > 1)
	{
		if (m_tile.row-2 == tile.row && m_tile.col == tile.col &&              // Target is this case
			!g_terrain.IsWall(bottom_left_tile.row+5, bottom_left_tile.col) && // No first wall
			!g_terrain.IsWall(bottom_left_tile.row+2, bottom_left_tile.col) && // No second wall
			!g_terrain.IsBlank(bottom_left_tile.row+3, bottom_left_tile.col))  // There's a player to skip over
			return true;
	}
	// Moving up over player without obstacle check
	if (m_tile.row < (width-1))
	{
		if (m_tile.row+2 == tile.row && m_tile.col == tile.col &&              // Target is this case
			!g_terrain.IsWall(bottom_left_tile.row-4, bottom_left_tile.col) && // No first wall
			!g_terrain.IsWall(bottom_left_tile.row-1, bottom_left_tile.col) && // No second wall
			!g_terrain.IsBlank(bottom_left_tile.row-2, bottom_left_tile.col))  // There's a player to skip over
			return true;
	}

	bottom_left_tile.row = m_tile.row * 3;
	bottom_left_tile.col = m_tile.col * 3;

	// Moving left over player with obstacle check
	if (m_tile.col != 0)
	{
		if (abs(m_tile.row - tile.row) == 1 && m_tile.col - 1 == tile.col &&                       // Target is of this case
			!g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col - 1) &&                   // No first wall
			(tile.col == 0 || g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col - 4)) && // Yes second wall
			!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col - 2))                    // There's a player to skip over

		{
			// Case for rebound down
			if (m_tile.row != 0 && m_tile.row - 1 == tile.row && !g_terrain.IsWall(bottom_left_tile.row - 1, bottom_left_tile.col - 1))
				return true;
			// Case for rebound up
			if (m_tile.row != width && m_tile.row + 1 == tile.row && !g_terrain.IsWall(bottom_left_tile.row + 2, bottom_left_tile.col - 1))
				return true;
		}
	}
	// Moving right over player with obstacle check
	if (m_tile.col != width)
	{
		if (abs(m_tile.row - tile.row) == 1 && m_tile.col + 1 == tile.col &&                           // Target is of this case
			!g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col + 2) &&                       // No first wall
			(tile.col == width || g_terrain.IsWall(bottom_left_tile.row, bottom_left_tile.col + 5)) && // Yes second wall
			!g_terrain.IsBlank(bottom_left_tile.row, bottom_left_tile.col + 3))                        // There's a player to skip over

		{
			// Case for rebound down
			if (m_tile.row != 0 && m_tile.row - 1 == tile.row && !g_terrain.IsWall(bottom_left_tile.row - 1, bottom_left_tile.col + 2))
				return true;
			// Case for rebound up
			if (m_tile.row != width && m_tile.row + 1 == tile.row && !g_terrain.IsWall(bottom_left_tile.row + 2, bottom_left_tile.col + 2))
				return true;
		}
	}
	// Moving down over player with obstacle check
	if (m_tile.row != 0)
	{
		if (abs(m_tile.col - tile.col) == 1 && m_tile.row - 1 == tile.row &&                       // Target is of this case
			!g_terrain.IsWall(bottom_left_tile.row - 1, bottom_left_tile.col) &&                   // No first wall
			(tile.row == 0 || g_terrain.IsWall(bottom_left_tile.row - 4, bottom_left_tile.col)) && // Yes second wall
			!g_terrain.IsBlank(bottom_left_tile.row - 2, bottom_left_tile.col))                    // There's a player to skip over

		{
			// Case for rebound left
			if (m_tile.col != 0 && m_tile.col - 1 == tile.col && !g_terrain.IsWall(bottom_left_tile.row - 1, bottom_left_tile.col - 1))
				return true;
			// Case for rebound right
			if (m_tile.col != width && m_tile.col + 1 == tile.col && !g_terrain.IsWall(bottom_left_tile.row - 1, bottom_left_tile.col + 2))
				return true;
		}
	}
	// Moving up over player with obstacle check
	if (m_tile.row != width)
	{
		if (abs(m_tile.col - tile.col) == 1 && m_tile.row + 1 == tile.row &&                           // Target is of this case
			!g_terrain.IsWall(bottom_left_tile.row + 2, bottom_left_tile.col) &&                       // No first wall
			(tile.row == width || g_terrain.IsWall(bottom_left_tile.row + 5, bottom_left_tile.col)) && // Yes second wall
			!g_terrain.IsBlank(bottom_left_tile.row + 3, bottom_left_tile.col))                        // There's a player to skip over

		{
			// Case for rebound left
			if (m_tile.col != 0 && m_tile.col - 1 == tile.col && !g_terrain.IsWall(bottom_left_tile.row + 2, bottom_left_tile.col - 1))
				return true;
			// Case for rebound right
			if (m_tile.col != width && m_tile.col + 1 == tile.col && !g_terrain.IsWall(bottom_left_tile.row + 2, bottom_left_tile.col + 3))
				return true;
		}
	}

	return false;
}

void QuoridorPlayer::Update()
{
	SetTile(m_tile, true);

	if (m_type && g_terrain.IsPlayerTurn())
	{
		ResetTemp();

		D3DXVECTOR3 temp = g_terrain.GetMousePos();

		TileQ target;
		g_terrain.GetRowColumn(&temp, &target.row, &target.col);

		TileQ tile_target(target.row / 3, target.col / 3);

		if (g_terrain.IsMousClick())
			IsLegalMove(tile_target);

		switch (g_terrain.GetPlayerQuoridorMode())
		{
		case 0: // Move
			if (target.row % 3 != 2 && target.col % 3 != 2)
			{
				if (IsLegalMove(tile_target))
				{
					SetTileTemp(tile_target);
					if (g_terrain.IsMousClick())
					{
						SetTile(m_tile, false);
						m_tile = tile_target;
						g_terrain.ChangeTurn();
					}
				}
			}
			break;
		case 1: // Wall
			break;
		case 2: // Stair
			break;
		}
	}
	else if(!m_type && !g_terrain.IsPlayerTurn()) {
		g_database.mAILogic->Execute();
	}
}
