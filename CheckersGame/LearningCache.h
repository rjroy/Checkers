#pragma once

#include <queue>
#include <hash_map>

//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
class CLearningCache
{
public:
	typedef std::pair<TKey, TValue> TDataPair;

	CLearningCache(unsigned int cacheSize) : m_cacheSize(cacheSize), m_head(NULL), m_tail(NULL) {}
	virtual ~CLearningCache();

	void Clear();

	TValue* Get( const TKey& key );
	void UpdateCache( const TKey& key, const TValue& newValue );

private:
	struct SNode
	{
		TDataPair m_data;
		SNode* m_next;
		SNode* m_prev;

		SNode( const TDataPair& data ) : m_data(data), m_next(NULL), m_prev(NULL) { }
	};
	typedef std::hash_map<TKey, SNode*> TNodeHash;

	SNode* m_head;
	SNode* m_tail;
	TNodeHash m_hash;
	const unsigned int m_cacheSize;

	void MoveToTail( SNode* node );
	void AddToTail( SNode* node );
	bool RemoveNode( SNode* node );
	void RemoveHeadAndHashEntry();
};

//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
CLearningCache<TKey,TValue>::~CLearningCache()
{
	Clear();
}

//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
void CLearningCache<TKey,TValue>::Clear()
{
	SNode* curr = m_head;
	while( curr )
	{
		SNode* next = curr->m_next;
		delete curr;
		curr = next;
	}
	m_head = NULL;
	m_tail = NULL;
	m_hash.clear();
}

//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
TValue* CLearningCache<TKey,TValue>::Get( const TKey& key )
{
	TNodeHash::iterator hashItr = m_hash.find( key );
	if( hashItr == m_hash.end() )
		return NULL;

	MoveToTail( hashItr->second );
	return &( hashItr->second->m_data.second );
}

//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
void CLearningCache<TKey,TValue>::UpdateCache( const TKey& key, const TValue& newValue )
{
	TValue* existingEntry = Get(key);
	if( existingEntry )
	{
		*existingEntry = newValue;
	}
	else
	{
		SNode* node = new SNode( TDataPair( key, newValue ) );
		AddToTail( node );
		m_hash[ key ] = node;
		while( m_hash.size() > m_cacheSize )
			RemoveHeadAndHashEntry();
	}
}
//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
bool CLearningCache<TKey,TValue>::RemoveNode( SNode* node )
{
	if( !node || !m_tail || !m_head )
		return false;

	if( node == m_head )
		m_head = node->m_next;

	if( node == m_tail )
		m_tail = node->m_prev;

	if( node->m_prev )
		node->m_prev->m_next = node->m_next;
	if( node->m_next )
		node->m_next->m_prev = node->m_prev;

	return true;
}

//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
void CLearningCache<TKey,TValue>::MoveToTail( SNode* node )
{
	if( node == m_tail )
		return;

	if( !RemoveNode( node ) )
		return;

	AddToTail( node );
}

//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
void CLearningCache<TKey,TValue>::AddToTail( SNode* node )
{
	if( !node )
		return;

	node->m_next = NULL;
	node->m_prev = m_tail;
	if( m_tail )
		m_tail->m_next = node;
	m_tail = node;
	if( !m_head )
		m_head = m_tail;
}

//--------------------------------------------------------------------------------------
template <typename TKey, typename TValue>
void CLearningCache<TKey,TValue>::RemoveHeadAndHashEntry()
{
	SNode* toDelete = m_head;
	if( !RemoveNode( toDelete ) )
		return;

	m_hash.erase( toDelete->m_data.first );
	delete toDelete;
}
