/* Copyright (C) 2021-2022 Mediatek Inc. */
#include "atenl.h"

#define CHAN(_ch, _freq, _ch_80, _ch_160, ...) {	\
	.ch = _ch,				\
	.freq = _freq,				\
	.ch_80 = _ch_80,			\
	.ch_160 = _ch_160,			\
	__VA_ARGS__				\
}

struct atenl_channel {
	/* ctrl ch */
	u16 ch;
	u16 freq;
	/* center ch */
	u16 ch_80;
	u16 ch_160;
	/* only use for channels that don't have 80 but has 40 */
	u16 ch_40;
};

static const struct atenl_channel atenl_channels_5ghz[] = {
	CHAN(8, 5040, 0, 0),
	CHAN(12, 5060, 0, 0),
	CHAN(16, 5080, 0, 0),

	CHAN(36, 5180, 42, 50),
	CHAN(40, 5200, 42, 50),
	CHAN(44, 5220, 42, 50),
	CHAN(48, 5240, 42, 50),

	CHAN(52, 5260, 58, 50),
	CHAN(56, 5280, 58, 50),
	CHAN(60, 5300, 58, 50),
	CHAN(64, 5320, 58, 50),

	CHAN(68, 5340, 0, 0),
	CHAN(80, 5400, 0, 0),
	CHAN(84, 5420, 0, 0),
	CHAN(88, 5440, 0, 0),
	CHAN(92, 5460, 0, 0),
	CHAN(96, 5480, 0, 0),

	CHAN(100, 5500, 106, 114),
	CHAN(104, 5520, 106, 114),
	CHAN(108, 5540, 106, 114),
	CHAN(112, 5560, 106, 114),
	CHAN(116, 5580, 122, 114),
	CHAN(120, 5600, 122, 114),
	CHAN(124, 5620, 122, 114),
	CHAN(128, 5640, 122, 114),

	CHAN(132, 5660, 138, 0),
	CHAN(136, 5680, 138, 0),
	CHAN(140, 5700, 138, 0),
	CHAN(144, 5720, 138, 0),

	CHAN(149, 5745, 155, 0),
	CHAN(153, 5765, 155, 0),
	CHAN(157, 5785, 155, 0),
	CHAN(161, 5805, 155, 0),
	CHAN(165, 5825, 0, 0, .ch_40 = 167),
	CHAN(169, 5845, 0, 0, .ch_40 = 167),
	CHAN(173, 5865, 0, 0),

	CHAN(184, 4920, 0, 0),
	CHAN(188, 4940, 0, 0),
	CHAN(192, 4960, 0, 0),
	CHAN(196, 4980, 0, 0),
};

static const struct atenl_channel atenl_channels_6ghz[] = {
	/* UNII-5 */
	CHAN(1, 5955, 7, 15),
	CHAN(5, 5975, 7, 15),
	CHAN(9, 5995, 7, 15),
	CHAN(13, 6015, 7, 15),
	CHAN(17, 6035, 23, 15),
	CHAN(21, 6055, 23, 15),
	CHAN(25, 6075, 23, 15),
	CHAN(29, 6095, 23, 15),
	CHAN(33, 6115, 39, 47),
	CHAN(37, 6135, 39, 47),
	CHAN(41, 6155, 39, 47),
	CHAN(45, 6175, 39, 47),
	CHAN(49, 6195, 55, 47),
	CHAN(53, 6215, 55, 47),
	CHAN(57, 6235, 55, 47),
	CHAN(61, 6255, 55, 47),
	CHAN(65, 6275, 71, 79),
	CHAN(69, 6295, 71, 79),
	CHAN(73, 6315, 71, 79),
	CHAN(77, 6335, 71, 79),
	CHAN(81, 6355, 87, 79),
	CHAN(85, 6375, 87, 79),
	CHAN(89, 6395, 87, 79),
	CHAN(93, 6415, 87, 79),
	/* UNII-6 */
	CHAN(97, 6435, 103, 111),
	CHAN(101, 6455, 103, 111),
	CHAN(105, 6475, 103, 111),
	CHAN(109, 6495, 103, 111),
	CHAN(113, 6515, 119, 111),
	CHAN(117, 6535, 119, 111),
	/* UNII-7 */
	CHAN(121, 6555, 119, 111),
	CHAN(125, 6575, 119, 111),
	CHAN(129, 6595, 135, 143),
	CHAN(133, 6615, 135, 143),
	CHAN(137, 6635, 135, 143),
	CHAN(141, 6655, 135, 143),
	CHAN(145, 6675, 151, 143),
	CHAN(149, 6695, 151, 143),
	CHAN(153, 6715, 151, 143),
	CHAN(157, 6735, 151, 143),
	CHAN(161, 6755, 167, 175),
	CHAN(165, 6775, 167, 175),
	CHAN(169, 6795, 167, 175),
	CHAN(173, 6815, 167, 175),
	CHAN(177, 6835, 183, 175),
	CHAN(181, 6855, 183, 175),
	CHAN(185, 6875, 183, 175),
	/* UNII-8 */
	CHAN(189, 6895, 183, 175),
	CHAN(193, 6915, 199, 207),
	CHAN(197, 6935, 199, 207),
	CHAN(201, 6955, 199, 207),
	CHAN(205, 6975, 199, 207),
	CHAN(209, 6995, 215, 207),
	CHAN(213, 7015, 215, 207),
	CHAN(217, 7035, 215, 207),
	CHAN(221, 7055, 215, 207),
	CHAN(225, 7075, 0, 0, .ch_40 = 227),
	CHAN(229, 7095, 0, 0, .ch_40 = 227),
	CHAN(233, 7115, 0, 0),
};

