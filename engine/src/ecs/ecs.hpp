#pragma once
#include "../engineCore/core.h"
#include <vector>
#include <array>
#include <memory>

#include <iostream>

class TypeIndex // internal use only
{
	inline static unsigned int s_TypeCounter = 0;

public:
	template<typename T>
	static unsigned int getIndex()
	{
		static bool s_IsTracked = false;
		static unsigned int s_TypeIndexValue;

		if (!s_IsTracked)
		{
			s_TypeIndexValue = s_TypeCounter++;
			s_IsTracked = true;
		}
		return s_TypeIndexValue;
	}
};

using EntityId = uint32_t;
constexpr unsigned int pageSize = 2;
constexpr unsigned int EMPTY_INDEX = ~0u; // = bits 11111...


struct SparsePage
{
	std::array<unsigned int, pageSize> m_IndexMap;
	SparsePage()
	{
		m_IndexMap.fill(EMPTY_INDEX);
	}
};

template <typename T>
struct SparseSet
{
	std::vector<std::unique_ptr<SparsePage>> m_Pages;
	std::vector<T> m_Dense;
	std::vector<EntityId> m_DenseCallback;

	T* getElement(EntityId id)
	{
		unsigned int pageIndex = id / pageSize;
		unsigned int itemIndex = id % pageSize;
		if (m_Pages[pageIndex]) return &(m_Dense[m_Pages[pageIndex]->m_IndexMap[itemIndex]]);
		return nullptr;
	}

	uint32_t& getIndex(EntityId id)
	{
		unsigned int pageIndex = id / pageSize;
		unsigned int itemIndex = id % pageSize;
		if (m_Pages[pageIndex]) return m_Pages[pageIndex]->m_IndexMap[itemIndex];
	}

	void add(EntityId id, T value)
	{
		unsigned int pageIndex = id / pageSize;
		unsigned int itemIndex = id % pageSize;

		if (pageIndex >= m_Pages.size())
		{
			m_Pages.resize(pageIndex + 1);
		}

		if (!m_Pages[pageIndex])
		{
			m_Pages[pageIndex] = std::make_unique<SparsePage>();
		}

		if (m_Pages[pageIndex]->m_IndexMap[itemIndex] == EMPTY_INDEX)
		{
			unsigned int denseIndex = static_cast<unsigned int>(m_Dense.size());
			m_Dense.emplace_back(value);
			m_DenseCallback.emplace_back(id);
			m_Pages[pageIndex]->m_IndexMap[itemIndex] = denseIndex;
		}
	}

	void remove(EntityId id)
	{
		unsigned int pageIndex = id / pageSize;
		unsigned int itemIndex = id % pageSize;

		if (pageIndex >= m_Pages.size() || !m_Pages[pageIndex]) return;
		unsigned int sparseIndex = m_Pages[pageIndex]->m_IndexMap[itemIndex];
		if (sparseIndex == EMPTY_INDEX) return;

		unsigned int lastIndex = static_cast<unsigned int>(m_Dense.size() - 1);
		if (sparseIndex != lastIndex)
		{
			m_Dense[sparseIndex] = m_Dense[lastIndex];
			EntityId movedId = m_DenseCallback[lastIndex];
			m_DenseCallback[sparseIndex] = movedId;

			unsigned int movedPage = movedId / pageSize;
			unsigned int movedItem = movedId % pageSize;
			m_Pages[movedPage]->m_IndexMap[movedItem] = sparseIndex;
		}

		m_Pages[pageIndex]->m_IndexMap[itemIndex] = EMPTY_INDEX;
		m_Dense.pop_back();
		m_DenseCallback.pop_back();
	}
};


class CUSTOMENGINE_API ECS
{
	friend class Entity;

	typedef void(*Deleter)(ECS*, unsigned int);
	typedef void(*Remover)(ECS*, EntityId);

	unsigned int m_EntityCounter = 0;
	std::vector<void*> m_SparseSets;
	std::vector<Deleter> m_Deleter;
	std::vector<Remover> m_Remover;

