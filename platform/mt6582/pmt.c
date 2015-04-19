#include <stdint.h>
#include <printf.h>
#include <string.h>
#include <target.h>
#include <platform/errno.h>
#include <platform/partition.h>
#include <platform/pmt.h>


pt_resident lastest_part[PART_MAX_COUNT];

unsigned char page_buf[4096+128];

extern u64 g_emmc_size;

#define PMT_REGION_SIZE     (0x1000)
#define PMT_REGION_OFFSET   (0x100000)

#define PMT_VER_V1          ("1.0")
#define PMT_VER_SIZE        (4)

static int load_pt(u8 *buf, part_dev_t *dev)
{
    int ret = ERR_NO_EXIST;
    u64 pt_start; 
    u64 mpt_start; 
    int pt_size = PMT_REGION_SIZE; 
    int buffer_size = pt_size;
    //unsigned char *head = &page_buf[0];
    unsigned char *head = page_buf;
    unsigned char *tail = page_buf + pt_size - PT_SIG_SIZE;

    pt_start = g_emmc_size - PMT_REGION_OFFSET;
    mpt_start = pt_start + PMT_REGION_SIZE;

    printf("============func=%s===scan pmt from %llx=====\n", __func__, pt_start);
    /* try to find the pmt at fixed address, signature:0x50547631 */

    dev->read(dev, pt_start, (u8*)page_buf, buffer_size); 
    if (is_valid_pt(head)) {
        if (!memcmp(page_buf + PT_SIG_SIZE, PMT_VER_V1, PMT_VER_SIZE)) {
            if (is_valid_pt(tail)) {
                printf("find pt at %llx\n", pt_start); 
                memcpy(buf, page_buf + PT_SIG_SIZE + PMT_VER_SIZE, PART_MAX_COUNT * sizeof(pt_resident));
                ret = DM_ERR_OK;
                return ret;
            } else {
                printf("invalid tail pt format\n");
                ret = ERR_NO_EXIST;
            }
        } else {
            printf("invalid pt version %s\n", page_buf + PT_SIG_SIZE);
            ret = ERR_NO_EXIST;
        }
    }
    
    dev->read(dev, mpt_start, (u8*)page_buf, buffer_size); 
    if (is_valid_mpt(head)) { 
        if (!memcmp(page_buf + PT_SIG_SIZE, PMT_VER_V1, PMT_VER_SIZE)) {
            if (is_valid_mpt(tail)) {
                printf("find mpt at %llx\n", mpt_start); 
                memcpy(buf, page_buf + PT_SIG_SIZE + PMT_VER_SIZE, PART_MAX_COUNT * sizeof(pt_resident));
                ret = DM_ERR_OK;
                return ret;
            } else {
                printf("invalid tail mpt format\n");
                ret = ERR_NO_EXIST;
            }
        } else {
            printf("invalid mpt version %s\n", page_buf + PT_SIG_SIZE);
            ret = ERR_NO_EXIST;
        }
    }

	return ret;      
}

static void pmt_init(part_dev_t *dev)
{
	int ret = 0;
	int i = 0;

	printf("pmt_init\n");

	memset(&lastest_part, 0, PART_MAX_COUNT * sizeof(pt_resident));
	ret = load_pt((u8 *)&lastest_part, dev);
	if (ret == ERR_NO_EXIST) {
		printf("no pt\n");
        return;
	}

	printf("Find pt\n");
	for (i = 0; i < PART_MAX_COUNT; i++) {
		if (lastest_part[i].size == 0) {
            break;
        }
		partition_layout[i].name = lastest_part[i].name;
		partition_layout[i].startblk = lastest_part[i].offset / 512;
		partition_layout[i].blknum = lastest_part[i].size / 512; 

		printf("partition %s size %016llx %016llx\n",lastest_part[i].name,lastest_part[i].offset,lastest_part[i].size);
	}

	partition_layout[i].name = NULL;
}

void parse_partitions(part_dev_t *dev)
{
    pmt_init(dev);
}
