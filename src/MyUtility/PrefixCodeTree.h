//-------------------------------------------------------------
//! @brief	接頭符号ツリー
//! @author	ｹｰﾄｩｽ=ｶｴﾚｽﾃｨｽ
//-------------------------------------------------------------
#pragma once

//-------------------------------------------------------------
// include
//-------------------------------------------------------------
#include <vector>
#include <bitset>
#include <array>

namespace MyUtility
{
namespace PrefixC
{
//-------------------------------------------------------------
// constant
//-------------------------------------------------------------
using index_t		  = int;
using ChildIndexArray = std::array<index_t, 2>;

static constexpr index_t invalidIndex = -1;

//-------------------------------------------------------------
// class (接頭符号ツリー)
//-------------------------------------------------------------
template<typename T>
class BasicPrefixCTree
{
public:

	//! ノードクラス
	class Node;

	//! ツリーをたどるクラス
	class TreeWalker;	

	//! ランダムアクセス
	template<size_t numBit>
	Node& operator[] (std::bitset<numBit> key);

	//! 符号とそれに対応する値を登録する
	bool Entry(unsigned bitset, T value, size_t numRead);

	template<size_t numBit>
	bool Entry(std::bitset<numBit> bitset, T value, size_t numRead = numBit);

	//! コンストラクタ
	BasicPrefixCTree()
	{
		//! 先頭ノードの作成
		m_nodeList.push_back(Node{});
		m_nodeTop = 0;
	}

private:

	std::vector<Node>		m_nodeList;
	index_t					m_nodeTop = invalidIndex;

	template<size_t numBit>
	std::pair<Node&, bool> CreateNodeIfNotFound(std::bitset<numBit> key, size_t readBit);
};

//-------------------------------------------------------------
// class (接頭符号ツリーのノード)
//-------------------------------------------------------------
template<typename T>
class BasicPrefixCTree<T>::Node
{
	friend class BasicPrefixCTree<T>;

public:
	//! メンバアクセス
	T    GetValue() const noexcept { return m_value; }
	void SetValue(T val) noexcept;

	//! このノードが値を持っているか
	bool HasValue() const noexcept { return m_hasvalue; }

	//! 子へのIndexを返す
	index_t ChildIndex(int bit) const;

private:

	T				m_value = T{};
	bool			m_hasvalue = false;
	ChildIndexArray	m_childIndex{ invalidIndex ,invalidIndex };
};

//-------------------------------------------------------------
// helpler class (ツリーをたどるクラス)
//-------------------------------------------------------------
template<typename T>
class BasicPrefixCTree<T>::TreeWalker
{
	friend class BasicPrefixCTree<T>;

public:

	//! 現在指しているノードを返す
	const typename BasicPrefixCTree<T>::Node& Get() const;

	//! 現在指しているノードへアクセス
	const typename BasicPrefixCTree<T>::Node* operator->() const { return &Get(); }

	//! 指定した分岐先に子が存在するか
	bool HasChild(int bit) const;

	//! 移動
	bool Next(int nextBit);

	//! コンストラクタ
	TreeWalker(const BasicPrefixCTree& tree)
		:m_tree(tree)
		, m_current(tree.m_nodeTop)
	{}

private:

	const BasicPrefixCTree&	m_tree;
	index_t					m_current;


	index_t GetNextIndex(int bit) const;
};

//-------------------------------------------------------------
// interface (ビットストリームインターフェース)
//-------------------------------------------------------------
class IbitStream
{
public:
	//! 1ビットロードする
	virtual IbitStream& operator >> (int& out) = 0;

	//! 終端か
	virtual bool Eof() const = 0;

