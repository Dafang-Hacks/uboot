/* system/core/gpttool/gpttool.c
**
** Copyright 2011, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <u-boot/zlib.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#ifdef _WIN32
typedef unsigned __int64 u64;
#else
typedef unsigned long long u64;
#endif

#ifdef _WIN32
#define _ATTR_PACKED
#else
#define _ATTR_PACKED   __attribute__((packed))
#endif

typedef struct partition {
        u8 boot_ind;         /* 0x80 - active */
        u8 head;             /* starting head */
        u8 sector;           /* starting sector */
        u8 cyl;              /* starting cylinder */
        u8 sys_ind;          /* What partition type */
        u8 end_head;         /* end head */
        u8 end_sector;       /* end sector */
        u8 end_cyl;          /* end cylinder */
        u32 start_sect;      /* starting sector counting from 0 */
        u32 nr_sects;        /* number of sectors in partition */
} _ATTR_PACKED;

typedef struct _legacy_mbr {
	u8 boot_code[440];
	u32 unique_mbr_signature;
	u16 unknown;
	struct partition partition_record[4];
	u16 signature;
} _ATTR_PACKED legacy_mbr;

typedef struct _custom_mbr {
	u8 boot_code[432];
	u32 custom_signature;
	u32 gpt_header_lba;  // gpt header position
	u32 unique_mbr_signature;
	u16 unknown;
	struct partition partition_record[4];
	u16 signature;
} _ATTR_PACKED custom_mbr;

#define MAKE_SIGNATURE32( a, b, c, d ) \
        (((u32)(u8)(d)       ) | \
        ( (u32)(u8)(c) << 8  ) | \
        ( (u32)(u8)(b) << 16 ) | \
        ( (u32)(u8)(a) << 24 ) )

#define INGENIC_SIGNATURE  (MAKE_SIGNATURE32('I','N','G','E'))


const u8 partition_type_uuid[16] = {
	0xa2, 0xa0, 0xd0, 0xeb, 0xe5, 0xb9, 0x33, 0x44,
	0x87, 0xc0, 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xc7,
};

#define GPT_PRIMARY_PARTITION_TABLE_LBA 1

#define EFI_VERSION 0x00010000
#define EFI_MAGIC "EFI PART"
#define EFI_ENTRIES 128
#define EFI_NAMELEN 36

struct efi_header {
	u8 magic[8];

	u32 version;
	u32 header_sz;

	u32 crc32;
	u32 reserved;

	u64 header_lba;
	u64 backup_lba;
	u64 first_lba;
	u64 last_lba;

	u8 volume_uuid[16];

	u64 entries_lba;

	u32 entries_count;
	u32 entries_size;
	u32 entries_crc32;
} _ATTR_PACKED;

struct efi_entry {
	u8 type_uuid[16];
	u8 uniq_uuid[16];
	u64 first_lba;
	u64 last_lba;
	u64 attr;
	u16 name[EFI_NAMELEN];
};

struct ptable {
	u8 mbr[512];
	union {
		struct efi_header header;
		u8 block[512];
	};
	struct efi_entry entry[EFI_ENTRIES];
};

typedef struct _item {
	char * name;
	char * value;
} item_t;

typedef struct _group {
	char * name;
	item_t items[128];
} group_t;

static u8 file_buffer[2048] = { 0 };
static group_t group_list[10] = { 0 };

u64 parse_size(char *sz);

char * str_trim(char * str) {
	char * p;
	p = str + strlen(str) - 1;
	while (*p && (*p <= ' ')) {
		p--;
	}
	*(++p) = 0;

	p = str;
	while (*p && (*p <= ' ')) {
		p++;
	}
	return p;
}

