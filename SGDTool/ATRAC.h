#ifndef ATRAC_H
#define ATRAC_H

#include <vector>
#include <fstream>
#include <string>

class ATRAC {
public:
	ATRAC();
	~ATRAC();

	void load();
	bool extract(uint32_t frequency);
	//std::vector<uint8_t> getData();
	bool setData(std::vector<uint8_t> v_data);
	bool save(std::fstream &file);
	void updateOffsets();

	//Stuff for creating SGXD
	void addName(std::string s, bool is_container);
	void editName(int index, std::string s);

	//"WAVE"
	uint32_t WAVE_size = 0;
	unsigned int files_amount = 0;

	struct WAVE_block_s
	{
		uint32_t name_offset = 0;
		uint8_t file_format = 0;//0x03 - Playstation 4-bit ADPCM, 0x04 - ATRAC3 +
		uint8_t channels = 0;
		uint32_t playback_frequency = 0;
		uint32_t bitrate = 0;//0x30 for 48kHz, 0x40 for 44.1kHz, 0x00 for ADPCM
		uint32_t data_size = 0;
		uint32_t total_samples = 0;
		uint32_t sgd_data_size = 0;
	};
	WAVE_block_s WAVE_block;

	//"NAME"
	uint32_t NAME_size = 0;
	uint32_t names_amount = 0;

	struct NAME_block_s
	{
		uint16_t file_index = 0;
		uint16_t name_type = 0xFF00;//0x0000 - container name (no index), 0x2000 - ADPCM name, 0x3000 - ATRAC3+ name.
		uint32_t name_offset = 0;
	};
	std::vector<NAME_block_s> NAME_block;
	std::vector<std::string> names_vec;//0 is always container name, 1 is the one with 0th index, 2 is the one with 1 index etc
	std::vector<uint8_t> data;
	uint32_t data_offset = 0;
	std::vector<uint8_t> hash;
	bool hash_exists = 0;
};

#endif