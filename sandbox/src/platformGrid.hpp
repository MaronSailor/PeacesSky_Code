#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

enum class BlockType : uint8_t
{
	Air = 0,
	Stone = 1,
	Brick1 = 2,
	Brick2 = 3,
	Metal1 = 4,
	Metal2 = 5,
	TeamA = 6,
	TeamB = 7
};

struct PlatformGrid
{
	int width = 0;
	int height = 0;
	int depth = 0;
	std::vector<BlockType> blocks;

	void resize(int newWidth, int newHeight, int newDepth)
	{
		width = std::max(0, newWidth);
		height = std::max(0, newHeight);
		depth = std::max(0, newDepth);
		blocks.assign(static_cast<size_t>(width * height * depth), BlockType::Air);
	}

	void resizePreserve(int newWidth, int newHeight, int newDepth, int offsetX, int offsetY, int offsetZ)
	{
		newWidth = std::max(0, newWidth);
		newHeight = std::max(0, newHeight);
		newDepth = std::max(0, newDepth);

		std::vector<BlockType> newBlocks(static_cast<size_t>(newWidth * newHeight * newDepth), BlockType::Air);

		auto newIndex = [&](int x, int y, int z)
			{
				return static_cast<size_t>((y * newDepth + z) * newWidth + x);
			};

		for (int y = 0; y < height; ++y)
		{
			for (int z = 0; z < depth; ++z)
			{
				for (int x = 0; x < width; ++x)
				{
					const int nx = x + offsetX;
					const int ny = y + offsetY;
					const int nz = z + offsetZ;

					if (nx < 0 || nx >= newWidth || ny < 0 || ny >= newHeight || nz < 0 || nz >= newDepth)
					{
						continue;
					}

					newBlocks[newIndex(nx, ny, nz)] = blocks[index(x, y, z)];
				}
			}
		}

		width = newWidth;
		height = newHeight;
		depth = newDepth;
		blocks = std::move(newBlocks);
	}

	bool inBounds(int x, int y, int z) const
	{
		return x >= 0 && x < width
			&& y >= 0 && y < height
			&& z >= 0 && z < depth;
	}

	BlockType get(int x, int y, int z) const
	{
		return blocks[index(x, y, z)];
	}

	void set(int x, int y, int z, BlockType type)
	{
		blocks[index(x, y, z)] = type;
	}

private:
	size_t index(int x, int y, int z) const
	{
		return static_cast<size_t>((y * depth + z) * width + x);
	}
};