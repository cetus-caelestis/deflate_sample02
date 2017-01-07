//-------------------------------------------------------------
//! @brief	LZ77関連
//! @author	ｹｰﾄｩｽ=ｶｴﾚｽﾃｨｽ
//-------------------------------------------------------------
#pragma once

//-------------------------------------------------------------
// include
//-------------------------------------------------------------
#include <vector>
#include <assert.h>

namespace MyUtility
{
namespace LZ
{
//-------------------------------------------------------------
// class (LZスライド窓)
//-------------------------------------------------------------	
class LZSlideWindow
{
public:

	explicit LZSlideWindow(size_t buffersize);

	void push_back(char value);
	char& at(size_t index);
	const char& at(size_t index) const;
	size_t size() const { return m_buffer.size();}

private:

	size_t				m_top    = 0;
	const size_t		m_maxbufferSize;
	std::vector<char>	m_buffer;
};

//-------------------------------------------------------------
// helper function
//-------------------------------------------------------------

//! 長さ/距離 に該当するデータパターンを返す
std::vector<char> GetPattern(const LZSlideWindow&, size_t length, size_t startDistance);

//! データパターンを記録する
void PushBackPattern(LZSlideWindow*, const std::vector<char> substr);


}// end namespace LZ
}// end namespace MyUtility