int parse_config_file(char * buffer)
{
	char * line = buffer;
	int is_last_line = 0;
	int group_index = -1;
	int item_index = -1;
	char * p;

	while (line && *line && !is_last_line) {
		// get next line
		char * next_line = strchr(line, '\n');
		if (next_line) {
			*next_line = 0;
		} else {
			is_last_line = 1;
		}

		if (p = strchr(line, '#')) {
			*p = 0;
		}

		if (p = strchr(line, ':')) { //find group name
			char * group_name;
			*p = 0;
			group_name = str_trim(line);
			if (group_name[0] != 0) {
				group_index ++;
				item_index = -1;
				if (group_index >= (sizeof(group_list)/sizeof(group_list[0]))) {
					return -1;
				}
				group_list[group_index].name = group_name;
			}
		} else if (p = strchr(line, '=')) { //find property name
			char * property_name;
			*(p++) = 0;
			property_name = str_trim(line);
			if ((property_name[0] != 0) && (group_index >= 0)) {
				item_index ++;
				if (item_index >= (sizeof(group_list[0].items)/sizeof(group_list[0].items[0]))) {
					return -1;
				}
				group_list[group_index].items[item_index].name = property_name;
				group_list[group_index].items[item_index].value = str_trim(p);
			}
		}

		if (next_line == NULL) break;

		line = next_line + 1;
	}

	return 0;
}

group_t * get_group(char * group_name) {
	int i;
	for(i=0; i<sizeof(group_list)/sizeof(group_list[0]); i++) {
		group_t * group = &group_list[i];

		if (group->name == NULL) continue;

		if (strcmp(group->name, group_name) == 0) {
			return group;
		}
	}
	return NULL;
}

item_t * get_item(char * group_name, char * item_name) {
	int i;
	group_t * group = get_group(group_name);

	if (group == NULL) return NULL;

	for(i=0; i<sizeof(group_list[0].items)/sizeof(group_list[0].items[0]); i++) {
		item_t * item = &(group->items[i]);

		if (item->name == NULL) continue;

		if (strcmp(item->name, item_name) == 0) {
			return item;
		}
	}

	return NULL;
}

char * get_item_value(char * group_name, char * item_name) {
	item_t * item = get_item(group_name, item_name);

	return (item == NULL) ? NULL : item->value;
}

/*int parse_partition_item(char * value, u64 * start, u64* size) {
	char temp[128];
	char * str_start;
	char * str_size;

	strncpy(temp, value, sizeof(temp));

	str_size = strchr(temp, ',');
	if (str_size == NULL) return -1;
	*str_size++ = 0;

	str_start = str_trim(temp);
	if (*str_start == 0) return -1;
	if (!isdigit(*str_start)) return -1;

	str_size = str_trim(str_size);
	if (*str_size == 0) return -1;
	if (!isdigit(*str_size)) return -1;

	*start = parse_size(str_start);
	*size = parse_size(str_size);

	return 0;
}*/

int parse_partition_item(char * value, u64 * start, u64* size) {
    char temp[128];
    char * str_start;
    char * str_size;
    char * str_type;
    char * sub_str;

    strncpy(temp, value, sizeof(temp));
    sub_str = strchr(temp, ',');
    if (sub_str == NULL) return -1;
    *sub_str++ = 0;

    //To gain start field
    str_start = str_trim(temp);
    if (*str_start == 0) return -1;
    if (!isdigit(*str_start)) return -1;

    //To gain size field
    str_size = str_trim(sub_str);
    if (*str_size == 0) return -1;
    if (!isdigit(*str_size)) return -1;

    //To gain fstype field
    sub_str = strchr(sub_str, ',');
    if (sub_str == NULL) return -1;
    *sub_str++ = 0;
    str_type = str_trim(sub_str);
    if (*str_type == 0) return -1;

    *start = parse_size(str_start);
    *size = parse_size(str_size);
    //*type = transfer_fstype(str_type);
    return 0;
}



void get_uuid(u8 *uuid)
{
#ifdef _WIN32
	//*uuid = ;
#else
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, uuid, 16);
	close(fd);
#endif
}

