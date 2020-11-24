#include "SGXD.h"

using namespace std;

SGXD::SGXD() {}
SGXD::~SGXD() {}

//Main functions
int SGXD::load(std::string file)
{
	file_path = file;
	ifstream sgd(file, ios::binary);

	cout << "\nLoading " << file << ':';
	if (!sgd)
	{
		cout << " ERROR" << endl;
		return 1;
	}
	else {
		cout << " SUCCESS" << endl;
	}

	uint32_t value;//for reading things we don't need to store

	sgd.seekg(0);
	sgd.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
	if (value != 0x44584753U)//"SGXD"
	{
		cout << "Ivalid SGXD file!" << endl;
		return 2;
	}
	sgd.seekg(4);
	sgd.read(reinterpret_cast<char*>(&sgd_name_offset), sizeof(uint32_t));
	sgd.read(reinterpret_cast<char*>(&data_offset), sizeof(uint32_t));
	sgd.read(reinterpret_cast<char*>(&data_size), sizeof(uint32_t));
	data_size -= 0x80000000U;//Remove 0x80000000 offset from size
	sgd.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
	if (value == 0x45564157U)//"WAVE"
		type = 0;
	else if (value == 0x444E4752U)//"RGND"
		type = 1;
	else
		type = -1;//Unknown type

	//Get container name
	sgd.seekg(sgd_name_offset);
	sgd_name = ReadString(sgd);

	if (type == 0)
	{
		atrac.load();
		atrac.data_offset = data_offset;
		atrac.WAVE_block.sgd_data_size = data_size;
	}
	else if (type == 1)
	{
		adpcm.load();
		//adpcm.WAVE_block.sgd_data_size = data_size;
	}
	else
	{
		cout << "Unknown SGXD type." << endl;
		return 2;
	}
}
void SGXD::printInfo()
{
	cout << "File: " << short_path(file_path) << '\n' <<
		"Container name: " << sgd_name << '\n' <<
		"Container name offset: " << hex_int_str(sgd_name_offset) << '\n' <<
		"Data offset: " << hex_int_str(data_offset) << '\n' <<
		"Data size: " << hex_int_str(data_size) << '\n' <<
		"File size: " << hex_int_str(file_size) << '\n' <<
		"Type: ";
	switch (type)
	{
	case 0:
		cout << "ATRAC3+\n" <<
			"Files/Names amount: " << atrac.files_amount << '/' << atrac.names_amount << '\n';
		break;
	case 1:
		cout << "PSX ADPCM (VAG)\n";
		break;
	default:
		cout << "Unknown (" << type << ")\n";
	}
}
bool SGXD::save(std::string path)
{
	//Base header
	if (type == 0)
		data_size = atrac.WAVE_block.sgd_data_size;
	/*else if (type == 1)
		data_size = adpcm.WAVE_block.sgd_data_size;*/
	fstream file(path, ios::trunc | ios::out | ios::in | ios::binary);
	file.write("SGXD", sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&sgd_name_offset), sizeof(uint32_t));
	file.write(reinterpret_cast<char*>(&data_offset), sizeof(uint32_t));
	data_size += 0x80000000;
	file.write(reinterpret_cast<char*>(&data_size), sizeof(uint32_t));
	data_size -= 0x80000000;
	file.seekp(0x10);

	if (type == 0)
		atrac.save(file);
	else if (type == 1)
		adpcm.save(file);
}
//Getters
//string SGXD::getPath()
//{
//	return file_path;
//}
//string SGXD::getName()
//{
//	return sgd_name;
//}

//Important shit
void SGXD::setContainerName(string s)
{
	sgd_name = s;
	if (type == 0)
	{
		for (int i = 0; i < atrac.names_amount; i++)
		{
			if (atrac.NAME_block[i].name_type == 0)
			{
				atrac.names_vec[i] = s;
				updateOffsets();
				return;
			}
		}
		atrac.addName(s, 0);
	}
	/*else if (type == 1)
	{
		for (int i = 0; i < adpcm.names_amount; i++)
		{
			if (adpcm.NAME_block[i].name_type == 0)
			{
				adpcm.names_vec[i] = s;
				updateOffsets();
				return;
			}
		}
	}*/
}
void SGXD::updateOffsets()
{
	if (type == 0)
	{
		atrac.updateOffsets();
		data_offset = atrac.data_offset;
		for (int i = 0; i < atrac.NAME_block.size(); i++)
		{
			if (atrac.NAME_block[i].name_type == 0)
				sgd_name_offset = atrac.NAME_block[i].name_offset;
		}
		data_size = atrac.WAVE_block.sgd_data_size;
	}
}

//Various things
string ReadString(ifstream& a)
{
	string result;
	uint8_t t = a.get();
	for (int length = 0; t != 0 && length < 0xff; length++)//Protection from yeeting into space
	{
		result += t;
		t = a.get();
	}
}
string hex_int_str(uint32_t a)
{
	std::ostringstream b;
	b << "0x" << std::hex << std::uppercase << a;
	return b.str();
}
string short_path(string path)
{
	vector<string> line;
	string token;
	istringstream iss(path);
	while (getline(iss, token, '\\'))
		line.push_back(token);
	if (line.size() > 5)
		return (line[0] + '\\' + line[1] + "\\...\\" + line[line.size() - 2] + '\\' + line[line.size() - 1]);
	else
		return path;
}