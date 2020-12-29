#ifndef ATRAC_H
#define ATRAC_H

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <direct.h>

class ATRAC {
public:
	enum ErrorCode {
		OK,
		FileCantOpen
	};
	ATRAC();
	~ATRAC();

	int load(std::ifstream& file);
	int extract(std::string path, uint32_t frequency, std::string name);

	//Sets new data and calls updateOffsets() because doing this manually is annoying. You can also don't touch offsets and edit data manually as everything is public
	int setData(std::vector<uint8_t> v_data);
	int save(std::fstream& file);
	void updateOffsets();

	//for creating/editing SGXD
	void addName(std::string s, bool is_container_name = false);
	void editName(int index, std::string s);

	//"WAVE"
	uint32_t WAVE_size = 0;
	uint32_t files_amount = 0;

	struct WAVE_block_s {
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
	WAVE_block_s WAVE_block;

	//"NAME"
	uint32_t NAME_size = 0;
	uint32_t names_amount = 0;

	struct NAME_block_s {
		uint16_t file_index = 0;
		uint16_t name_type = 0xFF00;//0x0000 - container name (no index), 0x2000 - ADPCM name, 0x3000 - ATRAC3+ name.
		uint32_t name_offset = 0;
	};
	std::vector<ATRAC::NAME_block_s> NAME_block;
	std::vector<std::string> names_vec;//0 is always container name, 1 is the one with 0th index, 2 is the one with 1 index etc

	std::vector<uint8_t> data;
	uint32_t data_offset = 0;

	std::vector<uint8_t> hash;
	//Took from Patapon 3 "DATAMS.BND/sound/atrac/make_new_hero.sgd"
	const unsigned char default_hash[0x30] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xD1,0x04,0x00,0xD5,0xA9,0x01,0x48,
		0xCB,0x91,0xBF,0xD4,0xB1,0xCB,0xFB,0x94,0xE5,0x0A,0x3D,0x10,0x00,0xF2,0x8B,0x38,
		0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10
	};
	bool hash_exists = 0;
};

#endif