//-------------------------------------------------------------
//! @brief	�Ǝ�Deflate����
//! @author	��ĩ�=��ڽè�
//-------------------------------------------------------------

//-------------------------------------------------------------
// include
//-------------------------------------------------------------
#include <assert.h>
#include <iostream>
#include <array>

#include "PrefixCodeTree.h"
#include "LZ.h"
#include "Deflate.h"

//-------------------------------------------------------------
// using
//-------------------------------------------------------------
using namespace MyUtility;

namespace
{
//-------------------------------------------------------------
// inner class
//-------------------------------------------------------------
class DeflateBitStream : public PrefixC::IbitStream
{
public:
	explicit DeflateBitStream(const char* binary, size_t numByte)
		:m_binary(binary)
		,m_numByte(numByte)
	{}

	//! 1�r�b�g���[�h����
	IbitStream& operator >> (int& out) override
	{
		out = Get();
		return *this;
	}
	//! �I�[��
	bool Eof() const noexcept override
	{
		return m_nextByte >= m_numByte;
	}
	//! �P�r�b�g���[�h (�߂�l�ɒl��Ԃ�)
	int Get()
	{
		int bit = GetBitImpl();
		Next();
		return bit;
	}

	//! �r�b�g������[�h
	//! �����ł͂Ȃ��f�[�^�� �v�f�̍ŉ��ʃr�b�g���珇�Ƀp�b�N����Ă���
	int GetRange(size_t numbit)
	{
		int bit = 0;
		for(int i = 0; i < numbit; ++i)
		{
			bit |= (Get() << i);
		}
		return bit;
	}
	//! �r�b�g������[�h
	//! ���������ꂽ�f�[�^�͍ŏ�ʃr�b�g���珇�Ƀp�b�N����Ă���
	int GetCodedRange(size_t numbit)
	{
		int bit = 0;
		for (int i = 0; i < numbit; ++i)
		{
			bit <<= 1;
			bit |= Get();
		}
		return bit;
	}

private:

	//! ���݌��Ă���bit�𔲂��o��
	int GetBitImpl() const
	{
		const int byte = m_binary[m_nextByte];
		const int maskedByte = byte & (0x01 << m_nextBit);

		return (maskedByte == 0) ? 0 : 1;
	}

	//! ���̃r�b�g�ɃV�t�g
	void Next() noexcept
	{
		++m_nextBit;
		if (m_nextBit >= 8)
		{
			m_nextBit = 0;
			++m_nextByte;
		}
	}

	// note:
	// �ǂނ����ŁA�������m�ۂ͂��Ȃ����Ƃɂ���
	// �o�C�i���f�[�^�̐������Ԃɒ���
	const char* m_binary;
	size_t		m_numByte;

