#include <stdint.h>
#include <printf.h>
#include <malloc.h>
#include <string.h>
#include <target.h>
#include <platform/errno.h>
#include <platform/mmc_core.h>
#include <platform/partition.h>

part_t partition_layout[PART_MAX_COUNT];

void __attribute__((weak)) parse_partitions(part_dev_t *dev)
{
    printf("no partition table exist\n");
}

void part_init(part_dev_t *dev)
{
    parse_partitions(dev);
}

static part_t tempart;

static char *get_p_name(char *name)
{
    char *pname = NULL;

    if (!strcmp(name, "SECURE")) {
        pname = "SECCFG";
    } else if (!strcmp(name, "SECSTATIC")) {
        pname = "SEC_RO";
    } else if (!strcmp(name, "SECAPANIC")) {
        pname = "EXPDB";
    } else if (!strcmp(name, "ANDSYSIMG")) {
        pname = "ANDROID";
    } else if (!strcmp(name, "USER")) {
        pname = "USRDATA";
    } else {
        pname = name;
    }

    return pname;
}

/* @name, partition table name(pname) or alias name */
part_t* mt_part_get_partition(char *name)
{
    char *pname;
    part_t *part = &partition_layout[0];

    pname = get_p_name(name);

	printf("[%s]%s->%s\n", __FUNCTION__, name, pname);
	
	while (part->name) {
		if (!strcmp(pname, part->name)) {
	        tempart.name = part->name;      // or name?
		    tempart.startblk = part->startblk;
		    tempart.blknum = part->blknum;

		    printf ("[%s] %x\n", __FUNCTION__, tempart.startblk);
		    return &tempart;
		}

		part++;
	}
    return NULL;
}

void mt_part_dump(void)
{
    part_t *part = &partition_layout[0];
    
    printf("Part Info Dump\n");
    while (part->name) {
        printf("[0x%016llx-0x%016llx] (%.8ld blocks): \"%s\"\n", 
            (u64)part->startblk * BLK_SIZE, 
            (u64)(part->startblk + part->blknum) * BLK_SIZE - 1, 
			   				part->blknum, part->name);
        part++;
    }
    printf("\n");
}

unsigned int write_partition(unsigned size, unsigned char *partition)
{
    return 0;
}

int partition_get_index(const char *name)
{
	int i;
	int loops;
	char *r_name = NULL;

	loops = sizeof(g_part_name_map) / sizeof(struct part_name_map);

	for (i = 0; i < loops; i++) {
		if (!strcmp(name, g_part_name_map[i].fb_name)) {
			r_name = g_part_name_map[i].r_name;
			break;
		}
	}

	if (!r_name)
		return -1;

	for (i = 0; i < PART_MAX_COUNT; i++) {
            /* lookup alias name needed */
		if (!strcmp(r_name, partition_layout[i].name)) {
			printf("[%s]find %s %s index %d\n", __FUNCTION__, name, r_name, i);
			return i;
		}
	}

	return -1;
}

u64 partition_get_offset(int index)
{
    if (index < 0 || index >= PART_MAX_COUNT)
        return -1;

    if (!partition_layout[index].blknum)
        return -1;

    return (u64)partition_layout[index].startblk * BLK_SIZE;
}

u64 partition_get_size(int index)
{
    if (index < 0 || index >= PART_MAX_COUNT)
        return -1;

    if (!partition_layout[index].blknum)
        return -1;

	return (u64)partition_layout[index].blknum * BLK_SIZE;
}

int partition_get_type(int index, char **p_type)
{
	int i, loops;

    if (index < 0 || index >= PART_MAX_COUNT)
        return -1;

    if (!partition_layout[index].blknum)
        return -1;

	loops = sizeof(g_part_name_map) / sizeof(struct part_name_map);

	for (i = 0;i < loops; i++) {
		if (!strcmp(partition_layout[index].name, g_part_name_map[i].r_name)) {
			*p_type = g_part_name_map[i].partition_type;
			return 0;
		}
	}
	return -1;
}

int partition_get_name(int index, char **p_name)
{
	int i, loops;

    if (index < 0 || index >= PART_MAX_COUNT)
        return -1;

    if (!partition_layout[index].blknum)
        return -1;

	loops = sizeof(g_part_name_map) / sizeof(struct part_name_map);

	for (i = 0; i < loops; i++) {
		if (!strcmp(partition_layout[index].name, g_part_name_map[i].r_name)){
			*p_name= g_part_name_map[i].fb_name;
			return 0;
		}
	}
	return -1;
}

int is_support_erase(int index)
{
	int i, loops;

    if (index < 0 || index >= PART_MAX_COUNT)
        return -1;

    if (!partition_layout[index].blknum)
        return -1;

	loops = sizeof(g_part_name_map) / sizeof(struct part_name_map);

	for (i = 0; i < loops; i++) {
		if (!strcmp(partition_layout[index].name, g_part_name_map[i].r_name)) {
			return g_part_name_map[i].is_support_erase;
		}
	}

	return -1;
}

