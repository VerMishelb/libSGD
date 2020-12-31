#include "ATRAC.h"

using namespace std;

ATRAC::ATRAC() {}
ATRAC::~ATRAC() {}


//Help functions
string read_string(ifstream& a) {
	string result;
	uint8_t t = a.get();

	//Protection from yeeting into space
	for (int length = 0; t != 0 && length < 0xff; length++) {
		if (t == '\0') {
			return result;
		}
		result += t;
		t = a.get();
	}
	return result;
}


int ATRAC::load(ifstream& file) {
	uint32_t value = 0;

	//WAVE
	file.seekg(0x14);
	file.read(reinterpret_cast<char*>(&WAVE_size), sizeof(uint32_t));
	file.seekg(0x1C);
	file.read(reinterpret_cast<char*>(&files_amount), sizeof(uint32_t));

	//Data block
	file.seekg(0x24);
	file.read(reinterpret_cast<char*>(&WAVE_block.name_offset), sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(&WAVE_block.file_format), sizeof(uint8_t));
	file.read(reinterpret_cast<char*>(&WAVE_block.channels), sizeof(uint8_t));
	file.seekg(0x2C);
	file.read(reinterpret_cast<char*>(&WAVE_block.playback_frequency), sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(&WAVE_block.bitrate), sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(&WAVE_block.data_size), sizeof(uint32_t));
	file.seekg(0x40);
	file.read(reinterpret_cast<char*>(&WAVE_block.total_samples), sizeof(uint32_t));
	file.seekg(0x4C);
	file.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
	if (value != WAVE_block.data_size) {
		std::cout << "WARNING: Data size differs.\n";
	}
	file.seekg(0x54);
	file.read(reinterpret_cast<char*>(&WAVE_block.sgd_data_size), sizeof(uint32_t));

	//NAME
	file.seekg(0x64);
	file.read(reinterpret_cast<char*>(&NAME_size), sizeof(uint32_t));
	file.seekg(0x6C);
	file.read(reinterpret_cast<char*>(&names_amount), sizeof(uint32_t));
	names_vec.clear();
	NAME_block.clear();
	names_vec.reserve(names_amount);
	NAME_block.resize(names_amount);
	for (uint32_t i = 0; i < names_amount; i++) {
		names_vec.push_back("Unknown " + to_string(i));
	}

	for (uint32_t i = 0; i < names_amount; i++) {
		file.seekg(0x70 + 0x8 * i);
		file.read(reinterpret_cast<char*>(&NAME_block[i].file_index), sizeof(uint16_t));
		file.read(reinterpret_cast<char*>(&NAME_block[i].name_type), sizeof(uint16_t));
		file.read(reinterpret_cast<char*>(&NAME_block[i].name_offset), sizeof(uint16_t));

		file.seekg(NAME_block[i].name_offset);
		if (NAME_block[i].name_type == 0) {
			names_vec[0] = read_string(file);
		}
		else {
			names_vec[NAME_block[i].file_index + 1] = read_string(file);
		}
	}

	//data
	file.seekg(data_offset);
	data.clear();
	data.resize(WAVE_block.data_size);
	file.read(reinterpret_cast<char*>(data.data()), WAVE_block.data_size);

	//Hash
	hash_exists = true;
	file.seekg(-0x10, ios::end);
	for (int i = 0; i < 4; i++)
	{
		file.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
		if (value != 0x10101010) {
			hash_exists = false;
			break;
		}
	}
	file.clear();//Just in case

	if (hash_exists) {
		hash.clear();
		hash.resize(0x30);
		file.seekg(-0x30, ios::end);
		file.read(reinterpret_cast<char*>(hash.data()), 0x30);
	}

	return ErrorCode::OK;
}


int ATRAC::extract(string path, uint32_t frequency, string name) {
	if (name == "") {
		if (names_vec.size() < 2) {
			name = "unknown.wav";
		}
		else {
			name = names_vec[1] + ".wav";
			if (names_vec[0] != names_vec[1]) {
				path += '/' + names_vec[0];
				cout << "Creating folder " << path << '\n';
				if (_mkdir(path.c_str()) != 0) {
					cout << "Can't create folder " << path << '\n';
				}
			}
		}
	}

	fstream file(path + '/' + name, ios::trunc | ios::out | ios::in | ios::binary);
	if (!file)
	{
		cout << "Can't extract to " << path << '/' << name << '\n';
		file.clear();
		file.close();
		return ErrorCode::FileCantCreate;
	}

	file.write(reinterpret_cast<char*>(data.data()), data.size());
	file.seekp(0x18);

	//By default extract file with playback frequency
	if (frequency == 0) {
		frequency = WAVE_block.playback_frequency;
	}

	file.write(reinterpret_cast<char*>(&frequency), sizeof(uint32_t));
	file.close();
	return ErrorCode::OK;
}


