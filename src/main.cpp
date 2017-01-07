#include <iostream>
#include <fstream>
#include <string>
#include "MyUtility/Deflate.h"

int main()
{
	using namespace MyUtility;
	try
	{
		// �J�X�^���n�t�}�� "ABRACADABRA"
		const char codedData[] = "\x15\xC5\x41\x11\x00\x00\x00\x01\xC1\x6C\x87\x04\xFA\x87\x31\x5E\x8B\x8A\xC9\x19";

		// ��������Deflate�f�[�^�̃p�[�X����
		auto data = Deflate::Decode(codedData, std::size(codedData));

		// ���ʂ̏o��
		std::string resultStr(data.data(), data.size());
		std::cout << resultStr << std::endl;
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what();
	}
	catch (...)
	{

	}
	getchar();
	return 0;
}

