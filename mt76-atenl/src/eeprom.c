#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <blkid/blkid.h>

#include "atenl.h"

#define EEPROM_PART_SIZE 0xFF000
char *eeprom_file;

static int
atenl_create_file(struct atenl *an, bool flash_mode)
{
	char fname[64], buf[1024];
	ssize_t w, len, max_len, total_len = 0;
	int fd_ori, fd, ret;

	/* reserve space for pre-cal data in flash mode */
	if (flash_mode) {
		atenl_dbg("%s: init eeprom with flash / binfile mode\n", __func__);
		max_len = EEPROM_PART_SIZE;
	} else {
		atenl_dbg("%s: init eeprom with efuse / default bin mode\n", __func__);
		max_len = 0x1e00;
	}

	snprintf(fname, sizeof(fname),
		 "/sys/kernel/debug/ieee80211/phy%d/mt76/eeprom",
		 an->main_phy_idx);
	fd_ori = open(fname, O_RDONLY);
	if (fd_ori < 0)
		return -1;

	fd = open(eeprom_file, O_RDWR | O_CREAT | O_EXCL, 00644);
	if (fd < 0)
		goto out;

	while ((len = read(fd_ori, buf, sizeof(buf))) > 0) {
retry:
		w = write(fd, buf, len);
		if (w > 0) {
			total_len += len;
			continue;
		}

		if (errno == EINTR)
			goto retry;

		perror("write");
		unlink(eeprom_file);
		close(fd);
		fd = -1;
		goto out;
	}

	/* reserve space for pre-cal data in flash mode */
	len = sizeof(buf);
	memset(buf, 0, len);
	while (total_len < max_len) {
		w = write(fd, buf, len);

		if (w > 0) {
			total_len += len;
			continue;
		}

		if (errno != EINTR) {
			perror("write");
			unlink(eeprom_file);
			close(fd);
			fd = -1;
			goto out;
		}
	}


	ret = lseek(fd, 0, SEEK_SET);
	if (ret) {
		close(fd_ori);
		close(fd);
		return ret;
	}

out:
	close(fd_ori);
	return fd;
}

static bool
atenl_eeprom_file_exists(void)
{
	struct stat st;

	return stat(eeprom_file, &st) == 0;
}

static int
atenl_eeprom_init_file(struct atenl *an, bool flash_mode)
{
	int fd;

	if (!atenl_eeprom_file_exists())
		return atenl_create_file(an, flash_mode);

	fd = open(eeprom_file, O_RDWR);
	if (fd < 0)
		perror("open");

	return fd;
}

static int
atenl_eeprom_init_chip_id(struct atenl *an)
{
	an->chip_id = *(u16 *)an->eeprom_data;

	switch (an->chip_id) {
	case MT7915_DEVICE_ID:
		an->adie_id = 0x7975;
		break;
	case MT7916_EEPROM_CHIP_ID:
	case MT7916_DEVICE_ID:
	case MT7981_DEVICE_ID:
		an->adie_id = 0x7976;
		break;
	case MT7986_DEVICE_ID: {
		bool is_7975 = false;
		u32 val;
		u8 sub_id;

		atenl_reg_read(an, 0x18050000, &val);

		switch (val & 0xf) {
		case MT7975_ONE_ADIE_SINGLE_BAND:
			is_7975 = true;
			/* fallthrough */
		case MT7976_ONE_ADIE_SINGLE_BAND:
			sub_id = 0xa;
			break;
		case MT7976_ONE_ADIE_DBDC:
			sub_id = 0x7;
			break;
		case MT7975_DUAL_ADIE_DBDC:
			is_7975 = true;
			/* fallthrough */
		case MT7976_DUAL_ADIE_DBDC:
		default:
			sub_id = 0xf;
			break;
		}

		an->sub_chip_id = sub_id;
		an->adie_id = is_7975 ? 0x7975 : 0x7976;
		break;
	}
	case MT7996_DEVICE_ID:
	case MT7992_DEVICE_ID:
	case MT7990_DEVICE_ID:
		/* TODO: parse info if required */
		break;
	default:
		return -1;
	}

	return 0;
}