static int
atenl_hqa_adapter(struct atenl *an, struct atenl_data *data)
{
	char cmd[64];
	u8 i;

	for (i = 0; i < MAX_BAND_NUM; i++) {
		u8 phy = get_band_val(an, i, phy_idx);

		if (!get_band_val(an, i, valid))
			continue;

		if (data->cmd == HQA_CMD_OPEN_ADAPTER) {
			sprintf(cmd, "iw phy phy%u interface add mon%u type monitor", phy, phy);
			system(cmd);
			sprintf(cmd, "iw dev wlan%u del", phy);
			system(cmd);
			sprintf(cmd, "ifconfig mon%u up", phy);
			system(cmd);
			/* set a special-defined country */
			sprintf(cmd, "iw reg set VV");
			system(cmd);
			atenl_nl_set_state(an, i, MT76_TM_STATE_IDLE);
		} else {
			atenl_nl_set_state(an, i, MT76_TM_STATE_OFF);
			sprintf(cmd, "iw reg set 00");
			system(cmd);
			sprintf(cmd, "iw dev mon%u del", phy);
			system(cmd);
			sprintf(cmd, "iw phy phy%u interface add wlan%u type managed", phy, phy);
			system(cmd);
		}
	}

	return 0;
}

static int
atenl_hqa_set_rf_mode(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);

	/* The testmode rf mode change applies to all bands */
	set_band_val(an, 0, rf_mode, ntohl(*(u32 *)hdr->data));
	set_band_val(an, 1, rf_mode, ntohl(*(u32 *)hdr->data));

	return 0;
}

static int
atenl_hqa_get_chip_id(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);

	*(u32 *)(hdr->data + 2) = htonl(an->chip_id);

	return 0;
}

static int
atenl_hqa_get_sub_chip_id(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);

	if (is_mt7986(an)) {
		u32 sub_id, val;
		int ret;

		ret = atenl_reg_read(an, 0x18050000, &val);
		if (ret)
			return ret;

		switch (val & 0xf) {
		case MT7975_ONE_ADIE_SINGLE_BAND:
		case MT7976_ONE_ADIE_SINGLE_BAND:
			sub_id = htonl(0xa);
			break;
		case MT7976_ONE_ADIE_DBDC:
			sub_id = htonl(0x7);
			break;
		case MT7975_DUAL_ADIE_DBDC:
		case MT7976_DUAL_ADIE_DBDC:
		default:
			sub_id = htonl(0xf);
			break;
		}

		memcpy(hdr->data + 2, &sub_id, 4);
	}

	return 0;
}

static int
atenl_hqa_get_rf_cap(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u32 band = ntohl(*(u32 *)hdr->data);
	struct atenl_band *anb;

	if (band >= MAX_BAND_NUM)
		return -EINVAL;

	anb = &an->anb[band];
	/* fill tx and rx ant */
	*(u32 *)(hdr->data + 2) = htonl(__builtin_popcount(anb->chainmask));
	*(u32 *)(hdr->data + 2 + 4) = *(u32 *)(hdr->data + 2);

	return 0;
}

