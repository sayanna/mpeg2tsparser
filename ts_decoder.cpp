#include "ts_decoder.hpp"

int ts_decoder::fill_buffer()
{
	int remaining_bytes_in_buffer = num_bytes_in_buffer - current_pos;
	memcpy(buffer,buffer+current_pos,remaining_bytes_in_buffer);
	total_pos_in_stream += (num_bytes_in_buffer - remaining_bytes_in_buffer);
	num_bytes_in_buffer = remaining_bytes_in_buffer;
	current_pos = remaining_bytes_in_buffer;

	int bytes_read = 0;
	if (read((buffer + current_pos),(buffer_size - num_bytes_in_buffer),bytes_read) < 0) {
		errlog("%s : read failed\n",__func__);
		return ts_failure;
	}

	num_bytes_in_buffer += bytes_read;
	if (bytes_read == 0) { return ts_eos; }

	return ts_success;
}

// int ts_decoder::move_to_next_header()
// {
// 	current_pos += ts_packet_size;
// 	int ret = find_next_sync();

// 	// const int data_mask = 0xff;
// 	// int pos = current_pos;
// 	// int shift = 0;
// 	// unsigned int data = 0;

// 	// dbglog("%s : buf[%d]=0x%x buf[%d]=0x%x\n",__func__,pos,buffer[pos],pos+1,buffer[pos+1]);
// 	// data = (data << shift) | (buffer[pos++] & data_mask); shift += 8;
// 	// data = (data << shift) | (buffer[pos++] & data_mask); shift += 8;
// 	// data = (data << shift) | (buffer[pos++] & data_mask); shift += 8;
// 	// data = (data << shift) | (buffer[pos++] & data_mask); shift += 8;

// 	// dbglog("%s : total_pos_in_stream(%llu) current_pos(%d) num_bytes_in_buffer(%d) data(0x%x)\n",
// 	// 	__func__,total_pos_in_stream+current_pos,current_pos,num_bytes_in_buffer,data);

// 	return ret;
// }

int ts_decoder::find_next_sync()
{
	int ret = ts_failure;

	do {
		int remaining_bytes_in_buffer = num_bytes_in_buffer - current_pos;

		if (remaining_bytes_in_buffer < min_header_size) {
			if (fill_buffer() < 0) {
				break;
			}
			continue;
		}

		if (buffer[current_pos] == sync_byte) {
			// dbglog("%s : sync_byte at %d\n",__func__,current_pos);
			ret = ts_success;
			break;
		}
	} while (1);

	return ret;
}

// int ts_decoder::get_current_ts_packet()
// {
// 	if (find_next_sync() < 0) {
// 		return ts_failure;
// 	}

// 	if (num_bytes_in_buffer < ts_packet_size) {
// 		return ts_failure;
// 	}

// }

int ts_decoder::decode_adaptation_field(ts_packet_header &header)
{
	const int byte_mask = 0xff;
	const int af_offset = 4;
	int pos = current_pos + af_offset;

	unsigned int af_len = (buffer[pos++] & byte_mask);
	// dbglog("%s : buffer[0]=%d\n",__func__,(int)buffer[0]);
	// dbglog("%s : buffer[1]=%d\n",__func__,(int)buffer[1]);
	// dbglog("%s : buffer[2]=%d\n",__func__,(int)buffer[2]);
	// dbglog("%s : buffer[3]=%d\n",__func__,(int)buffer[3]);
	// dbglog("%s : af_len(%d) buffer[4]=%d\n\n",__func__,af_len,(int)buffer[4]);
	const int disci_mask = 0x80;
	const int rai_mask   = 0x40;
	const int espi_mask  = 0x20;
	const int pcr_mask   = 0x10;
	const int opcr_mask  = 0x08;
	const int splice_mask = 0x04;
	const int tpd_mask    = 0x02;
	const int ext_mask    = 0x01;

	const int disci_shift  = 7;
	const int rai_shift    = 6;
	const int espi_shift   = 5;
	const int pcr_shift    = 4;
	const int opcr_shift   = 3;
	const int splice_shift = 2;
	const int tpd_shift    = 1;
	const int ext_shift    = 0;

	unsigned char data = (buffer[pos] & byte_mask);

	header.af.length = (af_len);
	header.af.disci  = (data & disci_mask) >> disci_shift;
	header.af.rai    = (data & rai_mask) >> rai_shift;
	header.af.espi   = (data & espi_mask) >> espi_shift;
	header.af.pcr    = (data & pcr_mask) >> pcr_shift;
	header.af.opcr   = (data & opcr_mask) >> opcr_shift;
	header.af.splice = (data & splice_mask) >> splice_shift;
	header.af.tpd    = (data & tpd_mask) >> tpd_shift;
	header.af.ext    = (data & ext_mask) >> ext_shift;

	return ts_success;
}