static void
atenl_eeprom_init_max_size(struct atenl *an)
{
	switch (an->chip_id) {
	case MT7915_DEVICE_ID:
		an->eeprom_size = 3584;
		an->eeprom_prek_offs = 0x62;
		break;
	case MT7916_EEPROM_CHIP_ID:
	case MT7916_DEVICE_ID:
	case MT7981_DEVICE_ID:
	case MT7986_DEVICE_ID:
		an->eeprom_size = 4096;
		an->eeprom_prek_offs = 0x19a;
		break;
	case MT7996_DEVICE_ID:
	case MT7992_DEVICE_ID:
	case MT7990_DEVICE_ID:
		an->eeprom_size = 7680;
		an->eeprom_prek_offs = 0x1a5;
	default:
		break;
	}
}

static void
atenl_eeprom_init_band_cap(struct atenl *an)
{
#define EAGLE_BAND_SEL(index)	MT_EE_WIFI_EAGLE_CONF##index##_BAND_SEL
	u8 *eeprom = an->eeprom_data;

	if (is_mt7915(an)) {
		u8 val = eeprom[MT_EE_WIFI_CONF];
		u8 band_sel = FIELD_GET(MT_EE_WIFI_CONF0_BAND_SEL, val);
		struct atenl_band *anb = &an->anb[0];

		/* MT7915A */
		if (band_sel == MT_EE_BAND_SEL_DEFAULT) {
			anb->valid = true;
			anb->cap = BAND_TYPE_2G_5G;
			return;
		}

		/* MT7915D */
		if (band_sel == MT_EE_BAND_SEL_2GHZ) {
			anb->valid = true;
			anb->cap = BAND_TYPE_2G;
		}

		val = eeprom[MT_EE_WIFI_CONF + 1];
		band_sel = FIELD_GET(MT_EE_WIFI_CONF0_BAND_SEL, val);
		anb++;

		if (band_sel == MT_EE_BAND_SEL_5GHZ) {
			anb->valid = true;
			anb->cap = BAND_TYPE_5G;
		}
	} else if (is_mt7916(an) || is_mt7981(an) || is_mt7986(an)) {
		struct atenl_band *anb;
		u8 val, band_sel;
		int i;

		for (i = 0; i < 2; i++) {
			val = eeprom[MT_EE_WIFI_CONF + i];
			band_sel = FIELD_GET(MT_EE_WIFI_CONF0_BAND_SEL, val);
			anb = &an->anb[i];

			anb->valid = true;
			switch (band_sel) {
			case MT_EE_BAND_SEL_2G:
				anb->cap = BAND_TYPE_2G;
				break;
			case MT_EE_BAND_SEL_5G:
				anb->cap = BAND_TYPE_5G;
				break;
			case MT_EE_BAND_SEL_6G:
				anb->cap = BAND_TYPE_6G;
				break;
			case MT_EE_BAND_SEL_5G_6G:
				anb->cap = BAND_TYPE_5G_6G;
				break;
			default:
				break;
			}
		}
	} else if (is_mt7996(an)) {
		struct atenl_band *anb;
		u8 val, band_sel;
		u8 band_sel_mask[3] = {EAGLE_BAND_SEL(0), EAGLE_BAND_SEL(1),
				       EAGLE_BAND_SEL(2)};
		int i;

		for (i = 0; i < 3; i++) {
			val = eeprom[MT_EE_WIFI_CONF + i];
			band_sel = FIELD_GET(band_sel_mask[i], val);
			anb = &an->anb[i];

			anb->valid = true;
			switch (band_sel) {
			case MT_EE_EAGLE_BAND_SEL_2GHZ:
				anb->cap = BAND_TYPE_2G;
				break;
			case MT_EE_EAGLE_BAND_SEL_5GHZ_LOW:
			case MT_EE_EAGLE_BAND_SEL_5GHZ_HIGH:
			case MT_EE_EAGLE_BAND_SEL_5GHZ:
				anb->cap = BAND_TYPE_5G;
				break;
			case MT_EE_EAGLE_BAND_SEL_6GHZ_LOW:
			case MT_EE_EAGLE_BAND_SEL_6GHZ_HIGH:
			case MT_EE_EAGLE_BAND_SEL_6GHZ:
				anb->cap = BAND_TYPE_6G;
				break;
			default:
				break;
			}
		}
	} else if (is_mt7992(an) || is_mt7990(an)) {
		struct atenl_band *anb;
		u8 val, band_sel;
		u8 band_sel_mask[2] = {EAGLE_BAND_SEL(0), EAGLE_BAND_SEL(1)};
		int i;

		for (i = 0; i < 2; i++) {
			val = eeprom[MT_EE_WIFI_CONF + i];
			band_sel = FIELD_GET(band_sel_mask[i], val);
			anb = &an->anb[i];

			anb->valid = true;
			switch (band_sel) {
			case MT_EE_EAGLE_BAND_SEL_2GHZ:
				anb->cap = BAND_TYPE_2G;
				break;
			case MT_EE_EAGLE_BAND_SEL_5GHZ_LOW:
			case MT_EE_EAGLE_BAND_SEL_5GHZ_HIGH:
			case MT_EE_EAGLE_BAND_SEL_5GHZ:
				anb->cap = BAND_TYPE_5G;
				break;
			case MT_EE_EAGLE_BAND_SEL_6GHZ_LOW:
			case MT_EE_EAGLE_BAND_SEL_6GHZ_HIGH:
			case MT_EE_EAGLE_BAND_SEL_6GHZ:
				anb->cap = BAND_TYPE_6G;
				break;
			default:
				break;
			}
		}
	}
}