	unsigned	m_nextBit  = 0;	// ���ɓǂ�bit
	size_t		m_nextByte = 0;	// ���ɓǂ�Byte
};

//-------------------------------------------------------------
// inner function
//-------------------------------------------------------------

// @brief �g���r�b�g�f�[�^�̓ǂݏo��
//-------------------------------------------------------------
size_t ReadExValue(DeflateBitStream& bitstream, size_t baseVal, size_t exBit)
{
	if (exBit == 0) return baseVal;
	return baseVal + bitstream.GetRange(exBit);
}

// @brief �X���C�h������q�؂���p�^�[���̒�������ǂݏo��
//-------------------------------------------------------------
size_t ReadLengthCode(unsigned code, DeflateBitStream& bitstream)
{
	const size_t CODE_BEGIN = 257;
	const size_t CODE_END   = 286;
	assert(code >= CODE_BEGIN && code < CODE_END);

	// �e�[�u���̐錾
	const std::pair<size_t, size_t> CODE_TABLE[] =
	{
		// �ŏ��̒��� / �g���r�b�g��
		std::make_pair(3,	0),
		std::make_pair(4,	0),
		std::make_pair(5,	0),
		std::make_pair(6,	0),
		std::make_pair(7,	0),
		std::make_pair(8,	0),
		std::make_pair(9,	0),
		std::make_pair(10,	0),
		std::make_pair(11,	1),
		std::make_pair(13,	1),
		std::make_pair(15,	1),
		std::make_pair(17,	1),
		std::make_pair(19,	2),
		std::make_pair(23,	2),
		std::make_pair(27,	2),
		std::make_pair(31,	2),
		std::make_pair(35,	3),
		std::make_pair(43,	3),
		std::make_pair(51,	3),
		std::make_pair(59,	3),
		std::make_pair(67,	4),
		std::make_pair(83,	4),
		std::make_pair(99,	4),
		std::make_pair(115,	4),
		std::make_pair(131,	5),
		std::make_pair(163,	5),
		std::make_pair(195,	5),
		std::make_pair(227,	5),
		std::make_pair(258,	0),
	};
	auto info = CODE_TABLE[code - CODE_BEGIN];
	return ReadExValue(bitstream, info.first, info.second);
}

// @brief �X���C�h���̎Q�ƊJ�n�n�_(����)�̏���ǂݏo��
//-------------------------------------------------------------
size_t ReadDistanceCode(unsigned code, DeflateBitStream& bitstream)
{
	const size_t CODE_END = 30;
	assert(code < CODE_END);

	// �e�[�u���̐錾
	const std::pair<size_t, size_t> CODE_TABLE[] =
	{
		// �ŒZ�̋��� / �g���r�b�g��
		std::make_pair(1,		0),
		std::make_pair(2,		0),
		std::make_pair(3,		0),
		std::make_pair(4,		0),
		std::make_pair(5,		1),
		std::make_pair(7,		1),
		std::make_pair(9,		2),
		std::make_pair(13,		2),
		std::make_pair(17,		3),
		std::make_pair(25,		3),
		std::make_pair(33,		4),
		std::make_pair(49,		4),
		std::make_pair(65,		5),
		std::make_pair(97,		5),
		std::make_pair(129,		6),
		std::make_pair(193,		6),
		std::make_pair(257,		7),
		std::make_pair(385,		7),
		std::make_pair(513,		8),
		std::make_pair(769,		8),
		std::make_pair(1025,	9),
		std::make_pair(1537,	9),
		std::make_pair(2049,	10),
		std::make_pair(3073,	10),
		std::make_pair(4097,	11),
		std::make_pair(6145,	11),
		std::make_pair(8193,	12),
		std::make_pair(12289,	12),
		std::make_pair(16385,	13),
		std::make_pair(24577,	13),
	};
	auto info = CODE_TABLE[code];
	return ReadExValue(bitstream, info.first, info.second);
}

//@brief �Œ胊�e�����n�t�}���c���[���쐬
//-------------------------------------------------------------
PrefixCodeTree MakeFixedHuffmanTree()
{
	PrefixCodeTree fixedTree;

	// 0 - 143 -> 8bit
	// [0011 0000] �` [10111111]
	for (int i = 0; i <= 143; ++i)
	{
		int key = 0x30 + i;
		fixedTree.Entry<8>(key, i);
	}
	// 144 - 255 -> 9bit
	// [110010000] �` [111111111]
	for (int i = 0; i <= (255 - 144); ++i)
	{
		int key = 0x190 + i;
		fixedTree.Entry<9>(key, i + 144);
	}
	// 256 - 279 -> 7bit
	// [0000000] �` [0010111]
	for (int i = 0; i <= (279 - 256); ++i)
	{
		int key = 0x00 + i;
		fixedTree.Entry<7>(key, i + 256);
	}
	// 280 - 287 -> 8bit
	// [11000000] �` [11000111]
	for (int i = 0; i <= (287 - 280); ++i)
	{
		int key = 0xC0 + i;
		fixedTree.Entry<8>(key, i + 280);
	}
	return fixedTree;
}

//@brief �Œ�n�t�}�������ɂ��p�[�X����
//-------------------------------------------------------------
void DecodeWithFixedHuffman(DeflateBitStream& bitstream, LZ::LZSlideWindow& slideWnd, std::vector<char>* resultbuffer)
{
	// �Œ�n�t�}���c���[�쐬
	auto tree = MakeFixedHuffmanTree();

	// �n�t�}������ -> (0 �` 286)
	unsigned val;
	while (PrefixC::Decode(bitstream, tree, &val))
	{
		// �I�[
		if (val == 256)
		{
			break;
		}
		// �l���̂܂�
		if (val <= 255)
		{
			resultbuffer->push_back(static_cast<char>(val));
			slideWnd.push_back(static_cast<char>(val));
			continue;
		}
		// if (val > 256)

		// �������
		size_t length = ReadLengthCode(val, bitstream);

		// ������� (5bit�Œ�)
		auto exVal = bitstream.GetCodedRange(5);
		size_t distance = ReadDistanceCode(exVal, bitstream);

		// ��v�����l�p�^�[���𒊏o
		auto valPattern = LZ::GetPattern(slideWnd, length, distance);

		resultbuffer->insert(resultbuffer->end(),valPattern.begin(), valPattern.end());
		LZ::PushBackPattern(&slideWnd, valPattern);
	}
}

// @brief ���K�����ꂽ�n�t�}���c���[���쐬
//-------------------------------------------------------------
template<size_t NUM_CODE>
PrefixCodeTree MakeNormalizedHuffmanTree(std::array<size_t, NUM_CODE>& codeLanArray)
{
	static_assert(NUM_CODE > 0, "�s���Ȕz��T�C�Y");

	// ���O�ɍő�̒����𒲂ׂĂ���
	constexpr auto CAPACITY_LENGTH = std::numeric_limits<size_t>::digits;
	const size_t   maxCodeLength = *std::max_element(codeLanArray.begin(), codeLanArray.end());
	assert(maxCodeLength <= CAPACITY_LENGTH);

	// �������ʂɏo�����镄�������J�E���g
	std::array<size_t, CAPACITY_LENGTH> codeLenCount{};
	for (size_t i = 0; i < NUM_CODE; ++i)
	{
		auto length = codeLanArray[i];
		if (length > 0)
		{
			codeLenCount[length - 1] += 1;
		}
	}

	// �������ʂ̍ŏ��̕���(�ŏ��Ɋ��蓖�Ă��镄��)������
	std::array<size_t, CAPACITY_LENGTH> allocateCodes{0};
	for (size_t i = 1; i < maxCodeLength; ++i)
	{
		// note:
		// ����������������O���[�v�̂Ȃ���
		// �������ĂɂȂ镄�����Pbit++�������̂�
		// ���̕������O���[�v�̕������蓖�ĂɎg�p����
		// ---> �u�������\�ȏ����𖞂���
		size_t prevEndCode = allocateCodes[i-1] + codeLenCount[i - 1];
		allocateCodes[i] = prevEndCode << 1;
	}

	// ���K�������R�[�h�Œu�����������ʂŃc���[���쐬
	PrefixCodeTree tree;
	for (size_t i = 0; i < NUM_CODE; ++i)
	{
		auto length = codeLanArray[i];
		if (length > 0)
		{
			// todo: ��Ő���
			unsigned newCode = static_cast<unsigned>(allocateCodes[length - 1]);
			unsigned value   = static_cast<unsigned>(i);
			tree.Entry(newCode, value, length);

			allocateCodes[length - 1] += 1;
		}
	}
	return tree;
}

//@brief "�����̒���"��\��������A�˂�n�t�}���c���[��ǂݍ���
//-------------------------------------------------------------
PrefixCodeTree ReadCodeLenCodeTree(DeflateBitStream& bitstream, int numCodeLenCode)
{
	// note:
	// �R�[�h�̒��������������� �ϑ��I�ȕ��тŋL�^����Ă���
	// ���i���p����Ȃ��������قǁA�����ɔz�u�������тɂ��邱�ƂŁA
	// ���ۗ��p����Ȃ������������̋L�^���ȗ���
	// �S�̂̃f�[�^�������炷�œK���̂���?
	const size_t indexSequence[] =
	{
		16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
	};

	// �ǂݏo����Ȃ������v�f�́u0�v�ɂȂ�
	std::array<size_t, 19> codeLenCodeLens{};
	for (int i = 0; i < numCodeLenCode; ++i)
	{
		auto index = indexSequence[i];
		codeLenCodeLens[index] = bitstream.GetRange(3);
	}
	// ���̕������z�񂩂�n�t�}���c���[�����
	return MakeNormalizedHuffmanTree(codeLenCodeLens);
}

//@brief �J��Ԃ�������ǂݏo��
//-------------------------------------------------------------
size_t ReadRunLength(unsigned code, DeflateBitStream& bitstream)
{
	const size_t CODE_BEGIN = 16;
	const size_t CODE_END = 19;
	assert(code >= CODE_BEGIN && code < CODE_END);

	// �e�[�u���̐錾
	const std::pair<size_t, size_t> TABLE[] =
	{
		// �ŏ��̌J�Ԃ��� / �g���r�b�g��
		std::make_pair(3,	2),
		std::make_pair(3,	3),
		std::make_pair(11,	7),
	};
	
	const auto info = TABLE[code - CODE_BEGIN];
	return ReadExValue(bitstream, info.first, info.second);
}

//@brief "�����̒���"�n�t�}���c���[���g���� �����c���[��ǂݏo��
//-------------------------------------------------------------
template<size_t CAPACITY_LENGTH>
PrefixCodeTree ReadCustomHuffmanTree(DeflateBitStream& bitstream, int numRead, PrefixCodeTree codeLenCodeTree)
{
	assert(numRead <= CAPACITY_LENGTH);
	std::array<size_t, CAPACITY_LENGTH> codeLenArray{};

	for (size_t index = 0; index < numRead; ++index)
	{
		// �r�b�g�ǂݏo�� -> "�����̒���"�n�t�}���c���[�Ńp�[�X
		unsigned val;
		if( PrefixC::Decode(bitstream, codeLenCodeTree, &val) == false)
		{
			throw std::runtime_error("�ǂݏo�����ɃG���[���������܂���");
		}
		// 15 �ȉ��͂��̂܂܋L�^
		if (val <= 15)
		{
			codeLenArray[index] = val;
			continue;
		}
		// 16�͒��O�̒l���A
		// 17, 18�́u0�v�����񐔌J��Ԃ�(���������O�X)
		auto runLength = ReadRunLength(val, bitstream);
		auto repeatVal = (val==16)?codeLenArray[index - 1] : 0;

		for (size_t j = 0; j < runLength; ++j)
		{
			codeLenArray[index+j] = repeatVal;
		}
		index += (runLength-1);
	}
	// ���̕������z�񂩂�n�t�}���c���[�����
	return MakeNormalizedHuffmanTree(codeLenArray);
}

//@brief �J�X�^�����e�����n�t�}���c���[���쐬
//-------------------------------------------------------------
PrefixCodeTree ReadLiteralTree(DeflateBitStream& bitstream, int numRead, PrefixCodeTree codeLenCodeTree)
{
	return 	ReadCustomHuffmanTree<286>(bitstream, numRead, codeLenCodeTree);
}

//@brief �J�X�^�������n�t�}���c���[���쐬
//-------------------------------------------------------------
PrefixCodeTree ReadDistanceTree(DeflateBitStream& bitstream, int numRead, PrefixCodeTree codeLenCodeTree)
{
	return 	ReadCustomHuffmanTree<32>(bitstream, numRead, codeLenCodeTree);
}

//@brief �J�X�^���n�t�}�������ɂ��p�[�X����
//-------------------------------------------------------------
void DecodeWithCustomHuffman(DeflateBitStream& bitstream, LZ::LZSlideWindow& slideWnd, std::vector<char>* resultbuffer)
{
	// HLIT:�@�L�^���ꂽ���e����������(257 �` 286)
	int numLiteralCode  = bitstream.GetRange(5) + 257;

	// HDIST: �L�^���ꂽ����������(1 �` 32)
	int numDistanceCode = bitstream.GetRange(5) + 1;

	// HCLEN: �u������̒����v��\��������(4 �` 19)
	int numCodeLenCode = bitstream.GetRange(4) + 4;

	// ���ԂɊe�X�̃n�t�}���c���[���쐬
	PrefixCodeTree codeLenCodeTree = ReadCodeLenCodeTree(bitstream, numCodeLenCode);		
	PrefixCodeTree literalTree     = ReadLiteralTree(bitstream,  numLiteralCode, codeLenCodeTree);
	PrefixCodeTree distanceTree    = ReadDistanceTree(bitstream, numDistanceCode, codeLenCodeTree);

	// ���Ƃ͌Œ�n�t�}���̎��Ƃقړ���
	unsigned val;
	while (PrefixC::Decode(bitstream, literalTree, &val))
	{
		if (val == 256)
		{
			break;
		}
		if (val <= 255)
		{
			resultbuffer->push_back(static_cast<char>(val));
			slideWnd.push_back(static_cast<char>(val));
			continue;
		}
		// �������
		size_t length = ReadLengthCode(val, bitstream);

		// ������� (�����n�t�}���c���[���g���ēǂ�)
		unsigned exVal;
		PrefixC::Decode(bitstream, distanceTree, &exVal);
		size_t distance = ReadDistanceCode(exVal, bitstream);

		// ��v�����l�p�^�[���𒊏o
		auto valPattern = LZ::GetPattern(slideWnd, length, distance);
		resultbuffer->insert(resultbuffer->end(), valPattern.begin(), valPattern.end());
		LZ::PushBackPattern(&slideWnd, valPattern);
	}
}

} // end namespace


// @brief �f�R�[�h����
//-------------------------------------------------------------	
std::vector<char> MyUtility::Deflate::Decode(const char* binary, size_t numByte)
{
	DeflateBitStream	bitstream(binary, numByte);
	LZ::LZSlideWindow	slideWnd(32768);

	std::vector<char> result;

	while (!bitstream.Eof())
	{
		bool isLast = (bitstream.Get() == 1);
		int  type   = bitstream.GetRange(2);

		switch (type)
		{
		case 0:
			throw std::runtime_error("�񈳏k�^�C�v�͖��Ή�");
		case 1:
			DecodeWithFixedHuffman(bitstream, slideWnd, &result); break;
		case 2:
			DecodeWithCustomHuffman(bitstream, slideWnd, &result); break;
		case 3:
			throw std::runtime_error("�悭�킩��Ȃ��f�[�^������");
		}
		if (isLast) 
			break;
	}
	return result;
}