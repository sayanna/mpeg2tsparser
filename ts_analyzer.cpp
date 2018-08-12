#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ts_analyzer.hpp"

using namespace std;

ts_analyzer::ts_analyzer(){
	fp = nullptr;
};

ts_analyzer::ts_analyzer(char *filename){
	set_file(filename);
};

ts_analyzer::~ts_analyzer(){
	if (fp) {
		fclose(fp);
	}
};

int ts_analyzer::init()
{
	return ts_decoder::init();
}

int ts_analyzer::deinit()
{
	return ts_decoder::deinit();
}

int ts_analyzer::display_header(ts_packet_header header)
{
	msglog(" [ts header] : tei(%d) pusi(%d) tp(%d) pid(%d) tsc(%d) afc(%d) cc(%d)", header.tei, header.pusi, header.tp,
		header.pid, header.tsc, header.afc, header.cc);

	if ((header.afc == afc_only_adaptation) || (header.afc == afc_both)) {
		adaptation_field &af = header.af;
		msglog(" len(%d) disci(%d) rai(%d) espi(%d) pcr(%d) opcr(%d) splice(%d) tpd(%d) ext(%d)\n",af.length,af.disci,
			af.rai,af.espi,af.pcr,af.opcr,af.splice,af.tpd,af.ext);
	} else {
		msglog("\n");
	}

	return ts_success;
}

int ts_analyzer::display_packet(ts_packet &packet)
{
	msglog(" [ts packet] : pos(%06d) ",packet.offset);
	display_header(packet.header);

	return ts_success;
}

int ts_analyzer::read(char *buffer, int bytes_to_read, int &bytes_read)
{
	if (!buffer) { return ts_failure; }
	if (!fp) { return ts_failure; }

	bytes_read = fread(buffer,1,bytes_to_read,fp);
	if (bytes_read != bytes_to_read) {
		if (feof(fp)) {
			msglog("%s : end of file reached for filename(%s)\n",__func__,filename);
			if (bytes_read == 0) { return ts_failure; }
		} else {
			errlog("%s : error reading from file (%s)\n",__func__,filename);
			return ts_failure;
		}
	}

	dbglog("%s : bytes_read(%d)\n",__func__,bytes_read);

	return ts_success;
}

int ts_analyzer::process(void)
{
	int num_packets_to_skip = 0;
	if (!fp) { return ts_failure; }

	do {
		ts_packet packet;
		ts_packet_header &header = packet.header;

		do {
			if (get_next_ts_packet(packet) < 0) {
				errlog("%s : get_next_ts_packet() failed\n",__func__);
				return ts_failure;
			}
		} while (num_packets_to_skip--);

		display_packet(packet);

		switch (option)
		{
			case process_manual:
			{
				msglog("%s : Enter number of packets to skip : ",__func__);
				cin >> num_packets_to_skip;
				break;
			}

			case process_all:
			{
				num_packets_to_skip = 0;
				break;
			}
		}

	} while(1);

	return ts_success;
}

int ts_analyzer::set_file(char *test_filename)
{
	int len = strlen(filename);

	if (len > max_filename_len) {
		errlog("%s : filename length(%d) greater than supported(%d)\n",__func__,len,max_filename_len);
		return ts_invalid_args;
	}

	strncpy(filename,test_filename,sizeof(filename));

	fp = fopen(filename,"rb");
	if (!fp) {
		errlog("%s : failed to open file(%s)\n",__func__,filename);
		return ts_invalid_args;
	}

	return ts_success;
}

int ts_analyzer::add_search_pid(int pid)
{
	search_pids.push_back(pid);

	return ts_success;
}
