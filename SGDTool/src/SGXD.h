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

namespace libSGD {
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

		/////////////
		//FUNCTIONS
		/////////////

		//Initialize values with ones from file.
		//file - Absolute path to .sgd file
		int load(std::string file);

		//Initializes standard values for given type. You must call setData() and atrac/adpcm.addName() after that.
		int create(SGXD::FileType type = SGXD::FileType::ATRACType);
		//Saves current file as [path]. If hash was not defined but required, default_hash is used.
		int save(std::string file);

		//Saves [file_index]th subfile to [path_fodler] with [frequency] playback rate. Use [name] to override default name, stated in NAME header.
		//path_folder - Absolute path where the container name folder will be created (path_folder/sgd_name/[n1.vag, n2.vag...]).
		//frequency - Playback frequency to extract with. If 0, gets value automatically.
		//file_index - Subfile index, 0 is the first subfile. If -1, all files will be extracted.
		//name - Override original file name with this (../myname.wav). [ATRAC ONLY]
		int extract(std::string path_folder, uint32_t frequency = 0, int file_index = -1, std::string name = "");

		//Prints important and not important stuff.
		void printInfo();

		//Calls type's updateOffsets() and gets SGXD header offsets
		void updateOffsets();

		//Use hash for saving and calculating offsets or not
		void setHash(bool val);

		//Use this instead of direct variable change as it fixes some additional stuff.
		void setContainerName(std::string s);

		//Variables
		uint32_t
			sgd_name_offset = 0,
			data_offset = 0,
			data_size = 0;

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
}

#endif