void init_mbr(u8 *mbr, u32 blocks, u32 gpt_header_lba, u32 custom_signature)
{
	custom_mbr * temp_mbr;

	mbr[0x1be] = 0x00; // nonbootable
	mbr[0x1bf] = 0xFF; // bogus CHS
	mbr[0x1c0] = 0xFF;
	mbr[0x1c1] = 0xFF;

	mbr[0x1c2] = 0xEE; // GPT partition
	mbr[0x1c3] = 0xFF; // bogus CHS
	mbr[0x1c4] = 0xFF;
	mbr[0x1c5] = 0xFF;

	mbr[0x1c6] = 0x01; // start
	mbr[0x1c7] = 0x00;
	mbr[0x1c8] = 0x00;
	mbr[0x1c9] = 0x00;

	memcpy(mbr + 0x1ca, &blocks, sizeof(u32));

	mbr[0x1fe] = 0x55;
	mbr[0x1ff] = 0xaa;
	if(!custom_signature) {
		temp_mbr = (custom_mbr*)mbr;
		temp_mbr->custom_signature = INGENIC_SIGNATURE;
		temp_mbr->gpt_header_lba = gpt_header_lba;
	}
}

int add_ptn(struct ptable *ptbl, u64 first, u64 last, const char *name)
{
	struct efi_header *hdr = &ptbl->header;
	struct efi_entry *entry = ptbl->entry;
	unsigned n;

	if (first < 34) {
		fprintf(stderr,"partition '%s' overlaps partition table, first=%lld\n", name, first);
		return -1;
	}

	if (last > hdr->last_lba) {
		fprintf(stderr,"partition '%s' does not fit on disk\n", name);
		return -1;
	}
	for (n = 0; n < EFI_ENTRIES; n++, entry++) {
		if (entry->type_uuid[0])
			continue;
		memcpy(entry->type_uuid, partition_type_uuid, 16);
		get_uuid(entry->uniq_uuid);
		entry->first_lba = first;
		entry->last_lba = last;
		for (n = 0; (n < EFI_NAMELEN) && *name; n++)
			entry->name[n] = *name++;
		return 0;
	}
	fprintf(stderr,"out of partition table entries\n");
	return -1;
 }

int usage(void)
{
	fprintf(stderr,
		"\n"
		"usage: gpttool <partitions_config_file> <out_mbr_file> <out_gpt_file> \n"
		"\n"
		"<partitions_config_file> sample: \n"
		"    property: \n"
		"        disk_size = 7432m \n"
		"        gpt_header_lba = 2m \n"
		"    partition: \n"
		"        # name=    start,    size \n"
		"        boot=         3m,     9m \n"
		"        recovery=    12m,     9m \n"
		"        misc=        21m,     5m \n"
		"        cache=       28m,    31m \n"
		"        system=      64m,   321m \n"
		"        userdata=   385m,   967m \n"
		);
	return 0;
}

void show(struct ptable *ptbl)
{
	struct efi_entry *entry = ptbl->entry;
	unsigned n, m;
	char name[EFI_NAMELEN + 1];

	fprintf(stderr,"\nptn  start block   end block     name\n");
	fprintf(stderr,"---- ------------- ------------- --------------------\n");

	for (n = 0; n < EFI_ENTRIES; n++, entry++) {
		if (entry->type_uuid[0] == 0)
			break;
		for (m = 0; m < EFI_NAMELEN; m++) {
			name[m] = entry->name[m] & 127;
		}
		name[m] = 0;
		fprintf(stderr,"#%03d %13lld %13lld %s\n",
			n + 1, entry->first_lba, entry->last_lba, name);
	}
}

u64 parse_size(char *sz)
{
	u64 n;

	int l = strlen(sz);

#ifdef _WIN32
	n = _strtoui64(sz, 0, 10);
#else
	n = strtoull(sz, 0, 10);
#endif

	if (l) {
		switch(sz[l-1]){
		case 'k':
		case 'K':
			n *= 1024;
			break;
		case 'm':
		case 'M':
			n *= (1024 * 1024);
			break;
		case 'g':
		case 'G':
			n *= (1024 * 1024 * 1024);
			break;
		}
	}
	return n;
}