	template <typename T>
	void addTypeStorage()
	{
		unsigned int typeIndex = TypeIndex::getIndex<T>();
		if (typeIndex >= m_SparseSets.size())
		{
			m_SparseSets.resize(typeIndex + 1, nullptr);
			m_Deleter.resize(typeIndex + 1, nullptr);
			m_Remover.resize(typeIndex + 1, nullptr);
		}
		if (!m_SparseSets[typeIndex])
		{
			m_SparseSets[typeIndex] = new SparseSet<T>();
			m_Deleter[typeIndex] = [](ECS* ecs, unsigned int typeIndex)
				{
					delete static_cast<SparseSet<T>*>(ecs->m_SparseSets[typeIndex]);
					ecs->m_SparseSets[typeIndex] = nullptr;
				};
			m_Remover[typeIndex] = [](ECS* ecs, EntityId id)
				{
					ecs->removeItem<T>(id);
				};
		}
	}

	template <typename T>
	void removeTypeStorage()
	{
		unsigned int typeIndex = TypeIndex::getIndex<T>();
		m_Deleter[typeIndex](this, typeIndex);
		m_Deleter[typeIndex] = nullptr;
	}

	template <typename T>
	void addItem(EntityId id, T item)
	{
		unsigned int typeIndex = TypeIndex::getIndex<T>();
		addTypeStorage<T>();
		static_cast<SparseSet<T>*>(m_SparseSets[TypeIndex::getIndex<T>()])->add(id, item);
	}

	template <typename T>
	void removeItem(EntityId id)
	{
		static_cast<SparseSet<T>*>(m_SparseSets[TypeIndex::getIndex<T>()])->remove(id);
	}

	template <typename T>
	T* selectItem(EntityId id)
	{
		unsigned int typeIndex = TypeIndex::getIndex<T>();
		if (m_SparseSets[typeIndex]) return static_cast<SparseSet<T>*>(m_SparseSets[typeIndex])->getElement(id);
		return nullptr;
	}

	template<typename T>
	uint32_t& getComponentIndexOfEntity(EntityId id)
	{
		unsigned int typeIndex = TypeIndex::getIndex<T>();
		return static_cast<SparseSet<T>*>(m_SparseSets[typeIndex])->getIndex(id);
	}

	void removeAllComponents(EntityId id)
	{
		for (Remover& remover : m_Remover) remover(this, id);
	}

public:
	~ECS()
	{
		for (int i{}; i < m_Deleter.size(); i++)
		{
			if (m_Deleter[i]) m_Deleter[i](this, i);
		}
	}

	template<typename T>
	std::vector<T>* getComponentStorage()
	{
		unsigned int typeIndex = TypeIndex::getIndex<T>();
		addTypeStorage<T>();
		if (m_SparseSets[typeIndex]) return &(static_cast<SparseSet<T>*>(m_SparseSets[typeIndex])->m_Dense);
		return nullptr;
	}

	EntityId getEntityIndex()
	{
		return m_EntityCounter++;
	}
};

class CUSTOMENGINE_API Entity
{
	friend class ECS;

	EntityId m_Id = ~0u;
	ECS* m_ecs = nullptr;

public:

	Entity(ECS& ecs)
	{
		m_ecs = &ecs;
		m_Id = m_ecs->getEntityIndex();
	}

	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	Entity(Entity&& other) noexcept
	{
		m_ecs = other.m_ecs;
		m_Id = other.m_Id;

		other.m_ecs = nullptr;
		other.m_Id = ~0u;
	}

	Entity& operator=(Entity&& other) noexcept
	{
		if (this == &other) return *this;

		if (m_ecs) destroy();

		m_ecs = other.m_ecs;
		m_Id = other.m_Id;

		other.m_ecs = nullptr;
		other.m_Id = ~0u;

		return *this;
	}

	~Entity()
	{
		if (m_ecs) destroy();
	}

	void destroy()
	{
		m_ecs->removeAllComponents(m_Id);
	}

	template<typename T>
	void addComponent(const T& component)
	{
		m_ecs->addItem(m_Id, component);
	}

	template<typename T>
	void removeComponent()
	{
		m_ecs->removeItem<T>(m_Id);
	}

	template<typename T>
	T& selectComponent()
	{
		return *(m_ecs->selectItem<T>(m_Id));
	}

	template<typename T>
	uint32_t& getComponentIndex()
	{
		return m_ecs->getComponentIndexOfEntity<T>(m_Id);
	}
};