static int
atenl_hqa_reset_counter(struct atenl *an, struct atenl_data *data)
{
	struct atenl_band *anb = &an->anb[an->cur_band];

	anb->reset_tx_cnt = true;
	anb->reset_rx_cnt = true;

	memset(&anb->rx_stat, 0, sizeof(anb->rx_stat));

	return 0;
}

static int
atenl_hqa_mac_bbp_reg(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	enum atenl_cmd cmd = data->cmd;
	u32 *v = (u32 *)hdr->data;
	u32 offset = ntohl(v[0]);
	int ret;

	if (cmd == HQA_CMD_READ_MAC_BBP_REG) {
		u16 num = ntohs(*(u16 *)(hdr->data + 4));
		u32 *ptr = (u32 *)(hdr->data + 2);
		u32 res;
		int i;

		if (num > SHRT_MAX) {
			ret = -EINVAL;
			goto out;
		}

		hdr->len = htons(2 + num * 4);
		for (i = 0; i < num && i < sizeof(hdr->data) / 4; i++) {
			ret = atenl_reg_read(an, offset + i * 4, &res);
			if (ret)
				goto out;

			res = htonl(res);
			memcpy(ptr + i, &res, 4);
		}
	} else {
		u32 val = ntohl(v[1]);

		ret = atenl_reg_write(an, offset, val);
		if (ret)
			goto out;
	}

	ret = 0;
out:
	memset(hdr->data, 0, 2);

	return ret;
}

static int
atenl_hqa_eeprom_bulk(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	enum atenl_cmd cmd = data->cmd;

	if (cmd == HQA_CMD_WRITE_BUFFER_DONE) {
		u32 buf_mode = ntohl(*(u32 *)hdr->data);

		switch (buf_mode) {
		case E2P_EFUSE_MODE:
			atenl_nl_write_efuse_all(an);
			break;
		default:
			break;
		}
	} else {
		u16 *v = (u16 *)hdr->data;
		u16 offset = ntohs(v[0]), len = ntohs(v[1]);
		u16 val;
		size_t i;

		if (offset >= an->eeprom_size || (len > sizeof(hdr->data) - 2))
			return -EINVAL;

		if (cmd == HQA_CMD_READ_EEPROM_BULK) {
			hdr->len = htons(len + 2);
			for (i = 0; i < len; i += 2) {
				if (offset + i >= an->eeprom_size)
					val = 0;
				else
					val = ntohs(*(u16 *)(an->eeprom_data + offset + i));
				*(u16 *)(hdr->data + 2 + i) = val;
			}
		} else { /* write eeprom */
			for (i = 0; i < DIV_ROUND_UP(len, 2); i++) {
				val = ntohs(v[i + 2]);
				memcpy(&an->eeprom_data[offset + i * 2], &val, 2);
			}
		}
	}

	return 0;
}

static int
atenl_hqa_get_efuse_free_block(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u32 free_block = htonl(0x14);

	/* TODO */
	*(u32 *)(hdr->data + 2) = free_block;

	return 0;
}

static int
atenl_hqa_get_band(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u32 band = ntohl(*(u32 *)hdr->data);

	if (band >= MAX_BAND_NUM)
		return -EINVAL;

	*(u32 *)(hdr->data + 2) = htonl(an->anb[band].cap);

	return 0;
}

static int
atenl_hqa_get_tx_power(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u32 tx_power = htonl(28);

	memcpy(hdr->data + 6, &tx_power, 4);

	return 0;
}

static int
atenl_hqa_get_freq_offset(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u32 freq_offset = htonl(10);

	/* TODO */
	memcpy(hdr->data + 2, &freq_offset, 4);

	return 0;
}

static int
atenl_hqa_get_cfg(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u32 val = htonl(1);

	/* TODO */
	memcpy(hdr->data + 2, &val, 4);

	return 0;
}

static int
atenl_hqa_read_temperature(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	char buf[64], *str;
	int fd, ret;
	u32 temp;
	u8 phy_idx = get_band_val(an, an->cur_band, phy_idx);

	ret = snprintf(buf, sizeof(buf),
		       "/sys/class/ieee80211/phy%u/hwmon%u/temp1_input",
		       phy_idx, phy_idx);
	if (snprintf_error(sizeof(buf), ret))
		return -1;

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return fd;

	ret = read(fd, buf, sizeof(buf) - 1);
	if (ret < 0)
		goto out;
	buf[ret] = 0;

	str = strchr(buf, ':');
	str += 2;
	temp = strtol(str, NULL, 10);
	/* unit conversion */
	*(u32 *)(hdr->data + 2) = htonl(temp / 1000);

	ret = 0;
out:
	close(fd);

	return ret;
}