	virtual ~IbitStream() {};
};

//-------------------------------------------------------------
// helpler function
//-------------------------------------------------------------

// @brief ビット列を１単位デコードする
//-------------------------------------------------------------
template<typename T>
inline bool Decode(IbitStream& stream, const BasicPrefixCTree<T>& tree, T* out)
{
	auto walker = BasicPrefixCTree<T>::TreeWalker(tree);
	while(!stream.Eof())
	{
		int bit;
		stream >> bit;

		// 対応する符号が見つからない
		if (walker.HasChild(bit) == false) break;

		walker.Next(bit);

		// 見つかった
		if (walker->HasValue())
		{
			*out = walker->GetValue();
			return true;		
		}
	}
	return false;
}

//-------------------------------------------------------------
// alias
//-------------------------------------------------------------
using PrefixCodeTree = BasicPrefixCTree<unsigned>;

//-------------------------------------------------------------
// implement
//-------------------------------------------------------------

// @brief 値を設定
//-------------------------------------------------------------
template<typename T>
inline void BasicPrefixCTree<T>::Node::SetValue(T val) noexcept
{
	m_value = val;
	m_hasvalue = true;
}

// @brief 子へのIndexを返す
// @note  値はBasicPrefixCTreeの持つvectorへのIndexを指す
//-------------------------------------------------------------
template<typename T>
inline index_t BasicPrefixCTree<T>::Node::ChildIndex(int bit) const
{
	return m_childIndex.at(bit);
}

// @brief ランダムアクセス風
// @note  ランダムアクセスではない
//-------------------------------------------------------------
template<typename T>
template<size_t numBit>
typename BasicPrefixCTree<T>::Node& BasicPrefixCTree<T>::operator[] (std::bitset<numBit> key)
{
	return CreateNodeIfNotFound(key).first;
}

// @brief	符号とそれに対応する値を登録する
//-------------------------------------------------------------
template<typename T>
bool BasicPrefixCTree<T>::Entry(unsigned bitset, T value, size_t numRead)
{
	constexpr auto CAPACITY_LENGTH = std::numeric_limits<size_t>::digits;
	if (numRead > CAPACITY_LENGTH)
	{
		throw std::runtime_error("ビット長さの指定が不正です");
	}

	using Bitset = std::bitset<CAPACITY_LENGTH>;
	return Entry(Bitset(bitset), value, numRead);
}
//-------------------------------------------------------------
template<typename T>
template<size_t numBit>
bool BasicPrefixCTree<T>::Entry(std::bitset<numBit> bitset, T value, size_t numRead)
{
	auto result = CreateNodeIfNotFound(bitset, numRead);
	if (result.second == false) 
		return false;

	result.first.SetValue(value);
	return true;
}

// @brief 指定された接頭符号に対応するノードを返す
//-------------------------------------------------------------
template<typename T>
template<size_t numBit>
std::pair<typename BasicPrefixCTree<T>::Node&, bool> BasicPrefixCTree<T>::CreateNodeIfNotFound(std::bitset<numBit> bits, size_t numRead)
{
	TreeWalker walker(*this);

	// 先頭ビットから順に読む
	int i = static_cast<int>(numRead) - 1;

	// ツリーをたどって、指定されたkeyにすでにノードが存在するかを確認
	for (; i >= 0; --i)
	{
		if (walker.HasChild(bits[i]) == false)
		{
			break;
		}
		walker.Next(bits[i]);
	}
	if(i < 0)
		return std::pair<Node&, bool>(m_nodeList.at(walker.m_current), false);

	// (ノードが作られていなかった続きから) 新しくノードを作成
	for (; i >= 0; --i)
	{
		index_t newIndex = static_cast<index_t>(m_nodeList.size());
		m_nodeList.push_back(Node{});

		auto& node = m_nodeList.at(walker.m_current);
		node.m_childIndex[(bits[i])] = newIndex;

		walker.Next(bits[i]);
	}
	return std::pair<Node&, bool>(m_nodeList.at(walker.m_current), true);
}

// @brief 現在指しているノードを返す
//-------------------------------------------------------------
template<typename T>
inline const typename typename BasicPrefixCTree<T>::Node& BasicPrefixCTree<T>::TreeWalker::Get() const
{
	return m_tree.m_nodeList.at(m_current);
}

// @brief 指定した分岐先に子が存在するか
//-------------------------------------------------------------
template<typename T>
inline bool BasicPrefixCTree<T>::TreeWalker::HasChild(int bit) const
{
	return GetNextIndex(bit) != invalidIndex;
}

// @brief 移動
//-------------------------------------------------------------
template<typename T>
inline bool BasicPrefixCTree<T>::TreeWalker::Next(int nextBit)
{
	m_current = GetNextIndex(nextBit);
	return m_current != invalidIndex;
}

// @brief 指定した分岐先の子を指すIndexを返す
//-------------------------------------------------------------
template<typename T>
inline index_t BasicPrefixCTree<T>::TreeWalker::GetNextIndex(int bit) const
{
	return Get().ChildIndex(bit);
}

}// end namespace

 //-------------------------------------------------------------
 // using
 //-------------------------------------------------------------
using PrefixC::PrefixCodeTree;

}// end namespace