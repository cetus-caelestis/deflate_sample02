//-------------------------------------------------------------
//! @brief	簡易Deflate実装
//! @author	ｹｰﾄｩｽ=ｶｴﾚｽﾃｨｽ
//-------------------------------------------------------------
#pragma once

//-------------------------------------------------------------
// include
//-------------------------------------------------------------
#include <vector>

namespace MyUtility
{
namespace Deflate
{

//! デコードする
std::vector<char> Decode(const char* binary, size_t numByte);


}// end namespace Deflate
}// end namespace MyUtility