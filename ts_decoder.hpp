#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>

#define errlog printf
#define msglog printf
#define dbglog printf

class ts_decoder
{
public:
	ts_decoder() {
		buffer = nullptr;
		current_pos = 0;
		total_pos_in_stream = 0;
		num_bytes_in_buffer = 0;
		stream_decode_started = false;

		memset(&current_ts_packet,0,sizeof(current_ts_packet));
	};

	~ts_decoder() {
		if (buffer) {
			free(buffer);
		}
	}

protected:

	static const int ts_packet_size = 188;

	typedef enum {
		ts_success = 0,
		ts_failure = -1,
		ts_invalid_args = -2,
		ts_eos = -3,
	} ts_error;

	typedef enum {
		tsc_invalid = -1,
		tsc_not_scrambled = 0,
		tsc_reserved = 1,
		tsc_even_key = 2,
		tsc_odd_key = 3
	} ts_tsc; // tranposrt scrambling control

	typedef enum {
		afc_reserved = 0,
		afc_only_payload = 1,    // no adaptation field, only payload
		afc_only_adaptation = 2, // only adaptation, no payload
		afc_both = 3,            // adaptation followed by payload
	} ts_afc; //adaptation field control

	typedef struct {
		int  length;    // length of adaptation field
		bool disci;     // discontinuity idicator
		bool rai;       // random access indicator
		bool espi;      // elementary stream priority indicator
		bool pcr;       // pcr flag
		bool opcr;      // origin pcr flag
		bool splice;    // splicing point flag
		bool tpd;       // transport private data
		bool ext;       // adaptation field extension flag
	} adaptation_field;

	typedef struct {
		bool   tei;     // transport error indicator
		bool   pusi;    // payload unit start indicator
		bool   tp;      // transposrt priority;
		int    pid;     // packet identifier
		ts_tsc tsc;     // transposrt scrambling control
		ts_afc afc;     // adaptation field control
		int    cc;      // continuity counter

		adaptation_field af;
	} ts_packet_header;

	typedef struct {
		int              offset;     // offset in the file
		ts_packet_header header;
		char             payload[ts_packet_size];
	} ts_packet;

	std::vector<int> search_pids;

	int init();
	int deinit();
	int get_next_ts_packet(ts_packet &packet);

	// int move_to_next_header();
	// int decode_header(ts_packet_header &header);
	// int decode_and_display_header();

	virtual int read(char *buf, int bytesToRead, int &bytesRead) =  0;
	virtual int display_header(ts_packet_header header) = 0;
	virtual int display_packet(ts_packet &packet) = 0;

private:
	static const int min_header_size = 4;
	static const char sync_byte = 0x47;
	static const int buffer_size = ts_packet_size * 1000;

	void *allocatedBuffer;
	char *buffer;
	int num_bytes_in_buffer;
	int current_pos;
	bool stream_decode_started;
	unsigned long long total_pos_in_stream;

	ts_packet current_ts_packet;

	int fill_buffer();
	int find_next_sync();
	int decode_header(ts_packet_header &header);
	int decode_adaptation_field(ts_packet_header &header);
};