static void
atenl_eeprom_init_antenna_cap(struct atenl *an)
{
	switch (an->chip_id) {
	case MT7915_DEVICE_ID:
		if (an->anb[0].cap == BAND_TYPE_2G_5G)
			an->anb[0].chainmask = 0xf;
		else {
			an->anb[0].chainmask = 0x3;
			an->anb[1].chainmask = 0xc;
		}
		break;
	case MT7916_EEPROM_CHIP_ID:
	case MT7916_DEVICE_ID:
		an->anb[0].chainmask = 0x3;
		an->anb[1].chainmask = 0x3;
		break;
	case MT7981_DEVICE_ID:
		an->anb[0].chainmask = 0x3;
		an->anb[1].chainmask = 0x7;
		break;
	case MT7986_DEVICE_ID:
		an->anb[0].chainmask = 0xf;
		an->anb[1].chainmask = 0xf;
		break;
	case MT7996_DEVICE_ID:
		/* TODO: handle 4T5R */
		an->anb[0].chainmask = 0xf;
		an->anb[1].chainmask = 0xf;
		an->anb[2].chainmask = 0xf;
		break;
	case MT7992_DEVICE_ID:
		/* TODO: handle BE7200 2i5i 5T5R */
		an->anb[0].chainmask = 0xf;
		an->anb[1].chainmask = 0xf;
		break;
	case MT7990_DEVICE_ID:
		an->anb[0].chainmask = 0x3;
		an->anb[1].chainmask = 0x7;
		break;
	default:
		break;
	}
}

int atenl_eeprom_init(struct atenl *an, u8 phy_idx)
{
	bool flash_mode;
	int eeprom_fd;
	char buf[30];

	set_band_val(an, 0, phy_idx, phy_idx);
	atenl_nl_check_flash(an);
	flash_mode = an->flash_part != NULL;

	/* Get the first main phy index for this chip.
	 * For single wiphy, phy_idx (i.e. wiphy idx) would be 0 and an->band_idx will also be 0.
	 */
	an->main_phy_idx = phy_idx - an->band_idx;
	snprintf(buf, sizeof(buf), "/tmp/atenl-eeprom-phy%u", an->main_phy_idx);
	eeprom_file = strdup(buf);

	eeprom_fd = atenl_eeprom_init_file(an, flash_mode);
	if (eeprom_fd < 0) {
		atenl_err("Failed to open eeprom file %s\n", eeprom_file);
		return -1;
	}

	an->eeprom_data = mmap(NULL, EEPROM_PART_SIZE, PROT_READ | PROT_WRITE,
			       MAP_SHARED, eeprom_fd, 0);
	if (!an->eeprom_data) {
		perror("mmap");
		close(eeprom_fd);
		return -1;
	}

	an->eeprom_fd = eeprom_fd;

	/* make sure the chip id is correct before further processing */
	if (atenl_eeprom_init_chip_id(an)) {
		atenl_err("Unknown chip id %x\n", an->chip_id);
		return -1;
	}
	atenl_eeprom_init_max_size(an);
	atenl_eeprom_init_band_cap(an);
	atenl_eeprom_init_antenna_cap(an);

	if (get_band_val(an, 1, valid))
		set_band_val(an, 1, phy_idx, phy_idx + 1);

	if (get_band_val(an, 2, valid))
		set_band_val(an, 2, phy_idx, phy_idx + 2);

	return 0;
}

