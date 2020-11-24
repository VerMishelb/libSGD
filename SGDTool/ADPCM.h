#ifndef ADPCM_H
#define ADPCM_H

#include <fstream>
#include <vector>
#include <string>

class ADPCM
{
private:
	unsigned int base_frequency = 44100;
	unsigned int files_amount = 0;

public:
	ADPCM();
	~ADPCM();
	int load();
	bool save(fstream& file);
};

#endif // !ADPCM_H