int is_support_flash(int index)
{
	int i, loops;

    if (index < 0 || index >= PART_MAX_COUNT)
        return -1;

    if (!partition_layout[index].blknum)
        return -1;

	loops = sizeof(g_part_name_map) / sizeof(struct part_name_map);

	for (i = 0; i < loops; i++) {
		if (!strcmp(partition_layout[index].name, g_part_name_map[i].r_name)) {
			return g_part_name_map[i].is_support_dl;
		}
	}
	return -1;
}

u64 emmc_write(u64 offset, void *data, u64 size)
{
	part_dev_t *dev = mt_part_get_device();
	return (u64)dev->write(dev, data, offset, (int)size);
}

u64 emmc_read(u64 offset, void *data, u64 size)
{
	part_dev_t *dev = mt_part_get_device();
	return (u64)dev->read(dev, offset, data, (int)size);
}

int emmc_erase(u64 offset, u64 size)
{
	return mmc_do_erase(0, offset, size);
}



static part_dev_t *mt_part_dev;
static uchar *mt_part_buf;

#ifdef MTK_EMMC_SUPPORT

int mt_part_generic_read(part_dev_t *dev, u64 src, uchar *dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
		u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        printf("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) 
		return 0;

	end = src + size;
	
	part_start    = src &  ((u64)BLK_SIZE - 1);
	part_end      = end &  ((u64)BLK_SIZE - 1);
	aligned_start = src & ~((u64)BLK_SIZE - 1);
	aligned_end   = end & ~((u64)BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
        if (part_len > (u64)size) {
            part_len = size;
        }

		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf)) != 1)
			return -EIO;
		memcpy(dst, buf + part_start, part_len);
		dst += part_len;
        src += part_len;
    }
    
	aligned_start = src & ~((u64)BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;
	
	if ((blkdev->block_read(dev_id, blknr, blkcnt, (unsigned long *)(dst))) != blkcnt)
		return -EIO;

    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && src < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf)) != 1)
			return -EIO;
		memcpy(dst, buf, part_end);
	}
	return size;
}

static int mt_part_generic_write(part_dev_t *dev, uchar *src, u64 dst, int size)
{
    int dev_id = dev->id;
    uchar *buf = &mt_part_buf[0];
    block_dev_desc_t *blkdev = dev->blkdev;
		u64 end, part_start, part_end, part_len, aligned_start, aligned_end;
    ulong blknr, blkcnt;

	if (!blkdev) {
        printf("No block device registered\n");
        return -ENODEV;
	}

	if (size == 0) 
		return 0;

	end = dst + size;
	
	part_start    = dst &  ((u64)BLK_SIZE - 1);
	part_end      = end &  ((u64)BLK_SIZE - 1);
	aligned_start = dst & ~((u64)BLK_SIZE - 1);
	aligned_end   = end & ~((u64)BLK_SIZE - 1);
 
	if (part_start) {
	    blknr = aligned_start >> BLK_BITS;	
		part_len = BLK_SIZE - part_start;
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf)) != 1)
			return -EIO;
		memcpy(buf + part_start, src, part_len);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf)) != 1)
        	return -EIO;
		dst += part_len;
        src += part_len;
    }
    
	aligned_start = dst & ~((u64)BLK_SIZE - 1);
	blknr  = aligned_start >> BLK_BITS;
	blkcnt = (aligned_end - aligned_start) >> BLK_BITS;

	if ((blkdev->block_write(dev_id, blknr, blkcnt, (unsigned long *)(src))) != blkcnt)
		return -EIO;
    
    src += (blkcnt << BLK_BITS);
    dst += (blkcnt << BLK_BITS);

	if (part_end && dst < end) {
	    blknr = aligned_end >> BLK_BITS;	
		if ((blkdev->block_read(dev_id, blknr, 1, (unsigned long*)buf)) != 1) {
			return -EIO;
		}
		memcpy(buf, src, part_end);
    	if ((blkdev->block_write(dev_id, blknr, 1, (unsigned long*)buf)) != 1) {
            return -EIO;
    	}
	}
	return size;
}

#else

#define mt_part_generic_read NULL
#define mt_part_generic_write NULL

#endif

int mt_part_register_device(part_dev_t *dev)
{
	printf("[mt_part_register_device]\n");
   	if (!mt_part_dev) {
		if (!dev->read)
		    dev->read = mt_part_generic_read;
		if (!dev->write)
		    dev->write = mt_part_generic_write;

		mt_part_dev = dev;

		mt_part_buf = (uchar*)malloc(BLK_SIZE * 2);
		printf("[mt_part_register_device]malloc %d : %x\n", (BLK_SIZE * 2), mt_part_buf);

		part_init(dev);
    }
    return 0;
}

part_dev_t *mt_part_get_device(void)
{
    if (mt_part_dev && !mt_part_dev->init && mt_part_dev->init_dev) {
        mt_part_dev->init_dev(mt_part_dev->id);
        mt_part_dev->init = 1;
    }
    return mt_part_dev;
}