int main(int argc, char **argv)
{
	struct ptable ptbl;
	struct efi_header *hdr = &ptbl.header;
	u64 disk_size, disk_blocks;
	u64 gpt_header_lba = GPT_PRIMARY_PARTITION_TABLE_LBA;
	char * str_disk_size, * str_gpt_lba, *str_custom_signature;
	char * mbr_file_name, * gpt_file_name, * config_file_name;
	group_t * partitions;
	u32 crc, custom_signature;
	FILE* fd = NULL;

	//"usage: gpttool <partitions_config_file> <out_mbr_file> <out_gpt_file> \n"

	if (argc != 4)
		return usage();

	config_file_name = argv[1];
	mbr_file_name = argv[2];
	gpt_file_name = argv[3];

	fd = fopen(config_file_name, "r");
	if (fd == NULL) {
		fprintf(stderr, "cannot read partitions from '%s'\n", config_file_name);
		return -1;
	}
	fread(file_buffer, 1, sizeof(file_buffer), fd);
	parse_config_file(file_buffer);
	fclose(fd);
	fd = NULL;

	str_gpt_lba = get_item_value("property", "gpt_header_lba");
	if ((str_gpt_lba == NULL) || (str_gpt_lba[0] == 0)) {
		fprintf(stderr,"cannot get gpt_header_lba property\n");
		return -1;
	}
	gpt_header_lba = parse_size(str_gpt_lba) / 512;

	str_disk_size = get_item_value("property", "disk_size");
	if ((str_disk_size == NULL) || (str_disk_size[0] == 0)) {
		fprintf(stderr,"cannot get disk_size property\n");
		return -1;
	}
	disk_size = parse_size(str_disk_size);
	disk_blocks = disk_size/512;
	fprintf(stderr,"disk: size = %lld, blocks = %lld\n", disk_size, disk_blocks);

	str_custom_signature = get_item_value("property", "custom_signature");
	if ((str_custom_signature == NULL) || (str_custom_signature[0] == 0))
		custom_signature = 0;
	else
		custom_signature = (MAKE_SIGNATURE32(str_custom_signature[0], str_custom_signature[1],
						     str_custom_signature[2], str_custom_signature[3]));

	memset(&ptbl, 0, sizeof(ptbl));
	init_mbr(ptbl.mbr, disk_blocks - 1, gpt_header_lba, custom_signature);
	memcpy(hdr->magic, EFI_MAGIC, sizeof(hdr->magic));
	hdr->version = EFI_VERSION;
	hdr->header_sz = sizeof(struct efi_header);
	hdr->header_lba = gpt_header_lba;
	hdr->backup_lba = disk_blocks - 1;
	hdr->first_lba = hdr->header_lba + 33;
	hdr->last_lba = disk_blocks - 1;
	get_uuid(hdr->volume_uuid);
	hdr->entries_lba = hdr->header_lba + 1;
	hdr->entries_count = 128;
	hdr->entries_size = sizeof(struct efi_entry);

	partitions = get_group("partition");
	if (partitions != NULL) {
		int entries_count = 0;
		int item_count = sizeof(group_list[0].items)/sizeof(group_list[0].items[0]);
		int i;
		for(i = 0; i < item_count; i++) {
			u64 start = 0, size = 0;

			item_t * partition = &(partitions->items[i]);

			if ((partition->name == NULL) || (partition->value == NULL)) break;

			parse_partition_item(partition->value, &start, &size);
			start /= 512;
			size  /= 512;
			add_ptn(&ptbl, start, start + size - 1, partition->name);
			entries_count ++;
		}
		hdr->entries_count = entries_count;
	}

	crc = crc32(0, Z_NULL, 0);
	crc = crc32(crc, (void*) ptbl.entry, hdr->entries_count * hdr->entries_size);
	hdr->entries_crc32 = crc;

	crc = crc32(0, Z_NULL, 0);
	crc = crc32(crc, (void*) &ptbl.header, sizeof(ptbl.header));
	hdr->crc32 = crc;

	#define MBR_SIZE  (512)
	fd = fopen(mbr_file_name, "wb");
	if (fd == NULL) {
		fprintf(stderr,"error: cannot open '%s'\n", mbr_file_name);
		return -1;
	}
	fwrite(&ptbl, 1, MBR_SIZE, fd);
	fclose(fd);
	fd = NULL;

	fd = fopen(gpt_file_name, "wb");
	if (fd == NULL) {
		fprintf(stderr,"error: cannot open '%s'\n", gpt_file_name);
		return -1;
	}
	fwrite(((u8*)&ptbl) + MBR_SIZE, 1, sizeof(ptbl) - MBR_SIZE, fd);
	fclose(fd);
	fd = NULL;

	show(&ptbl);

	return 0;
}
