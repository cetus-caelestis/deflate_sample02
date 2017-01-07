//-------------------------------------------------------------
//! @brief	LZ77関連
//! @author	ｹｰﾄｩｽ=ｶｴﾚｽﾃｨｽ
//-------------------------------------------------------------

//-------------------------------------------------------------
// include
//-------------------------------------------------------------
#include <assert.h>
#include "LZ.h"

//-------------------------------------------------------------
// using
//-------------------------------------------------------------
using namespace MyUtility;

// @brief	コンストラクタ
//-------------------------------------------------------------
LZ::LZSlideWindow::LZSlideWindow(size_t buffersize)
	:m_maxbufferSize(buffersize)
{
	m_buffer.reserve(buffersize);
	assert(m_buffer.size() == 0);
}

// @brief	要素のプッシュ
//-------------------------------------------------------------
void LZ::LZSlideWindow::push_back(char value)
{
	// 1周目
	if (m_buffer.size() < m_maxbufferSize)
	{
		m_buffer.push_back(value);
	}
	// 2周目以降
	else
	{
		m_buffer.at(m_top) = value;
		m_top = (m_top + 1) % m_maxbufferSize;
	}
}

// @brief	ランダムアクセス
// @note	構造はだいたい環状バッファと同じ
//-------------------------------------------------------------
char& LZ::LZSlideWindow::at(size_t index)
{
	// note; 
	// 本当はバッファサイズを２の乗数に固定することで
	// 「%」を「&」の計算に置き換えられる
	return m_buffer.at((index + m_top)% m_maxbufferSize);
}
//-------------------------------------------------------------
const char& LZ::LZSlideWindow::at(size_t index) const
{
	return m_buffer.at((index + m_top) % m_maxbufferSize);
}


// @brief	長さ/距離 に該当するデータパターンを返す
// @note	長さ > 距離 に対応できるようにする
// @note	新しくpush_back()された値ほど、距離は短い
//-------------------------------------------------------------
std::vector<char> LZ::GetPattern(const LZSlideWindow& slideWnd, size_t length, size_t startDistance)
{
	std::vector<char> vec;
	for (size_t i = 0; i < length; ++i)
	{
		size_t distance = startDistance - (i % startDistance);
		size_t index = slideWnd.size() - distance;

		vec.push_back(slideWnd.at(index));
	}
	return vec;
}

// @brief データパターンを記録する
//-------------------------------------------------------------
void LZ::PushBackPattern(LZSlideWindow* slideWnd, const std::vector<char> substr)
{
	for (const auto& value : substr)
	{
		slideWnd->push_back(value);
	}
}