int ATRAC::setData(std::vector<uint8_t> v_data) {
	int return_val = ErrorCode::OK;
	data = v_data;
	uint32_t samples = 0;
	for (int i = 0; i < 0x200; i+=4) {
		memcpy(&samples, &v_data[i], 4);
		//"fact"
		if (samples == 0x74636166) {
			memcpy(&samples, &v_data[i+8], 4);
			break;
		}
		samples = 0;
	}
	if (samples != 0) {
		WAVE_block.total_samples = samples;
	}
	else {
		return_val = ErrorCode::DataIncorrect;
	}
	updateOffsets();
	return return_val;
}


int ATRAC::save(fstream& file) {
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
	file.write(reinterpret_cast<char*>(&WAVE_block.sgd_data_size), sizeof(uint32_t));

	while (file.tellp() % 0x10 != 0)
		file.put(0);

	//NAME
	file.seekp(WAVE_offs + WAVE_size + 0x8);
	uint32_t NAME_offs = file.tellp();
	file.write("NAME", sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&NAME_size), sizeof(uint32_t));
	for (int i = 0; i < 4; i++)
		file.put(0);
	file.write(reinterpret_cast<char*>(&names_amount), sizeof(uint32_t));

	//DON'T FORGET TO SORT NAME BLOCKS BY ADDRESSES [done, but includes <algorhithm> only for this]
	std::vector<NAME_block_s>* NAME_block_temp = new std::vector<NAME_block_s>;
	*NAME_block_temp = NAME_block;
	std::sort(NAME_block_temp->begin(), NAME_block_temp->end());

	file.write(reinterpret_cast<char*>(NAME_block_temp->data()), sizeof(NAME_block_s) * NAME_block_temp->size());
	//file.write(reinterpret_cast<char*>(&NAME_block[0]), sizeof(NAME_block_s) * uint32_t(NAME_block.size()));

	delete NAME_block_temp;


	for (uint32_t i = 0; i < names_vec.size(); i++) {
		file.write(names_vec[i].c_str(), names_vec[i].size());
		file.put(0);
	}
	while (file.tellp() % 0x10 != 0)
		file.put(0);

	//Data block
	file.write(reinterpret_cast<char*>(data.data()), data.size());

	//Hash
	if (hash_exists) {
		while (file.tellp() % 0x10 != 0)
			file.put(0);
		if (hash.size() < 0x30) {
			file.write((char*)default_hash, 0x30);
		}
		else {
			file.write(reinterpret_cast<char*>(hash.data()), 0x30);
		}
		file.seekp(-0x28, ios::end);
		uint32_t hash_offset = uint32_t(file.tellp()) - 8;
		file.write(reinterpret_cast<char*>(&hash_offset), sizeof(uint32_t));
	}
	//ADD LOAD FUNCTION

	return ErrorCode::OK;
}


void ATRAC::updateOffsets() {
	int temp_offset;

	//WAVE
	//null bytes and files amount + 0x38 * files, then residual bytes
	WAVE_size = 8 + 0x38 * files_amount;
	while ((WAVE_size - 8) % 0x10 != 0)
		WAVE_size++;

	//NAME
	//null bytes and names amount + 0x8 * names, then residual bytes
	NAME_size = 8 + 8 * names_amount;
	//first name in the table offset
	temp_offset = 0x10 + 8 + WAVE_size + 8 + NAME_size;
	for (uint32_t i = 0; i < names_amount; i++) {
		NAME_block[i].name_offset = temp_offset;
		temp_offset += names_vec[i].size() + 1;
		//+1 for null byte
		NAME_size += names_vec[i].size() + 1;
	}
	while ((NAME_size - 8) % 0x10 != 0)
		NAME_size++;
	data_offset = 0x10 + 8 + WAVE_size + 8 + NAME_size;
	WAVE_block.data_size = data.size();
	WAVE_block.sgd_data_size = WAVE_block.data_size;

	//Hash
	if (hash_exists) {
		while (WAVE_block.sgd_data_size % 0x10 != 0)
			WAVE_block.sgd_data_size++;
	}
}


void ATRAC::addName(string s, bool is_container_name) {
	names_amount++;
	names_vec.push_back(s);
	NAME_block_s tmp_block;

	if (is_container_name) {
		tmp_block.name_type = 0;
		tmp_block.file_index = 0;
	}
	else {
		tmp_block.name_type = 0x3000;
		tmp_block.file_index = names_amount - 1;
	}
	//DON'T TOUCH, IT WORKS. JUST SWAP TO RELEASE/DEBUG AND BACKWARDS, INTELLISENSE ERROR WILL DISAPPEAR
	NAME_block.push_back(tmp_block);
	updateOffsets();
}


void ATRAC::editName(int index, string ass) {
	names_vec[index] = ass;
	updateOffsets();
}