static int
atenl_hqa_skip(struct atenl *an, struct atenl_data *data)
{
	return 0;
}

static int
atenl_hqa_check_efuse_mode(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	bool flash_mode = an->mtd_part != NULL;
	enum atenl_cmd cmd = data->cmd;
	u32 mode;

	switch (cmd) {
	case HQA_CMD_CHECK_EFUSE_MODE:
		mode = flash_mode ? 0 : 1;
		break;
	case HQA_CMD_CHECK_EFUSE_MODE_TYPE:
		mode = flash_mode ? E2P_FLASH_MODE : E2P_BIN_MODE;
		break;
	case HQA_CMD_CHECK_EFUSE_MODE_NATIVE:
		mode = flash_mode ? E2P_FLASH_MODE : E2P_EFUSE_MODE;
		break;
	default:
		mode = E2P_BIN_MODE;
		break;
	}

	*(u32 *)(hdr->data + 2) = htonl(mode);

	return 0;
}

static inline u16
atenl_get_freq_by_channel(u8 ch_band, u16 ch)
{
	u16 base;

	if (ch_band == CH_BAND_6GHZ) {
		base = 5950;
	} else if (ch_band == CH_BAND_5GHZ) {
		if (ch >= 184)
			return 4920 + (ch - 184) * 5;

		base = 5000;
	} else {
		base = 2407;
	}

	return base + ch * 5;
}

u16 atenl_get_center_channel(u8 bw, u8 ch_band, u16 ctrl_ch)
{
	const struct atenl_channel *chan = NULL;
	const struct atenl_channel *ch_list;
	u16 center_ch;
	u8 ch_num;
	int i;

	if (ch_band == CH_BAND_2GHZ || bw <= TEST_CBW_40MHZ)
		return 0;

	if (ch_band == CH_BAND_6GHZ) {
		ch_list = atenl_channels_6ghz;
		ch_num = ARRAY_SIZE(atenl_channels_6ghz);
	} else {
		ch_list = atenl_channels_5ghz;
		ch_num = ARRAY_SIZE(atenl_channels_5ghz);
	}

	for (i = 0; i < ch_num; i++) {
		if (ctrl_ch == ch_list[i].ch) {
			chan = &ch_list[i];
			break;
		}
	}

	if (!chan)
		return 0;

	switch (bw) {
	case TEST_CBW_160MHZ:
		center_ch = chan->ch_160;
		break;
	case TEST_CBW_80MHZ:
		center_ch = chan->ch_80;
		break;
	default:
		center_ch = 0;
		break;
	}

	return center_ch;
}

static void atenl_get_bw_string(u8 bw, char *buf)
{
	switch (bw) {
	case TEST_CBW_160MHZ:
		sprintf(buf, "160");
		break;
	case TEST_CBW_80MHZ:
		sprintf(buf, "80");
		break;
	case TEST_CBW_40MHZ:
		sprintf(buf, "40");
		break;
	default:
		sprintf(buf, "20");
		break;
	}
}

void atenl_set_channel(struct atenl *an, u8 bw, u8 ch_band,
		       u16 ch, u16 center_ch1, u16 center_ch2)
{
	char bw_str[8] = {};
	char cmd[128];
	u16 freq = atenl_get_freq_by_channel(ch_band, ch);
	u16 freq_center1 = atenl_get_freq_by_channel(ch_band, center_ch1);
	int ret;

	if (bw > TEST_CBW_MAX)
		return;

	atenl_get_bw_string(bw, bw_str);

	if (bw == TEST_CBW_20MHZ)
		ret = snprintf(cmd, sizeof(cmd), "iw dev mon%d set freq %u %s",
						 get_band_val(an, an->cur_band, phy_idx),
						 freq, bw_str);
	else
		ret = snprintf(cmd, sizeof(cmd), "iw dev mon%d set freq %u %s %u",
						 get_band_val(an, an->cur_band, phy_idx),
						 freq, bw_str, freq_center1);
	if (snprintf_error(sizeof(cmd), ret))
		return;

	atenl_dbg("[%d]%s: cmd: %s\n", getpid(), __func__, cmd);

	system(cmd);
}

