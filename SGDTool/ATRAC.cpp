#include "ATRAC.h"

using namespace std;

ATRAC::ATRAC() {}
ATRAC::~ATRAC() {}

void ATRAC::load()
{
	//DON'T FORGET TO READ FUCKING HASH!!!!!
}
bool ATRAC::setData(std::vector<uint8_t> v_data)
{
	data = v_data;
	WAVE_block.data_size = data.size();
	WAVE_block.sgd_data_size = WAVE_block.data_size;

	if (hash_exists)
	{		
		while (WAVE_block.sgd_data_size % 0x10 != 0)
			WAVE_block.sgd_data_size++;
	}
	updateOffsets();
}
bool ATRAC::save(fstream& file)
{
	//WAVE
	uint32_t WAVE_offs = file.tellp();
	file.write("WAVE", sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&WAVE_size), sizeof(uint32_t));
	for (int i = 0; i < 4; i++)
		file.put(0);
	file.write(reinterpret_cast<char*>(&files_amount), sizeof(uint32_t));

	//Data block (1 time because ATRAC is always 1)
	for (int i = 0; i < 4; i++)
		file.put(0);
	file.write(reinterpret_cast<char*>(&WAVE_block.name_offset), sizeof(uint32_t));
	file.put(WAVE_block.file_format);
	file.put(WAVE_block.channels);
	for (int i = 0; i < 2; i++)
		file.put(0);
	file.write(reinterpret_cast<char*>(&WAVE_block.playback_frequency), sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&WAVE_block.bitrate), sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&WAVE_block.data_size), sizeof(uint32_t));
	for (int i = 0; i < 2; i++) {
		file.put(0);
		file.put(0x10);
	}
	for (int i = 0; i < 4; i++)
		file.put(0);
	file.write(reinterpret_cast<char*>(&WAVE_block.total_samples), sizeof(uint32_t));
	for (int i = 0; i < 8; i++)
		file.put(0xFF);
	file.write(reinterpret_cast<char*>(&WAVE_block.data_size), sizeof(uint32_t));
	for (int i = 0; i < 4; i++)
		file.put(0);
	WAVE_block.sgd_data_size += 0x80000000;
	file.write(reinterpret_cast<char*>(&WAVE_block.sgd_data_size), sizeof(uint32_t));
	WAVE_block.sgd_data_size -= 0x80000000;

	while (file.tellp() % 0x10 != 0)
		file.put(0);
	//Go back and write WAVE size
	file.seekp(WAVE_offs + 8);
	for (uint32_t counter = 0; ; counter += 4)
	{
		char code[4];
		file.read(code, sizeof(uint32_t));
		if (strcmp(code, "NAME") == 0)
		{
			WAVE_size = counter;
			break;
		}
	}
	file.seekp(WAVE_offs + 4);
	file.write(reinterpret_cast<char*>(&WAVE_size), sizeof(uint32_t));

	//NAME
	file.seekp(WAVE_offs + WAVE_size + 0x8);
	uint32_t NAME_offs = file.tellp();
	file.write("NAME", sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&NAME_size), sizeof(uint32_t));
	for (int i = 0; i < 4; i++)
		file.put(0);
	file.write(reinterpret_cast<char*>(&names_amount), sizeof(uint32_t));

	//Data block
	file.write(reinterpret_cast<char*>(&NAME_block[0]), sizeof(NAME_block_s) * uint64_t(NAME_block.size()));//Don't forget to sort by addresses
	for (int i = 0; i < names_vec.size(); i++)
	{
		file.write(names_vec[i].c_str(), names_vec[i].size());
		file.put(0);
	}
	while (file.tellp() % 0x10 != 0)
		file.put(0);

	file.write(reinterpret_cast<char*>(&data[0]), data.size());
	if (hash_exists)
	{
		while (file.tellp() % 0x10 != 0)
			file.put(0);
		file.write(reinterpret_cast<char*>(&hash[0]), hash.size());
	}
	//FIX OFFSETS, ADD LOAD FUNCTION
}
void ATRAC::updateOffsets()
{
	int temp_offset;
	WAVE_size = 8 + 0x38 * files_amount;
	while ((WAVE_size - 8) % 0x10 != 0)
		WAVE_size++;

	//NAME
	NAME_size = 8 + 8 * names_amount;
	temp_offset = 0x10 + 8 + WAVE_size + 8 + NAME_size;//first name in the table offset
	for (int i = 0; i < names_amount; i++)
	{
		NAME_block[i].name_offset = temp_offset;
		temp_offset += names_vec[i].size() + 1;
		NAME_size += names_vec[i].size() + 1;//+1 for null byte
	}
	while ((NAME_size - 8) % 0x10 != 0)
		NAME_size++;
	data_offset = 0x10 + 8 + WAVE_size + 8 + NAME_size;
	WAVE_block.data_size = data.size();

}
void ATRAC::addName(string s, bool is_container)
{
	names_amount++;
	names_vec.push_back(s);
	NAME_block_s tmp_block;

	if (is_container)
	{
		tmp_block.name_type = 0;
		tmp_block.file_index = 0;
	}
	else
	{
		tmp_block.name_type = 0x3000;
		tmp_block.file_index = names_amount - 1;
	}

	NAME_block.push_back(tmp_block);//DON'T TOUCH, IT WORKS. JUST SWAP TO RELEASE/DEBUG AND BACKWARDS, ERROR WILL DISAPPEAR
	updateOffsets();
}
void ATRAC::editName(int index, string ass)
{
	names_vec[index] = ass;
	updateOffsets();
}