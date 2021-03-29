#include "SGXD.h"

using namespace std;

libSGD::SGXD::SGXD() {}
libSGD::SGXD::~SGXD() {}


//Help functions
string hex_int_str(uint32_t a) {
	std::ostringstream b;
	b << "0x" << std::hex << std::uppercase << a;
	return b.str();
}


string short_path(string path) {
	vector<string> line;
	char separator = '/';
	//Windows case
	if (path.find('\\') != string::npos)
		separator = '\\';
	string token;
	istringstream iss(path);
	while (getline(iss, token, separator))
		line.push_back(token);
	if (line.size() > 5)
		return (line[0] + separator + line[1] + separator + "..." + separator + line[line.size() - 2] + separator + line[line.size() - 1]);
	else
		return path;
}

//Main functions
int libSGD::SGXD::load(std::string file) {
	file_path = file;
	ifstream sgd(file, ios::binary);

	cout << "\nLoading " << file << ':';
	if (!sgd)
	{
		cout << " ERROR\n";
		sgd.clear();
		return ErrorCode::FileCantOpen;
	}
	else {
		cout << " SUCCESS\n";
	}

	uint32_t value = 0;//for reading things we don't need to store

	//Reading SGXD block
	sgd.seekg(0);
	sgd.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
	if (value != 0x44584753U)//"SGXD"
	{
		cout << "Ivalid SGXD file!" << endl;
		return SGXD::ErrorCode::FileInvalidSGXD;
	}
	sgd.seekg(4);
	sgd.read(reinterpret_cast<char*>(&sgd_name_offset), sizeof(uint32_t));
	sgd.read(reinterpret_cast<char*>(&data_offset), sizeof(uint32_t));
	sgd.read(reinterpret_cast<char*>(&data_size), sizeof(uint32_t));
	data_size -= 0x80000000U;//Subtract 0x80000000 PSP memory offset from size
	sgd.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
	if (value == 0x45564157)//"WAVE" block
		type = SGXD::FileType::ATRACType;
	else if (value == 0x444E4752)//"RGND" block
		type = SGXD::FileType::ADPCMType;
	else
		type = SGXD::FileType::UnknownType;

	//Get container name
	sgd.seekg(sgd_name_offset);
	sgd_name = libSGD::read_string(sgd);

	if (type == SGXD::FileType::ATRACType)
	{
		atrac.data_offset = data_offset;
		atrac.load(sgd);
		if (atrac.WAVE_block.sgd_data_size != data_size) {
			std::cout << "WARNING: File size differs.\n";
		}
	}
	else if (type == SGXD::FileType::ADPCMType)
	{
		adpcm.load(sgd);
		//adpcm.WAVE_block.sgd_data_size = data_size;
	}
	else
	{
		cout << "Unknown SGXD type." << endl;
		return SGXD::ErrorCode::FileInvalidSGXD;
	}
	sgd.close();
	return ErrorCode::OK;
}


int libSGD::SGXD::create(SGXD::FileType type_p) {
	type = type_p;
	if (type_p == SGXD::FileType::ATRACType) {
		atrac.files_amount = 1;
		atrac.WAVE_block.file_format = 0x04;
		atrac.WAVE_block.channels = 2;
		atrac.WAVE_block.playback_frequency = 44100;
		atrac.WAVE_block.bitrate = 0x40;
		atrac.hash_exists = false;
	}
	else if (type_p == SGXD::FileType::ADPCMType) {
		std::cout << "ADPCM part is not finished.\n";
	}
	return 0;
}


void libSGD::SGXD::printInfo() {
	cout <<
		"File: " << short_path(file_path) << '\n' <<
		"Container name: " << sgd_name << '\n' <<
		"Container name offset: " << hex_int_str(sgd_name_offset) << '\n' <<
		"Data offset: " << hex_int_str(data_offset) << '\n' <<
		"Container data size: " << hex_int_str(data_size) << '\n' <<
		"Data type: ";
	switch (type)
	{
	case SGXD::FileType::ATRACType:
		cout <<
			"ATRAC3+\n" <<
			"Files/Names: " << atrac.files_amount << '/' << atrac.names_amount << '\n' <<
			"Hash: " << ((atrac.hash_exists == 1) ? "Yes\n" : "No\n");
		break;
	case SGXD::FileType::ADPCMType:
		cout <<
			"PSX ADPCM (VAG)\n" <<
			"Files/Names: \n" <<
			"Hash: \n";
		break;
	default:
		cout << "Unknown (" << type << ")\n";
	}
}


int libSGD::SGXD::save(std::string path) {
	//SGXD block
	if (type == 0)
		data_size = atrac.WAVE_block.sgd_data_size;
	/*else if (type == 1)
		data_size = adpcm.WAVE_block.sgd_data_size;*/
	fstream file(path, ios::trunc | ios::out | ios::in | ios::binary);
	if (!file)
	{
		cout << "Can't save " << path << '\n';
		file.clear();
		return ErrorCode::FileCantOpen;
	}
	file.write("SGXD", sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&sgd_name_offset), sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&data_offset), sizeof(uint32_t));
	data_size += 0x80000000;
	file.write(reinterpret_cast<char*>(&data_size), sizeof(uint32_t));
	data_size -= 0x80000000;
	file.seekp(0x10);

	//Other blocks depending on file type
	if (type == 0)
		atrac.save(file);
	/*else if (type == 1)
		adpcm.save(file);*/
	file.close();
	return ErrorCode::OK;
}


int libSGD::SGXD::extract(string path, uint32_t frequency, int file_index, string name) {
	//NOTE: File index -1 extracts ALL files, while 0...files_amount-1 extracts corresponding file
	if (type == ATRACType) {
		atrac.extract(path, frequency, name);
	}
	return ErrorCode::OK;
}


void libSGD::SGXD::setHash(bool val) {
	if (type == ATRACType) {
		atrac.hash_exists = val;
		updateOffsets();
	}
	else if (type == ADPCMType) {

	}
}


//Important stuff
void libSGD::SGXD::setContainerName(string s) {
	sgd_name = s;
	if (type == FileType::ATRACType) {
		atrac.setName(s, true);
		updateOffsets();
	}
	/*else if (type == 1) {
		for (int i = 0; i < adpcm.names_amount; i++) {
			if (adpcm.NAME_block[i].name_type == 0) {
				adpcm.names_vec[i] = s;
				updateOffsets();
				return;
			}
		}
	}*/
}


void libSGD::SGXD::updateOffsets() {
	if (type == 0) {
		atrac.updateOffsets();

		//Grab useful info from object
		data_offset = atrac.data_offset;
		for (uint32_t i = 0; i < atrac.NAME_block.size(); i++) {
			if (atrac.NAME_block[i].name_type == 0) {
				sgd_name_offset = atrac.NAME_block[i].name_offset;
				break;
			}
		}
		data_size = atrac.WAVE_block.sgd_data_size;
	}
}
