//-------------------------------------------------------------
//! @brief	独自Deflate実装
//! @author	ｹｰﾄｩｽ=ｶｴﾚｽﾃｨｽ
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

	//! 1ビットロードする
	IbitStream& operator >> (int& out) override
	{
		out = Get();
		return *this;
	}
	//! 終端か
	bool Eof() const noexcept override
	{
		return m_nextByte >= m_numByte;
	}
	//! １ビットロード (戻り値に値を返す)
	int Get()
	{
		int bit = GetBitImpl();
		Next();
		return bit;
	}

	//! ビット列をロード
	//! 符号ではないデータは 要素の最下位ビットから順にパックされている
	int GetRange(size_t numbit)
	{
		int bit = 0;
		for(int i = 0; i < numbit; ++i)
		{
			bit |= (Get() << i);
		}
		return bit;
	}
	//! ビット列をロード
	//! 符号化されたデータは最上位ビットから順にパックされている
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

	//! 現在見ているbitを抜き出す
	int GetBitImpl() const
	{
		const int byte = m_binary[m_nextByte];
		const int maskedByte = byte & (0x01 << m_nextBit);

		return (maskedByte == 0) ? 0 : 1;
	}

	//! 次のビットにシフト
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
	// 読むだけで、メモリ確保はしないことにする
	// バイナリデータの生存期間に注意
	const char* m_binary;
	size_t		m_numByte;

	unsigned	m_nextBit  = 0;	// 次に読むbit
	size_t		m_nextByte = 0;	// 次に読むByte
};

//-------------------------------------------------------------
// inner function
//-------------------------------------------------------------

// @brief 拡張ビットデータの読み出し
//-------------------------------------------------------------
size_t ReadExValue(DeflateBitStream& bitstream, size_t baseVal, size_t exBit)
{
	if (exBit == 0) return baseVal;
	return baseVal + bitstream.GetRange(exBit);
}

