#include <iostream>
#include "SGXD.h"

/*
The following file includes some examples of libSGD usage.
If you don't know how to use certain function, check this place as you'll probably find answers here.
*/

int main() {
	SGXD test;
	std::string path = "F:\\PSP\\GAME\\UCES01177_p2eur\\PSP_GAME\\USRDIR\\@DATA_CMN\\sound\\@carnival_bgm_00\\@carnival_bgm\\organization.sgd";

	//Load SGD, show info, then extract subfiles and re-save SGD to other place
	test.load(path);
	test.printInfo();
	test.extract("D:\\VSProjects\\libSGD tests\\LibSGD tests\\test_files\\extract");
	test.save("D:\\VSProjects\\libSGD tests\\LibSGD tests\\test_files\\save\\1.sgd");

	//Create ATRAC SGD and save it
	SGXD myfile;
	myfile.create(SGXD::FileType::ATRACType);
	myfile.setContainerName("terraria_wtf");

		std::ifstream source("D:\\VSProjects\\libSGD tests\\LibSGD tests\\test_files\\create\\atrac_test.wav", std::ios::binary);
		std::vector<uint8_t> source_data;
		source.seekg(0, std::ios::end);
		uint32_t s = source.tellg();
		source.seekg(0);
		source_data.resize(s);
		source.read(reinterpret_cast<char*>(source_data.data()), s);

	myfile.atrac.setData(source_data);
	myfile.atrac.setName("terraria_omgwtf");
	myfile.setHash(true);
	myfile.updateOffsets();

	myfile.save("D:\\VSProjects\\libSGD tests\\LibSGD tests\\test_files\\create\\result.sgd");
	//Broken:
	//0x4 File name offset (aka container name). 32 bit
	//0x8 First subfile offset(RIFF). 32 bit
	return 0;
}