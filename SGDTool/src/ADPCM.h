#ifndef ADPCM_H
#define ADPCM_H

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <direct.h>
#include <algorithm>
#include "libSGDNamespace.h"

namespace libSGD {
	class ADPCM
	{
	public:
		enum ErrorCode {
			OK,
			FileCantOpen,
			FileCantCreate,
			NameMissing
		};
		ADPCM();
		~ADPCM();

		/////////////
		//FUNCTIONS
		/////////////

		int load(std::ifstream& file);
		int save(std::fstream& file);
		int extract(std::string path_folder, uint32_t frequency = 11025, int file_index = -1, std::string name = "");

		//Разобраться, как ffmpeg определяет ошибочную длину файла. Должен быть флаг.
		int setData(std::vector<uint8_t>& v_data, int file_index = -1);
		void updateOffsets();

		int setName(std::string s, int file_index = 0);

		/////////////
		//VARIABLES
		/////////////

		//"RGND"
		uint32_t RGND_size = 0;
		uint32_t files_amount = 0;
		uint32_t data_block_offset = 0;

		struct RGND_block_s {
			uint32_t name_offset = 0;

			//0x03 - Playstation 4-bit ADPCM, 0x04 - ATRAC3 +
			uint8_t file_format = 0;
			uint8_t channels = 0;
			uint32_t playback_frequency = 0;

			//0x30 for 48kHz, 0x40 for 44.1kHz, 0x00 for ADPCM
			uint32_t bitrate = 0;
			//Original data size
			uint32_t data_size = 0;
			uint32_t total_samples = 0;

			//Data size from main SGXD header, also known as "full data size"
			uint32_t sgd_data_size = 0;
		};
		RGND_block_s RGND_block;

		//"NAME"
		uint32_t NAME_size = 0;
		uint32_t names_amount = 0;

		struct NAME_block_s {
			uint16_t file_index = 0;
			uint16_t name_type = 0xFF00;//0x0000 - container name (no index), 0x2000 - ADPCM name, 0x3000 - ATRAC3+ name.
			uint32_t name_offset = 0;

			//For sorting
			bool operator< (const NAME_block_s& a) const {
				return name_offset < a.name_offset;
			}
		};
		std::vector<ADPCM::NAME_block_s> NAME_block;
		std::vector<std::string> names_vec;//0 is always container name, 1 is the one with 0th index, 2 is the one with 1 index etc

		std::vector<uint8_t> data;
		uint32_t data_offset = 0;
	};
}

#endif // !ADPCM_H