// @brief スライド窓から拝借するパターンの長さ情報を読み出す
//-------------------------------------------------------------
size_t ReadLengthCode(unsigned code, DeflateBitStream& bitstream)
{
	const size_t CODE_BEGIN = 257;
	const size_t CODE_END   = 286;
	assert(code >= CODE_BEGIN && code < CODE_END);

	// テーブルの宣言
	const std::pair<size_t, size_t> CODE_TABLE[] =
	{
		// 最小の長さ / 拡張ビット数
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

// @brief スライド窓の参照開始地点(距離)の情報を読み出す
//-------------------------------------------------------------
size_t ReadDistanceCode(unsigned code, DeflateBitStream& bitstream)
{
	const size_t CODE_END = 30;
	assert(code < CODE_END);

	// テーブルの宣言
	const std::pair<size_t, size_t> CODE_TABLE[] =
	{
		// 最短の距離 / 拡張ビット数
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

//@brief 固定リテラルハフマンツリーを作成
//-------------------------------------------------------------
PrefixCodeTree MakeFixedHuffmanTree()
{
	PrefixCodeTree fixedTree;

	// 0 - 143 -> 8bit
	// [0011 0000] ～ [10111111]
	for (int i = 0; i <= 143; ++i)
	{
		int key = 0x30 + i;
		fixedTree.Entry<8>(key, i);
	}
	// 144 - 255 -> 9bit
	// [110010000] ～ [111111111]
	for (int i = 0; i <= (255 - 144); ++i)
	{
		int key = 0x190 + i;
		fixedTree.Entry<9>(key, i + 144);
	}
	// 256 - 279 -> 7bit
	// [0000000] ～ [0010111]
	for (int i = 0; i <= (279 - 256); ++i)
	{
		int key = 0x00 + i;
		fixedTree.Entry<7>(key, i + 256);
	}
	// 280 - 287 -> 8bit
	// [11000000] ～ [11000111]
	for (int i = 0; i <= (287 - 280); ++i)
	{
		int key = 0xC0 + i;
		fixedTree.Entry<8>(key, i + 280);
	}
	return fixedTree;
}

//@brief 固定ハフマン符号によるパース処理
//-------------------------------------------------------------
void DecodeWithFixedHuffman(DeflateBitStream& bitstream, LZ::LZSlideWindow& slideWnd, std::vector<char>* resultbuffer)
{
	// 固定ハフマンツリー作成
	auto tree = MakeFixedHuffmanTree();

	// ハフマン符号 -> (0 ～ 286)
	unsigned val;
	while (PrefixC::Decode(bitstream, tree, &val))
	{
		// 終端
		if (val == 256)
		{
			break;
		}
		// 値そのまま
		if (val <= 255)
		{
			resultbuffer->push_back(static_cast<char>(val));
			slideWnd.push_back(static_cast<char>(val));
			continue;
		}
		// if (val > 256)

		// 長さ情報
		size_t length = ReadLengthCode(val, bitstream);

		// 距離情報 (5bit固定)
		auto exVal = bitstream.GetCodedRange(5);
		size_t distance = ReadDistanceCode(exVal, bitstream);

		// 一致した値パターンを抽出
		auto valPattern = LZ::GetPattern(slideWnd, length, distance);

		resultbuffer->insert(resultbuffer->end(),valPattern.begin(), valPattern.end());
		LZ::PushBackPattern(&slideWnd, valPattern);
	}
}

// @brief 正規化されたハフマンツリーを作成
//-------------------------------------------------------------
template<size_t NUM_CODE>
PrefixCodeTree MakeNormalizedHuffmanTree(std::array<size_t, NUM_CODE>& codeLanArray)
{
	static_assert(NUM_CODE > 0, "不正な配列サイズ");

	// 事前に最大の長さを調べておく
	constexpr auto CAPACITY_LENGTH = std::numeric_limits<size_t>::digits;
	const size_t   maxCodeLength = *std::max_element(codeLanArray.begin(), codeLanArray.end());
	assert(maxCodeLength <= CAPACITY_LENGTH);

	// 符号長別に出現する符号個数をカウント
	std::array<size_t, CAPACITY_LENGTH> codeLenCount{};
	for (size_t i = 0; i < NUM_CODE; ++i)
	{
		auto length = codeLanArray[i];
		if (length > 0)
		{
			codeLenCount[length - 1] += 1;
		}
	}

	// 符号長別の最小の符号(最初に割り当てられる符号)を準備
	std::array<size_t, CAPACITY_LENGTH> allocateCodes{0};
	for (size_t i = 1; i < maxCodeLength; ++i)
	{
		// note:
		// 符号長が一つ小さいグループのなかで
		// 未割当てになる符号を１bit++したものを
		// この符号長グループの符号割り当てに使用する
		// ---> 瞬時複合可能な条件を満たす
		size_t prevEndCode = allocateCodes[i-1] + codeLenCount[i - 1];
		allocateCodes[i] = prevEndCode << 1;
	}

	// 正規化したコードで置き換えた結果でツリーを作成
	PrefixCodeTree tree;
	for (size_t i = 0; i < NUM_CODE; ++i)
	{
		auto length = codeLanArray[i];
		if (length > 0)
		{
			// todo: 後で整理
			unsigned newCode = static_cast<unsigned>(allocateCodes[length - 1]);
			unsigned value   = static_cast<unsigned>(i);
			tree.Entry(newCode, value, length);

			allocateCodes[length - 1] += 1;
		}
	}
	return tree;
}

//@brief "符号の長さ"を表す符号を連ねるハフマンツリーを読み込む
//-------------------------------------------------------------
PrefixCodeTree ReadCodeLenCodeTree(DeflateBitStream& bitstream, int numCodeLenCode)
{
	// note:
	// コードの長さを示す符号は 変則的な並びで記録されている
	// 普段利用されない符号長ほど、末尾に配置される並びにすることで、
	// 実際利用されなかった符号長の記録を省略し
	// 全体のデータ数を減らす最適化のため?
	const size_t indexSequence[] =
	{
		16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
	};

	// 読み出されなかった要素は「0」になる
	std::array<size_t, 19> codeLenCodeLens{};
	for (int i = 0; i < numCodeLenCode; ++i)
	{
		auto index = indexSequence[i];
		codeLenCodeLens[index] = bitstream.GetRange(3);
	}
	// この符号長配列からハフマンツリーを作る
	return MakeNormalizedHuffmanTree(codeLenCodeLens);
}

//@brief 繰り返し長さを読み出す
//-------------------------------------------------------------
size_t ReadRunLength(unsigned code, DeflateBitStream& bitstream)
{
	const size_t CODE_BEGIN = 16;
	const size_t CODE_END = 19;
	assert(code >= CODE_BEGIN && code < CODE_END);

	// テーブルの宣言
	const std::pair<size_t, size_t> TABLE[] =
	{
		// 最小の繰返し回数 / 拡張ビット数
		std::make_pair(3,	2),
		std::make_pair(3,	3),
		std::make_pair(11,	7),
	};
	
	const auto info = TABLE[code - CODE_BEGIN];
	return ReadExValue(bitstream, info.first, info.second);
}

//@brief "符号の長さ"ハフマンツリーを使って 符号ツリーを読み出す
//-------------------------------------------------------------
template<size_t CAPACITY_LENGTH>
PrefixCodeTree ReadCustomHuffmanTree(DeflateBitStream& bitstream, int numRead, PrefixCodeTree codeLenCodeTree)
{
	assert(numRead <= CAPACITY_LENGTH);
	std::array<size_t, CAPACITY_LENGTH> codeLenArray{};

	for (size_t index = 0; index < numRead; ++index)
	{
		// ビット読み出し -> "符号の長さ"ハフマンツリーでパース
		unsigned val;
		if( PrefixC::Decode(bitstream, codeLenCodeTree, &val) == false)
		{
			throw std::runtime_error("読み出し中にエラーが発生しました");
		}
		// 15 以下はそのまま記録
		if (val <= 15)
		{
			codeLenArray[index] = val;
			continue;
		}
		// 16は直前の値を、
		// 17, 18は「0」を一定回数繰り返す(ランレングス)
		auto runLength = ReadRunLength(val, bitstream);
		auto repeatVal = (val==16)?codeLenArray[index - 1] : 0;

		for (size_t j = 0; j < runLength; ++j)
		{
			codeLenArray[index+j] = repeatVal;
		}
		index += (runLength-1);
	}
	// この符号長配列からハフマンツリーを作る
	return MakeNormalizedHuffmanTree(codeLenArray);
}

//@brief カスタムリテラルハフマンツリーを作成
//-------------------------------------------------------------
PrefixCodeTree ReadLiteralTree(DeflateBitStream& bitstream, int numRead, PrefixCodeTree codeLenCodeTree)
{
	return 	ReadCustomHuffmanTree<286>(bitstream, numRead, codeLenCodeTree);
}

//@brief カスタム距離ハフマンツリーを作成
//-------------------------------------------------------------
PrefixCodeTree ReadDistanceTree(DeflateBitStream& bitstream, int numRead, PrefixCodeTree codeLenCodeTree)
{
	return 	ReadCustomHuffmanTree<32>(bitstream, numRead, codeLenCodeTree);
}

//@brief カスタムハフマン符号によるパース処理
//-------------------------------------------------------------
void DecodeWithCustomHuffman(DeflateBitStream& bitstream, LZ::LZSlideWindow& slideWnd, std::vector<char>* resultbuffer)
{
	// HLIT:　記録されたリテラル符号個数(257 ～ 286)
	int numLiteralCode  = bitstream.GetRange(5) + 257;

	// HDIST: 記録された距離符号個数(1 ～ 32)
	int numDistanceCode = bitstream.GetRange(5) + 1;

	// HCLEN: 「符号後の長さ」を表す符号個数(4 ～ 19)
	int numCodeLenCode = bitstream.GetRange(4) + 4;

	// 順番に各々のハフマンツリーを作成
	PrefixCodeTree codeLenCodeTree = ReadCodeLenCodeTree(bitstream, numCodeLenCode);		
	PrefixCodeTree literalTree     = ReadLiteralTree(bitstream,  numLiteralCode, codeLenCodeTree);
	PrefixCodeTree distanceTree    = ReadDistanceTree(bitstream, numDistanceCode, codeLenCodeTree);

	// あとは固定ハフマンの時とほぼ同じ
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
		// 長さ情報
		size_t length = ReadLengthCode(val, bitstream);

		// 距離情報 (距離ハフマンツリーを使って読む)
		unsigned exVal;
		PrefixC::Decode(bitstream, distanceTree, &exVal);
		size_t distance = ReadDistanceCode(exVal, bitstream);

		// 一致した値パターンを抽出
		auto valPattern = LZ::GetPattern(slideWnd, length, distance);
		resultbuffer->insert(resultbuffer->end(), valPattern.begin(), valPattern.end());
		LZ::PushBackPattern(&slideWnd, valPattern);
	}
}

} // end namespace


// @brief デコードする
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
			throw std::runtime_error("非圧縮タイプは未対応");
		case 1:
			DecodeWithFixedHuffman(bitstream, slideWnd, &result); break;
		case 2:
			DecodeWithCustomHuffman(bitstream, slideWnd, &result); break;
		case 3:
			throw std::runtime_error("よくわかんないデータが来た");
		}
		if (isLast) 
			break;
	}
	return result;
}