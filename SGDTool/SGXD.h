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
//size_no_header=size_full-0x30

class SGXD {
public:
	enum ErrorCode {
		OK,
		FileCantOpen,
		FileInvalidSGXD
	};
	enum FileType {
		UnknownType = -1,
		ATRACType,
		ADPCMType
	};
	SGXD();
	~SGXD();


	//Main functions

	//Initialize values with ones from file.
	int load(std::string file);
	//Initializes standard values for given type.
	int create(SGXD::FileType type = SGXD::FileType::ATRACType);
	//Saves current file as [path] with or without hash. If hash was not defined, default_hash is used.
	int save(std::string file);
	//Saves [file_index]th subfile to [path] folder with [frequency] playback rate. Use [name] to override default name, stated in NAME header.
	int extract(std::string path, uint32_t frequency = 0, int file_index = -1, std::string name = "");
	//Prints important and not important stuff.
	void printInfo();
	//Updates offsets and sizes based on known information.
	void updateOffsets();
	//Use hash for saving and calculating offsets or not
	void setHash(bool val);

	//Use this instead of direct variable change as it fixes some additional stuff.
	void setContainerName(std::string s);

	//Variables
	uint32_t
		sgd_name_offset,
		data_offset,
		data_size,
		file_size;
	//1=VAG or 0=ATRAC
	int type = FileType::UnknownType;
	//Container name
	std::string sgd_name;
	std::string file_path;

	ATRAC atrac;
	ADPCM adpcm;


	//hash at the end
	//todo
};

#endif