int ts_decoder::decode_header(ts_packet_header &header)
{
	// int rc = find_next_sync();
	// if (rc < 0) { return rc; }

	memset(&header,0,sizeof(header));

	const int tei_mask  = 0x800000;
	const int pusi_mask = 0x400000;
	const int tp_mask   = 0x200000;
	const int pid_mask  = 0x1fff00;
	const int tsc_mask  = 0xc0;
	const int afc_mask  = 0x30;
	const int cc_mask   = 0x0f;


	const int tei_shift  = 23;
	const int pusi_shift = 22;
	const int tp_shift   = 21;
	const int pid_shift  = 8;
	const int tsc_shift  = 6;
	const int afc_shift  = 4;
	const int cc_shift   = 0;

	const int byte_mask = 0xff;
	const int shift     = 8;

	int pos = current_pos;
	unsigned int data = 0;

	// dbglog("%s : buffer[0]=%d\n",__func__,(int)buffer[0]);
	// dbglog("%s : buffer[1]=%d\n",__func__,(int)buffer[1]);
	// dbglog("%s : buffer[2]=%d\n",__func__,(int)buffer[2]);
	// dbglog("%s : buffer[3]=%d\n",__func__,(int)buffer[3]);
	data = (buffer[pos++] & byte_mask);
	data = (data << shift) | (buffer[pos++] & byte_mask);
	data = (data << shift) | (buffer[pos++] & byte_mask);
	data = (data << shift) | (buffer[pos++] & byte_mask);

	// dbglog("%s : data(0x%x)\n",__func__,data);
	header.tei  = static_cast<bool>((data & tei_mask) >> tei_shift);
	header.pusi = static_cast<bool>((data & pusi_mask) >> pusi_shift);
	header.tp   = static_cast<bool>((data & tp_mask) >> tp_shift);
	header.pid  = static_cast<int>((data & pid_mask) >> pid_shift);
	header.tsc  = static_cast<ts_tsc>((data & tsc_mask) >> tsc_shift);
	header.afc  = static_cast<ts_afc>((data & afc_mask) >> afc_shift);
	header.cc   = static_cast<int>((data & cc_mask) >> cc_shift);

	if ((header.afc == afc_only_adaptation) || (header.afc == afc_both)) {
		if (num_bytes_in_buffer > min_header_size) {
			decode_adaptation_field(header);
		}
	}

	return ts_success;
}

// int ts_decoder::decode_and_display_header(void)
// {
// 	return ts_success;
// }

int ts_decoder::get_next_ts_packet(ts_packet &packet)
{
	if (stream_decode_started) { current_pos += ts_packet_size; }
	stream_decode_started = true;

	while (find_next_sync() == ts_success) {
		ts_packet_header header;
		decode_header(header);

		// for (auto e: search_pids)
		// {
		// 	if (header.pid == e) {
				packet.offset = total_pos_in_stream+current_pos;
				packet.header = header;

				current_ts_packet = packet;
				// dbglog("%s : found pid(%d)\n",__func__,e);
				return ts_success;
		// 	}
		// }
	}

	return ts_failure;
}

int ts_decoder::deinit()
{
	dbglog("%s : allocated buffer(%p)\n",__func__,(void*)buffer);
	if (allocatedBuffer) {
		free(allocatedBuffer);
	}

	return ts_success;
}

int ts_decoder::init()
{
	allocatedBuffer = malloc(buffer_size);
	buffer = static_cast<char*>(allocatedBuffer);
	if (!buffer) {
		errlog("%s : memory allocation for buffer of size(%d) failed\n",__func__,buffer_size);
		return ts_failure;
	}

	dbglog("%s : allocated buffer(%p)\n",__func__,(void*)buffer);

	return ts_success;
}
