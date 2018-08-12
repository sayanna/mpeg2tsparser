#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ts_decoder.hpp"

class ts_analyzer : public ts_decoder
{
public:
	ts_analyzer();
	ts_analyzer(char *filename);
	~ts_analyzer();

	typedef enum {
		process_manual = 0,
		process_all = 1
	} process_options;

	process_options option;

	int set_file(char *filename);
	int add_search_pid(int pid);
	
	int init();
	int deinit();
	int process(void);

	int read(char *buf, int bytes_to_read, int &bytes_read);
	int display_header(ts_packet_header header);
	int display_packet(ts_packet &packet);

private:
	static const int max_filename_len = 256;
	char filename[max_filename_len];

	FILE *fp;
};