static int
atenl_hqa_set_channel(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u32 *v = (u32 *)hdr->data;
	u8 band = ntohl(v[2]);
	u16 ch1 = ntohl(v[3]);	/* center */
	u16 ch2 = ntohl(v[4]);
	u8 bw = ntohl(v[5]);
	u8 pri_sel = ntohl(v[7]);
	u8 ch_band = ntohl(v[9]);
	u16 ctrl_ch = 0;

	if (band >= MAX_BAND_NUM)
		return -EINVAL;

	if ((bw == TEST_CBW_160MHZ && pri_sel > 7) ||
	    (bw == TEST_CBW_80MHZ && pri_sel > 3) ||
	    (bw == TEST_CBW_40MHZ && pri_sel > 1)) {
		atenl_err("%s: ctrl channel select error\n", __func__);
		return -EINVAL;
	}

	an->cur_band = band;

	if (ch_band == CH_BAND_2GHZ) {
		ctrl_ch = ch1;
		switch (bw) {
		case TEST_CBW_40MHZ:
			if (pri_sel == 1)
				ctrl_ch += 2;
			else
				ctrl_ch -= 2;
			break;
		default:
			break;
		}

		atenl_set_channel(an, bw, CH_BAND_2GHZ, ctrl_ch, ch1, 0);
	} else {
		const struct atenl_channel *chan = NULL;
		const struct atenl_channel *ch_list;
		u8 ch_num;
		int i;

		if (ch_band == CH_BAND_6GHZ) {
			ch_list = atenl_channels_6ghz;
			ch_num = ARRAY_SIZE(atenl_channels_6ghz);
		} else {
			ch_list = atenl_channels_5ghz;
			ch_num = ARRAY_SIZE(atenl_channels_5ghz);
		}

		if (bw == TEST_CBW_160MHZ) {
			for (i = 0; i < ch_num; i++) {
				if (ch1 == ch_list[i].ch_160) {
					chan = &ch_list[i];
					break;
				} else if (ch1 < ch_list[i].ch_160) {
					chan = &ch_list[i - 1];
					break;
				}
			}
		} else if (bw == TEST_CBW_80MHZ) {
			for (i = 0; i < ch_num; i++) {
				if (ch1 == ch_list[i].ch_80) {
					chan = &ch_list[i];
					break;
				} else if (ch1 < ch_list[i].ch_80) {
					chan = &ch_list[i - 1];
					break;
				}
			}
		} else {
			for (i = 0; i < ch_num; i++) {
				if (ch1 <= ch_list[i].ch) {
					if (ch1 == ch_list[i].ch)
						chan = &ch_list[i];
					else
						chan = &ch_list[i - 1];
					break;
				}
			}
		}

		if (!chan)
			return -EINVAL;

		if (bw != TEST_CBW_20MHZ) {
			chan += pri_sel;
			if (chan > &ch_list[ch_num - 1])
				return -EINVAL;
		}
		ctrl_ch = chan->ch;

		atenl_set_channel(an, bw, ch_band, ctrl_ch, ch1, ch2);
	}

	*(u32 *)(hdr->data + 2) = data->ext_id;

	return 0;
}

static int
atenl_hqa_tx_time_option(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u32 *v = (u32 *)hdr->data;
	u8 band = ntohl(v[1]);
	u32 option = ntohl(v[2]);

	if (band >= MAX_BAND_NUM)
		return -EINVAL;

	set_band_val(an, band, use_tx_time, !!option);
	*(u32 *)(hdr->data + 2) = data->ext_id;

	return 0;
}