void atenl_eeprom_close(struct atenl *an)
{
	msync(an->eeprom_data, EEPROM_PART_SIZE, MS_SYNC);
	munmap(an->eeprom_data, EEPROM_PART_SIZE);
	close(an->eeprom_fd);

	if (!an->cmd_mode) {
		if (remove(eeprom_file))
			perror("remove");
	}

	free(eeprom_file);
}

int atenl_eeprom_update_precal(struct atenl *an, int write_offs, int size)
{
	u32 offs = an->eeprom_prek_offs;
	u8 cal_indicator, *eeprom, *pre_cal;

	if (!an->cal)
		return 0;

	eeprom = an->eeprom_data;
	pre_cal = eeprom + an->eeprom_size;
	cal_indicator = an->cal_info[4];

	memcpy(eeprom + offs, &cal_indicator, sizeof(u8));
	memcpy(pre_cal, an->cal_info, PRE_CAL_INFO);
	pre_cal += (PRE_CAL_INFO + write_offs);

	if (an->cal)
		memcpy(pre_cal, an->cal, size);
	else
		memset(pre_cal, 0, size);

	return 0;
}

int atenl_mtd_open(struct atenl *an, int flags)
{
	char dev[128], buf[16];
	char *part_num;
	FILE *f;
	int fd;

	f = fopen("/proc/mtd", "r");
	if (!f)
		return -1;

	while (fgets(dev, sizeof(dev), f)) {
		if (!strcasestr(dev, an->flash_part))
			continue;

		part_num = strtok(dev, ":");
		if (part_num)
			break;
	}

	fclose(f);

	if (!part_num)
		return -1;

	/* mtdblockX emulates an mtd device as a block device.
	 * Use mtdblockX instead of mtdX to avoid padding & buffer handling.
	 */
	snprintf(buf, sizeof(buf), "/dev/mtdblock%s", part_num + 3);

	fd = open(buf, flags);

	return fd;
}

int atenl_mmc_open(struct atenl *an, int flags)
{
	const char *mmc_dev = "/dev/mmcblk0";
	int nparts, part_num;
	blkid_partlist plist;
	blkid_probe probe;
	int i, fd = -1;
	char buf[16];

	probe = blkid_new_probe_from_filename(mmc_dev);
	if (!probe)
		return -1;

	plist = blkid_probe_get_partitions(probe);
	if (!plist)
		goto out;

	nparts = blkid_partlist_numof_partitions(plist);
	if (!nparts)
		goto out;

	for (i = 0; i < nparts; i++) {
		blkid_partition part;
		const char *name;

		part = blkid_partlist_get_partition(plist, i);
		if (!part)
			continue;

		name = blkid_partition_get_name(part);
		if (!name)
			continue;

		if (strncasecmp(name, an->flash_part, strlen(an->flash_part)))
			continue;

		part_num = blkid_partition_get_partno(part);
		snprintf(buf, sizeof(buf), "%sp%d", mmc_dev, part_num);

		fd = open(buf, flags);
		if (fd >= 0)
			break;
	}

out:
	blkid_free_probe(probe);
	return fd;
}

void atenl_flash_write(struct atenl *an, int fd, u32 size, bool is_mtd)
{
	u32 flash_size, offs;
	int ret = 0;

	flash_size = lseek(fd, 0, SEEK_END);
	if (size > flash_size)
		goto fail;

	offs = an->flash_offset;
	ret = lseek(fd, offs, SEEK_SET);
	if (ret < 0)
		goto fail;

	ret = write(fd, an->eeprom_data, size);
	if (ret < 0)
		goto fail;

	atenl_info("write to %s partition %s offset 0x%x size 0x%x\n",
		   is_mtd ? "mtd" : "mmc", an->flash_part, offs, size);
	return;

fail:
	atenl_err("Failed to write %s: write size %d (flash size %d) ret = %d\n",
		  is_mtd ? "mtd" : "mmc", size, flash_size, ret);
}

