#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>

struct InventoryItem
{
	std::string id;
	int count = 0;
};

struct Inventory
{
	size_t capacity = 32;

	bool isFull() const
	{
		return items.size() >= capacity;
	}

	size_t size() const
	{
		return items.size();
	}

	int getCount(const std::string& id) const
	{
		for (const auto& item : items)
		{
			if (item.id == id) return item.count;
		}
		return 0;
	}

	bool contains(const std::string& id) const
	{
		return getCount(id) > 0;
	}

	bool addItem(const std::string& id, int count = 1)
	{
		if (count <= 0) return false;

		for (auto& item : items)
		{
			if (item.id == id)
			{
				item.count += count;
				return true;
			}
		}

		if (isFull()) return false;

		items.push_back({ id, count });
		return true;
	}

	bool removeItem(const std::string& id, int count = 1)
	{
		if (count <= 0) return false;

		for (auto it = items.begin(); it != items.end(); ++it)
		{
			if (it->id == id)
			{
				if (it->count < count) return false;

				it->count -= count;
				if (it->count == 0)
				{
					items.erase(it);
				}
				return true;
			}
		}

		return false;
	}

	const std::vector<InventoryItem>& getItems() const
	{
		return items;
	}

	void clear()
	{
		items.clear();
	}

private:
	std::vector<InventoryItem> items;
};