static inline enum atenl_cmd atenl_get_cmd_by_id(u16 cmd_idx)
{
#define CMD_ID_GROUP	GENMASK(15, 8)
	u8 group = FIELD_GET(CMD_ID_GROUP, cmd_idx);

	if (cmd_idx == 0x1600)
		return HQA_CMD_EXT;

	if (group == 0x10) {
		switch (cmd_idx) {
		case 0x1000:
			return HQA_CMD_OPEN_ADAPTER;
		case 0x1001:
			return HQA_CMD_CLOSE_ADAPTER;
		case 0x100b:
			return HQA_CMD_SET_TX_PATH;
		case 0x100c:
			return HQA_CMD_SET_RX_PATH;
		case 0x1011:
			return HQA_CMD_SET_TX_POWER;
		case 0x1018:
			return HQA_CMD_SET_TX_POWER_MANUAL;
		case 0x100d:
			return HQA_CMD_LEGACY;
		default:
			break;
		}
	} else if (group == 0x11) {
		switch (cmd_idx) {
		case 0x1104:
			return HQA_CMD_SET_TX_BW;
		case 0x1105:
			return HQA_CMD_SET_TX_PKT_BW;
		case 0x1106:
			return HQA_CMD_SET_TX_PRI_BW;
		case 0x1107:
			return HQA_CMD_SET_FREQ_OFFSET;
		case 0x1109:
			return HQA_CMD_SET_TSSI;
		case 0x110d:
			return HQA_CMD_ANT_SWAP_CAP;
		case 0x1101:
		case 0x1102:
			return HQA_CMD_LEGACY;
		default:
			break;
		}
	} else if (group == 0x12) {
		switch (cmd_idx) {
		case 0x1200:
			return HQA_CMD_RESET_TX_RX_COUNTER;
		default:
			break;
		}
	} else if (group == 0x13) {
		switch (cmd_idx) {
		case 0x1301:
			return HQA_CMD_WRITE_MAC_BBP_REG;
		case 0x1302:
			return HQA_CMD_READ_MAC_BBP_REG;
		case 0x1307:
			return HQA_CMD_READ_EEPROM_BULK;
		case 0x1306:
		case 0x1308:
			return HQA_CMD_WRITE_EEPROM_BULK;
		case 0x1309:
			return HQA_CMD_CHECK_EFUSE_MODE;
		case 0x130a:
			return HQA_CMD_GET_EFUSE_FREE_BLOCK;
		case 0x130d:
			return HQA_CMD_GET_TX_POWER;
		case 0x130e:
			return HQA_CMD_SET_CFG;
		case 0x130f:
			return HQA_CMD_GET_FREQ_OFFSET;
		case 0x1311:
			return HQA_CMD_CONTINUOUS_TX;
		case 0x1312:
			return HQA_CMD_SET_RX_PKT_LEN;
		case 0x1313:
			return HQA_CMD_GET_TX_INFO;
		case 0x1314:
			return HQA_CMD_GET_CFG;
		case 0x131f:
			return HQA_CMD_UNKNOWN;
		case 0x131a:
			return HQA_CMD_GET_TX_TONE_POWER;
		default:
			break;
		}
	} else if (group == 0x14) {
		switch (cmd_idx) {
		case 0x1401:
			return HQA_CMD_READ_TEMPERATURE;
		default:
			break;
		}
	} else if (group == 0x15) {
		switch (cmd_idx) {
		case 0x1500:
			return HQA_CMD_GET_FW_INFO;
		case 0x1505:
			return HQA_CMD_SET_TSSI;
		case 0x1509:
			return HQA_CMD_SET_RF_MODE;
		case 0x1511:
			return HQA_CMD_WRITE_BUFFER_DONE;
		case 0x1514:
			return HQA_CMD_GET_CHIP_ID;
		case 0x151b:
			return HQA_CMD_GET_SUB_CHIP_ID;
		case 0x151c:
			return HQA_CMD_GET_RX_INFO;
		case 0x151e:
			return HQA_CMD_GET_RF_CAP;
		case 0x1522:
			return HQA_CMD_CHECK_EFUSE_MODE_TYPE;
		case 0x1523:
			return HQA_CMD_CHECK_EFUSE_MODE_NATIVE;
		case 0x152d:
			return HQA_CMD_GET_BAND;
		case 0x1594:
			return HQA_CMD_SET_RU;
		case 0x1502:
		case 0x150b:
			return HQA_CMD_LEGACY;
		default:
			break;
		}
	}

	return HQA_CMD_ERR;
}

static inline enum atenl_ext_cmd atenl_get_ext_cmd(u16 ext_cmd_idx)
{
#define EXT_CMD_ID_GROUP	GENMASK(7, 4)
	u8 ext_group = FIELD_GET(EXT_CMD_ID_GROUP, ext_cmd_idx);

	if (ext_group == 0) {
		switch (ext_cmd_idx) {
		case 0x1:
			return HQA_EXT_CMD_SET_CHANNEL;
		case 0x2:
			return HQA_EXT_CMD_SET_TX;
		case 0x3:
			return HQA_EXT_CMD_START_TX;
		case 0x4:
			return HQA_EXT_CMD_START_RX;
		case 0x5:
			return HQA_EXT_CMD_STOP_TX;
		case 0x6:
			return HQA_EXT_CMD_STOP_RX;
		case 0x8:
			return HQA_EXT_CMD_IBF_SET_VAL;
		case 0x9:
			return HQA_EXT_CMD_IBF_GET_STATUS;
		case 0xc:
			return HQA_EXT_CMD_IBF_PROF_UPDATE_ALL;
		default:
			break;
		}
	} else if (ext_group == 1) {
	} else if (ext_group == 2) {
		switch (ext_cmd_idx) {
		case 0x26:
			return HQA_EXT_CMD_SET_TX_TIME_OPT;
		case 0x27:
			return HQA_EXT_CMD_OFF_CH_SCAN;
		default:
			break;
		}
	}

	return HQA_EXT_CMD_UNSPEC;
}

