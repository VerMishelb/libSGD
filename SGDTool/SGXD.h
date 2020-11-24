#ifndef SGXD_H
#define SGXD_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "ATRAC.h"
#include "ADPCM.h"

//uint32_t = unsigned int
//uint64_t = unsigned long long
//uint16_t = unsigned short
/*
	Error codes:
		0	- no error
		1	- file can't be opened
		2	- invalid SGXD

	Types:
		0	- ATRAC
		1	- ADPCM
		-1	- Unknown
*/
//size_no_header=size_full-0x30

class SGXD {
public:
	SGXD();
	~SGXD();

	//Main functions
	int load(std::string file);//Initialize values with ones from file
	void create(int type);//Initialize standard values for given type
	bool save(std::string file);//WRITE FUNCTION FOR SAVING, RECHECK ALL FUNCTIONS
	bool save();//Saves current file in case it exists
	void extract();//Extracts all files
	void extract(int file_index);
	void printInfo();//Print interesting stuff. Or not so interesting.
	void updateOffsets();

	//Important shit
	void setContainerName(string s);

	//Variables
	uint32_t
		sgd_name_offset,
		data_offset,
		data_size,
		file_size;
	int type = -1;//1=VAG or 0=ATRAC
	std::string sgd_name;//container name
	std::string file_path;

	ATRAC atrac;
	ADPCM adpcm;


	//hash at the end
	//todo
};

#endif