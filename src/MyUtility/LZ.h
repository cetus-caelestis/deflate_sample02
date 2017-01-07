//-------------------------------------------------------------
//! @brief	LZ77�֘A
//! @author	��ĩ�=��ڽè�
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
// class (LZ�X���C�h��)
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

//! ����/���� �ɊY������f�[�^�p�^�[����Ԃ�
std::vector<char> GetPattern(const LZSlideWindow&, size_t length, size_t startDistance);

//! �f�[�^�p�^�[�����L�^����
void PushBackPattern(LZSlideWindow*, const std::vector<char> substr);


}// end namespace LZ
}// end namespace MyUtility