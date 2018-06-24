/**
 * speed_dug.c
 *
 * used to debug speed of read/write
 **/
#include <singlelist.h>

#include <os_clib.h>
#include <speed_dug.h>

static unsigned long long rd_btime = 0;
static unsigned long long wr_btime = 0;
static unsigned long long rd_sum_time = 0;
static unsigned long long wr_sum_time = 0;
static unsigned int rd_sum_bytes = 0;
static unsigned int wr_sum_bytes = 0;

void __speed_dug_begin(int mode, PageList *pl)
{
	int bytes = 0;
	PageList *pl_node;
	struct singlelist *pos;

	singlelist_for_each(pos, &pl->head) {
		pl_node = singlelist_entry(pos, PageList, head);
		bytes += pl_node->Bytes;
	}

	if (mode == NDD_READ) {
		rd_sum_bytes += bytes;
		rd_btime = ndd_get_time_nsecs();
	} else {
		wr_sum_bytes += bytes;
		wr_btime = ndd_get_time_nsecs();
	}
}

void __speed_dug_end(int mode)
{
	int times_ms, bytes_KB, KBps;
	unsigned long long etime = ndd_get_time_nsecs();

	if (mode == NDD_READ) {
		rd_sum_time += (etime - rd_btime);
		if (rd_sum_bytes >= DEBUG_TIME_BYTES) {
			times_ms = ndd_div_s64_32(rd_sum_time, 1000 * 1000);
			bytes_KB = rd_sum_bytes / 1024;
			KBps = (bytes_KB * 1000) / times_ms;
			ndd_print(NDD_DEBUG, "READ: nand_driver speed debug, %dms, %dKB, %d.%d%dMB/s\n",
				  times_ms, bytes_KB, KBps / 1024, (KBps % 1024) * 10 / 1024,
				  (((KBps % 1024) * 10) % 1024) * 10 / 1024);
			rd_sum_bytes = rd_sum_time = 0;
		}
	} else {
		wr_sum_time += (etime - wr_btime);
		if (wr_sum_bytes >= DEBUG_TIME_BYTES) {
			times_ms = ndd_div_s64_32(wr_sum_time, 1000 * 1000);
			bytes_KB = wr_sum_bytes / 1024;
			KBps = (bytes_KB * 1000) / times_ms;
			ndd_print(NDD_DEBUG, "WRITE: nand_driver speed debug, %dms, %dKB, %d.%d%dMB/s\n",
				  times_ms, bytes_KB, KBps / 1024, (KBps % 1024) * 10 / 1024,
				  (((KBps % 1024) * 10) % 1024) * 10 / 1024);
			wr_sum_bytes = wr_sum_time = 0;
		}
	}
}