int atenl_eeprom_write_flash(struct atenl *an)
{
	u32 size = an->eeprom_size;
	u32 precal_size, *precal_info;
	int fd;

	/* flash_offset = -1 for binfile mode */
	if (an->flash_part == NULL || !(~an->flash_offset)) {
		atenl_err("Flash partition or offset is not specified\n");
		return 0;
	}

	precal_info = (u32 *)(an->eeprom_data + size);
	precal_size = precal_info[0] + precal_info[1];

	if (precal_size)
		size += PRE_CAL_INFO + precal_size;

	fd = atenl_mtd_open(an, O_RDWR | O_SYNC);
	if (fd >= 0) {
		atenl_flash_write(an, fd, size, true);
		goto out;
	}

	fd = atenl_mmc_open(an, O_RDWR | O_SYNC);
	if (fd >= 0) {
		atenl_flash_write(an, fd, size, false);
		goto out;
	}

	atenl_err("Failed to open %s\n", an->flash_part);

out:
	close(fd);
	return 0;
}

int atenl_eeprom_clear_flash(struct atenl *an)
{
	u32 size = EEPROM_PART_SIZE;
	u32 flash_size, offs;
	int fd, ret = 0;
	u8 *buf;

	/* flash_offset = -1 for binfile mode */
	if (an->flash_part == NULL || !(~an->flash_offset)) {
		atenl_err("Flash partition or offset is not specified\n");
		return 0;
	}

	fd = atenl_mtd_open(an, O_RDWR | O_SYNC);
	if (fd >= 0)
		goto clear;

	fd = atenl_mmc_open(an, O_RDWR | O_SYNC);
	if (fd < 0)
		goto fail;

clear:
	flash_size = lseek(fd, 0, SEEK_END);
	if (size > flash_size)
		size = flash_size;

	ret = lseek(fd, 0, SEEK_SET);
	if (ret < 0)
		goto fail;

	buf = (u8 *)calloc(size, sizeof(char));
	if (!buf) {
		perror("calloc");
		goto fail;
	}

	ret = write(fd, buf, size);
	if (ret < 0)
		goto fail;

	atenl_info("clear flash size 0x%x\n", size);
	goto out;

fail:
	atenl_err("Failed to clear flash memory ret = %d\n", ret);

out:
	free(buf);
	close(fd);
	return 0;
}

/* Directly read values from driver's eeprom.
 * It's usally used to get calibrated data from driver.
 */
int atenl_eeprom_read_from_driver(struct atenl *an, u32 offset, int len)
{
	u8 *eeprom_data = an->eeprom_data + offset;
	char fname[64], buf[1024];
	int fd_ori, ret;
	ssize_t rd;

	snprintf(fname, sizeof(fname),
		 "/sys/kernel/debug/ieee80211/phy%d/mt76/eeprom",
		 an->main_phy_idx);
	fd_ori = open(fname, O_RDONLY);
	if (fd_ori < 0)
		return -1;

	ret = lseek(fd_ori, offset, SEEK_SET);
	if (ret < 0)
		goto out;

	while ((rd = read(fd_ori, buf, sizeof(buf))) > 0 && len) {
		if (len < rd) {
			memcpy(eeprom_data, buf, len);
			break;
		} else {
			memcpy(eeprom_data, buf, rd);
			eeprom_data += rd;
			len -= rd;
		}
	}

	ret = 0;
out:
	close(fd_ori);
	return ret;
}

/* Update all eeprom values to driver before writing efuse or ext eeprom */
static void
atenl_eeprom_sync_to_driver(struct atenl *an)
{
	int i;

	for (i = 0; i < an->eeprom_size; i += 16)
		atenl_nl_write_eeprom(an, i, &an->eeprom_data[i], 16);
}