#define ATENL_GROUP(_cmd, _resp_len, _ops)	\
	[HQA_CMD_##_cmd] = { .resp_len=_resp_len, .ops=_ops }
static const struct atenl_cmd_ops atenl_ops[] = {
	ATENL_GROUP(OPEN_ADAPTER, 2, atenl_hqa_adapter),
	ATENL_GROUP(CLOSE_ADAPTER, 2, atenl_hqa_adapter),
	ATENL_GROUP(SET_TX_PATH, 2, atenl_nl_process),
	ATENL_GROUP(SET_RX_PATH, 2, atenl_nl_process),
	ATENL_GROUP(SET_TX_POWER, 2, atenl_nl_process),
	ATENL_GROUP(SET_TX_POWER_MANUAL, 2, atenl_hqa_skip),
	ATENL_GROUP(SET_TX_BW, 2, atenl_hqa_skip),
	ATENL_GROUP(SET_TX_PKT_BW, 2, atenl_hqa_skip),
	ATENL_GROUP(SET_TX_PRI_BW, 2, atenl_hqa_skip),
	ATENL_GROUP(SET_FREQ_OFFSET, 2, atenl_nl_process),
	ATENL_GROUP(ANT_SWAP_CAP, 6, atenl_hqa_skip),
	ATENL_GROUP(RESET_TX_RX_COUNTER, 2, atenl_hqa_reset_counter),
	ATENL_GROUP(WRITE_MAC_BBP_REG, 2, atenl_hqa_mac_bbp_reg),
	ATENL_GROUP(READ_MAC_BBP_REG, 0, atenl_hqa_mac_bbp_reg),
	ATENL_GROUP(READ_EEPROM_BULK, 0, atenl_hqa_eeprom_bulk),
	ATENL_GROUP(WRITE_EEPROM_BULK, 2, atenl_hqa_eeprom_bulk),
	ATENL_GROUP(CHECK_EFUSE_MODE, 6, atenl_hqa_check_efuse_mode),
	ATENL_GROUP(GET_EFUSE_FREE_BLOCK, 6, atenl_hqa_get_efuse_free_block),
	ATENL_GROUP(GET_TX_POWER, 10, atenl_hqa_get_tx_power),
	ATENL_GROUP(GET_FREQ_OFFSET, 6, atenl_hqa_get_freq_offset), /*TODO: MCU CMD, read eeprom?*/
	ATENL_GROUP(CONTINUOUS_TX, 6, atenl_nl_process),
	ATENL_GROUP(SET_RX_PKT_LEN, 2, atenl_hqa_skip),
	ATENL_GROUP(GET_TX_INFO, 10, atenl_nl_process),
	ATENL_GROUP(GET_CFG, 6, atenl_hqa_get_cfg), /*TODO*/
	ATENL_GROUP(GET_TX_TONE_POWER, 6, atenl_hqa_skip),
	ATENL_GROUP(SET_CFG, 2, atenl_nl_process),
	ATENL_GROUP(READ_TEMPERATURE, 6, atenl_hqa_read_temperature),
	ATENL_GROUP(GET_FW_INFO, 32, atenl_hqa_skip), /* TODO: check format */
	ATENL_GROUP(SET_TSSI, 2, atenl_nl_process),
	ATENL_GROUP(SET_RF_MODE, 2, atenl_hqa_set_rf_mode),
	ATENL_GROUP(WRITE_BUFFER_DONE, 2, atenl_hqa_eeprom_bulk),
	ATENL_GROUP(GET_CHIP_ID, 6, atenl_hqa_get_chip_id),
	ATENL_GROUP(GET_SUB_CHIP_ID, 6, atenl_hqa_get_sub_chip_id),
	ATENL_GROUP(GET_RX_INFO, 0, atenl_nl_process),
	ATENL_GROUP(GET_RF_CAP, 10, atenl_hqa_get_rf_cap),
	ATENL_GROUP(CHECK_EFUSE_MODE_TYPE, 6, atenl_hqa_check_efuse_mode),
	ATENL_GROUP(CHECK_EFUSE_MODE_NATIVE, 6, atenl_hqa_check_efuse_mode),
	ATENL_GROUP(GET_BAND, 6, atenl_hqa_get_band),
	ATENL_GROUP(SET_RU, 2, atenl_nl_process_many),

	ATENL_GROUP(LEGACY, 2, atenl_hqa_skip),
	ATENL_GROUP(UNKNOWN, 1024, atenl_hqa_skip),
};
#undef ATENL_GROUP

#define ATENL_EXT(_cmd, _resp_len, _ops)	\
	[HQA_EXT_CMD_##_cmd] = { .resp_len=_resp_len, .ops=_ops }
static const struct atenl_cmd_ops atenl_ext_ops[] = {
	ATENL_EXT(SET_CHANNEL, 6, atenl_hqa_set_channel),
	ATENL_EXT(SET_TX, 6, atenl_nl_process),
	ATENL_EXT(START_TX, 6, atenl_nl_process),
	ATENL_EXT(STOP_TX, 6, atenl_nl_process),
	ATENL_EXT(START_RX, 6, atenl_nl_process),
	ATENL_EXT(STOP_RX, 6, atenl_nl_process),
	ATENL_EXT(SET_TX_TIME_OPT, 6, atenl_hqa_tx_time_option),
	ATENL_EXT(OFF_CH_SCAN, 6, atenl_nl_process),
	ATENL_EXT(IBF_SET_VAL, 6, atenl_nl_process),
	ATENL_EXT(IBF_GET_STATUS, 10, atenl_nl_process),
	ATENL_EXT(IBF_PROF_UPDATE_ALL, 6, atenl_nl_process_many),
};
#undef ATENL_EXT

int atenl_hqa_recv(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	u16 cmd_type = ntohs(hdr->cmd_type);
	int fd = an->pipefd[PIPE_WRITE];
	int ret;

	if (ntohl(hdr->magic_no) != RACFG_MAGIC_NO)
		return -EINVAL;

	if (FIELD_GET(RACFG_CMD_TYPE_MASK, cmd_type) != RACFG_CMD_TYPE_ETHREQ &&
	    FIELD_GET(RACFG_CMD_TYPE_MASK, cmd_type) != RACFG_CMD_TYPE_PLATFORM_MODULE) {
		atenl_err("[%d]%s: cmd type error = 0x%x\n", getpid(), __func__, cmd_type);
		return -EINVAL;
	}

	atenl_dbg("[%d]%s: recv cmd type = 0x%x, id = 0x%x\n",
		  getpid(), __func__, cmd_type, ntohs(hdr->cmd_id));

	ret = write(fd, data, data->len);
	if (ret < 0) {
		perror("pipe write");
		return ret;
	}

	return 0;
}

int atenl_hqa_proc_cmd(struct atenl *an, struct atenl_data *data)
{
	struct atenl_cmd_hdr *hdr = atenl_hdr(data);
	const struct atenl_cmd_ops *ops;
	u16 cmd_id = ntohs(hdr->cmd_id);
	u16 status = 0;

	data->cmd = atenl_get_cmd_by_id(cmd_id);
	if (data->cmd == HQA_CMD_ERR) {
		atenl_err("Unknown command id: 0x%04x\n", cmd_id);
		goto done;
	}

	if (data->cmd == HQA_CMD_EXT) {
		data->ext_id = ntohl(*(u32 *)hdr->data);
		data->ext_cmd = atenl_get_ext_cmd(data->ext_id);
		if (data->ext_cmd == HQA_EXT_CMD_UNSPEC) {
			atenl_err("Unknown ext command id: 0x%04x\n", data->ext_id);
			goto done;
		}

		ops = &atenl_ext_ops[data->ext_cmd];
	} else {
		ops = &atenl_ops[data->cmd];
	}

	atenl_dbg_print_data(data, __func__,
			     ntohs(hdr->len) + ETH_HLEN + RACFG_HLEN);
	if (ops->ops)
		status = htons(ops->ops(an, data));
	if (ops->resp_len)
		hdr->len = htons(ops->resp_len);

	*(u16 *)hdr->data = status;

done:
	data->len = ntohs(hdr->len) + ETH_HLEN + RACFG_HLEN;
	hdr->cmd_type |= ~htons(RACFG_CMD_TYPE_MASK);

	return 0;
}