void atenl_eeprom_cmd_handler(struct atenl *an, u8 phy_idx, char *cmd)
{
	an->cmd_mode = true;

	if (atenl_eeprom_init(an, phy_idx))
		return;

	if (!strncmp(cmd, "sync eeprom all", 15)) {
		atenl_eeprom_write_flash(an);
	} else if (!strncmp(cmd, "clear eeprom all", 16)) {
		atenl_eeprom_clear_flash(an);
	} else if (!strncmp(cmd, "eeprom", 6)) {
		char *s = strchr(cmd, ' ');

		if (!s) {
			atenl_err("eeprom: please type a correct command\n");
			return;
		}

		s++;
		if (!strncmp(s, "reset", 5)) {
			unlink(eeprom_file);
		} else if (!strncmp(s, "file", 4)) {
			atenl_info("%s\n", eeprom_file);
			if (an->flash_part != NULL)
				atenl_info("%s mode\n",
					   ~an->flash_offset == 0 ? "Binfile" : "Flash");
			else
				atenl_info("Efuse / Default bin mode\n");
		} else if (!strncmp(s, "set", 3)) {
			u32 offset, val;

			s = strchr(s, ' ');
			if (!s)
				return;
			s++;

			if (!sscanf(s, "%x=%x", &offset, &val) ||
			    offset > EEPROM_PART_SIZE)
				return;

			if (offset == 0 || offset == 1) {
				atenl_info("Modifying chip id is NOT allowed\n");
				return;
			}

			an->eeprom_data[offset] = val;
			atenl_info("set offset 0x%x to 0x%x\n", offset, val);
		} else if (!strncmp(s, "update buffermode", 17)) {
			atenl_eeprom_sync_to_driver(an);
			atenl_nl_update_buffer_mode(an);
		} else if (!strncmp(s, "write", 5)) {
			s = strchr(s, ' ');
			if (!s)
				return;
			s++;

			if (!strncmp(s, "flash", 5)) {
				atenl_eeprom_write_flash(an);
			} else if (!strncmp(s, "to efuse", 8)) {
				atenl_eeprom_sync_to_driver(an);
				atenl_nl_write_efuse_all(an);
			} else if (!strncmp(s, "to ext", 6)) {
				atenl_eeprom_sync_to_driver(an);
				atenl_nl_write_ext_eeprom_all(an);
			}
		} else if (!strncmp(s, "read", 4)) {
			u32 offset;

			s = strchr(s, ' ');
			if (!s)
				return;
			s++;

			if (!sscanf(s, "%x", &offset) ||
			    offset > EEPROM_PART_SIZE)
				return;

			atenl_info("val = 0x%x (%u)\n", an->eeprom_data[offset],
							an->eeprom_data[offset]);
		} else if (!strncmp(s, "precal", 6)) {
			s = strchr(s, ' ');
			if (!s)
				return;
			s++;

			if (!strncmp(s, "sync group", 10)) {
				atenl_nl_precal_sync_from_driver(an, PREK_SYNC_GROUP);
			} else if (!strncmp(s, "sync dpd 2g", 11)) {
				atenl_nl_precal_sync_from_driver(an, PREK_SYNC_DPD_2G);
			} else if (!strncmp(s, "sync dpd 5g", 11)) {
				atenl_nl_precal_sync_from_driver(an, PREK_SYNC_DPD_5G);
			} else if (!strncmp(s, "sync dpd 6g", 11)) {
				atenl_nl_precal_sync_from_driver(an, PREK_SYNC_DPD_6G);
			} else if (!strncmp(s, "group clean", 11)) {
				atenl_nl_precal_sync_from_driver(an, PREK_CLEAN_GROUP);
			} else if (!strncmp(s, "dpd clean", 9)) {
				atenl_nl_precal_sync_from_driver(an, PREK_CLEAN_DPD);
			} else if (!strncmp(s, "sync", 4)) {
				atenl_nl_precal_sync_from_driver(an, PREK_SYNC_ALL);
			}
		} else if (!strncmp(s, "ibf sync", 8)) {
			atenl_get_ibf_cal_result(an);
		} else if (!strncmp(s, "rx gain sync", 12)) {
			atenl_get_rx_gain_cal_result(an);
		} else {
			atenl_err("Unknown eeprom command: %s\n", cmd);
		}
	} else {
		atenl_err("Unknown command: %s\n", cmd);
	}
}
