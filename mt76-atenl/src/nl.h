/* Copyright (C) 2021-2022 Mediatek Inc. */
#ifndef __ATENL_NL_H
#define __ATENL_NL_H

/* This is copied from mt76/testmode.h */

#define MT76_TM_TIMEOUT			10
#define MT76_TM_EEPROM_BLOCK_SIZE	16

/**
 * enum mt76_testmode_attr - testmode attributes inside NL80211_ATTR_TESTDATA
 *
 * @MT76_TM_ATTR_UNSPEC: (invalid attribute)
 *
 * @MT76_TM_ATTR_RESET: reset parameters to default (flag)
 * @MT76_TM_ATTR_STATE: test state (u32), see &enum mt76_testmode_state
 *
 * @MT76_TM_ATTR_MTD_PART: mtd/emmc partition or binfile used for eeprom data (string)
 * @MT76_TM_ATTR_MTD_OFFSET: offset of eeprom data within the partition (u32)
 * @MT76_TM_ATTR_BAND_IDX: band idx of the chip (u8)
 *
 * @MT76_TM_ATTR_SKU_EN: config txpower sku is enabled or disabled in testmode (u8)
 * @MT76_TM_ATTR_TX_COUNT: configured number of frames to send when setting
 *	state to MT76_TM_STATE_TX_FRAMES (u32)
 * @MT76_TM_ATTR_TX_LENGTH: packet tx mpdu length (u32)
 * @MT76_TM_ATTR_TX_RATE_MODE: packet tx mode (u8, see &enum mt76_testmode_tx_mode)
 * @MT76_TM_ATTR_TX_RATE_NSS: packet tx number of spatial streams (u8)
 * @MT76_TM_ATTR_TX_RATE_IDX: packet tx rate/MCS index (u8)
 * @MT76_TM_ATTR_TX_RATE_SGI: packet tx use short guard interval (u8)
 * @MT76_TM_ATTR_TX_RATE_LDPC: packet tx enable LDPC (u8)
 * @MT76_TM_ATTR_TX_RATE_STBC: packet tx enable STBC (u8)
 * @MT76_TM_ATTR_TX_LTF: packet tx LTF, set 0 to 2 for 1x, 2x, and 4x LTF (u8)
 *
 * @MT76_TM_ATTR_TX_ANTENNA: tx antenna mask (u8)
 * @MT76_TM_ATTR_TX_POWER_CONTROL: enable tx power control (u8)
 * @MT76_TM_ATTR_TX_POWER: per-antenna tx power array (nested, u8 attrs)
 *
 * @MT76_TM_ATTR_TX_PKT_BW: per-packet data bandwidth (u8)
 * @MT76_TM_ATTR_TX_PRI_SEL: primary channel selection index (u8)
 *
 * @MT76_TM_ATTR_FREQ_OFFSET: RF frequency offset (u32)
 *
 * @MT76_TM_ATTR_STATS: statistics (nested, see &enum mt76_testmode_stats_attr)
 *
 * @MT76_TM_ATTR_PRECAL: pre-cal data (u8)
 * @MT76_TM_ATTR_PRECAL_INFO: group size, dpd size, dpd_info, transmit size,
 *                            eeprom cal indicator (u32),
 *                            dpd_info = [dpd_per_chan_size, chan_num_2g,
 *                                        chan_num_5g, chan_num_6g]
 * @MT76_TM_ATTR_TX_SPE_IDX: tx spatial extension index (u8)
 *
 * @MT76_TM_ATTR_TX_DUTY_CYCLE: packet tx duty cycle (u8)
 * @MT76_TM_ATTR_TX_IPG: tx inter-packet gap, in unit of us (u32)
 * @MT76_TM_ATTR_TX_TIME: packet transmission time, in unit of us (u32)
 *
 * @MT76_TM_ATTR_DRV_DATA: driver specific netlink attrs (nested)
 *
 * @MT76_TM_ATTR_MAC_ADDRS: array of nested MAC addresses (nested)
 * @MT76_TM_ATTR_AID: association index for wtbl (u8)
 * @MT76_TM_ATTR_RU_ALLOC: resource unit allocation subfield (u8)
 * @MT76_TM_ATTR_RU_IDX: resource unit index (u8)
 *
 * @MT76_TM_ATTR_EEPROM_ACTION: eeprom setting actions
 *	(u8, see &enum mt76_testmode_eeprom_action)
 * @MT76_TM_ATTR_EEPROM_OFFSET: offset of eeprom data block for writing (u32)
 * @MT76_TM_ATTR_EEPROM_VAL: values for writing into a 16-byte data block
 *	(nested, u8 attrs)
 *
 * @MT76_TM_ATTR_CFG: config testmode rf feature (nested, see &mt76_testmode_cfg)
 * @MT76_TM_ATTR_TXBF_ACT: txbf setting actions (u8)
 * @MT76_TM_ATTR_TXBF_PARAM: txbf parameters (nested)
 *
 * @MT76_TM_ATTR_OFF_CH_SCAN_CH: config the channel of background chain (ZWDFS) (u8)
 * @MT76_TM_ATTR_OFF_CH_SCAN_CENTER_CH: config the center channel of background chain (ZWDFS) (u8)
 * @MT76_TM_ATTR_OFF_CH_SCAN_BW: config the bandwidth of background chain (ZWDFS) (u8)
 * @MT76_TM_ATTR_OFF_CH_SCAN_PATH: config the tx path of background chain (ZWDFS) (u8)
 *
 * @MT76_TM_ATTR_IPI_THRESHOLD: config the IPI index you want to read (u8)
 * @MT76_TM_ATTR_IPI_PERIOD: config the time period for reading
 *			     the histogram of specific IPI index (u8)
 * @MT76_TM_ATTR_IPI_ANTENNA_INDEX: config the antenna index for reading
 *				    the histogram of specific IPI index (u8)
 * @MT76_TM_ATTR_IPI_RESET: reset the IPI counter
 *
 * @MT76_TM_ATTR_LM_ACT: list mode setting actions (u8)
 * @MT76_TM_ATTR_LM_SEG_IDX: segment index used in list mode (u8)
 * @MT76_TM_ATTR_LM_CENTER_CH: center channel used in list mode (u8)
 * @MT76_TM_ATTR_LM_CBW: system index used in list mode (u8)
 * @MT76_TM_ATTR_LM_STA_IDX: station index used in list mode (u8)
 * @MT76_TM_ATTR_LM_SEG_TIMEOUT: TX/RX segment timeout used in list mode (u8)
 *
 * @MT76_TM_ATTR_RADIO_IDX: radio index used for single multi-radio wiphy (u32)
 *
 * @MT76_TM_ATTR_FAST_CAL: perform a fast calibration for a channel switch speed boost
 *	(u8, see &enum mt76_testmode_fast_cal_type)
 *
 */
enum mt76_testmode_attr {
	MT76_TM_ATTR_UNSPEC,

	MT76_TM_ATTR_RESET,
	MT76_TM_ATTR_STATE,

	MT76_TM_ATTR_MTD_PART,
	MT76_TM_ATTR_MTD_OFFSET,
	MT76_TM_ATTR_BAND_IDX,

	MT76_TM_ATTR_SKU_EN,
	MT76_TM_ATTR_TX_COUNT,
	MT76_TM_ATTR_TX_LENGTH,
	MT76_TM_ATTR_TX_RATE_MODE,
	MT76_TM_ATTR_TX_RATE_NSS,
	MT76_TM_ATTR_TX_RATE_IDX,
	MT76_TM_ATTR_TX_RATE_SGI,
	MT76_TM_ATTR_TX_RATE_LDPC,
	MT76_TM_ATTR_TX_RATE_STBC,
	MT76_TM_ATTR_TX_LTF,

	MT76_TM_ATTR_TX_ANTENNA,
	MT76_TM_ATTR_TX_POWER_CONTROL,
	MT76_TM_ATTR_TX_POWER,

	MT76_TM_ATTR_TX_PKT_BW,
	MT76_TM_ATTR_TX_PRI_SEL,

	MT76_TM_ATTR_FREQ_OFFSET,

	MT76_TM_ATTR_STATS,

	MT76_TM_ATTR_PRECAL,
	MT76_TM_ATTR_PRECAL_INFO,

	MT76_TM_ATTR_TX_SPE_IDX,

	MT76_TM_ATTR_TX_DUTY_CYCLE,
	MT76_TM_ATTR_TX_IPG,
	MT76_TM_ATTR_TX_TIME,

	MT76_TM_ATTR_DRV_DATA,

	MT76_TM_ATTR_MAC_ADDRS,
	MT76_TM_ATTR_AID,
	MT76_TM_ATTR_RU_ALLOC,
	MT76_TM_ATTR_RU_IDX,

	MT76_TM_ATTR_EEPROM_ACTION,
	MT76_TM_ATTR_EEPROM_OFFSET,
	MT76_TM_ATTR_EEPROM_VAL,

	MT76_TM_ATTR_CFG,
	MT76_TM_ATTR_TXBF_ACT,
	MT76_TM_ATTR_TXBF_PARAM,

	MT76_TM_ATTR_OFF_CH_SCAN_CH,
	MT76_TM_ATTR_OFF_CH_SCAN_CENTER_CH,
	MT76_TM_ATTR_OFF_CH_SCAN_BW,
	MT76_TM_ATTR_OFF_CH_SCAN_PATH,

	MT76_TM_ATTR_IPI_THRESHOLD,
	MT76_TM_ATTR_IPI_PERIOD,
	MT76_TM_ATTR_IPI_ANTENNA_INDEX,
	MT76_TM_ATTR_IPI_RESET,

	MT76_TM_ATTR_LM_ACT,
	MT76_TM_ATTR_LM_SEG_IDX,
	MT76_TM_ATTR_LM_CENTER_CH,
	MT76_TM_ATTR_LM_CBW,
	MT76_TM_ATTR_LM_STA_IDX,
	MT76_TM_ATTR_LM_SEG_TIMEOUT,

	MT76_TM_ATTR_RADIO_IDX,

	MT76_TM_ATTR_FAST_CAL,

	/* keep last */
	NUM_MT76_TM_ATTRS,
	MT76_TM_ATTR_MAX = NUM_MT76_TM_ATTRS - 1,
};

/**
 * enum mt76_testmode_state - statistics attributes
 *
 * @MT76_TM_STATS_ATTR_TX_PENDING: pending tx frames (u32)
 * @MT76_TM_STATS_ATTR_TX_QUEUED: queued tx frames (u32)
 * @MT76_TM_STATS_ATTR_TX_DONE: completed tx frames (u32)
 *
 * @MT76_TM_STATS_ATTR_RX_PACKETS: number of rx packets (u64)
 * @MT76_TM_STATS_ATTR_RX_FCS_ERROR: number of rx packets with FCS error (u64)
 * @MT76_TM_STATS_ATTR_LAST_RX: information about the last received packet
 *	see &enum mt76_testmode_rx_attr
 * @MT76_TM_STATS_ATTR_RX_LEN_MISMATCH: number of rx packets with length
 *	mismatch error (u64)
 */
enum mt76_testmode_stats_attr {
	MT76_TM_STATS_ATTR_UNSPEC,
	MT76_TM_STATS_ATTR_PAD,

	MT76_TM_STATS_ATTR_TX_PENDING,
	MT76_TM_STATS_ATTR_TX_QUEUED,
	MT76_TM_STATS_ATTR_TX_DONE,

	MT76_TM_STATS_ATTR_RX_PACKETS,
	MT76_TM_STATS_ATTR_RX_FCS_ERROR,
	MT76_TM_STATS_ATTR_LAST_RX,
	MT76_TM_STATS_ATTR_RX_LEN_MISMATCH,

	/* keep last */
	NUM_MT76_TM_STATS_ATTRS,
	MT76_TM_STATS_ATTR_MAX = NUM_MT76_TM_STATS_ATTRS - 1,
};


/**
 * enum mt76_testmode_rx_attr - packet rx information
 *
 * @MT76_TM_RX_ATTR_FREQ_OFFSET: frequency offset (s32)
 * @MT76_TM_RX_ATTR_RCPI: received channel power indicator (array, u8)
 * @MT76_TM_RX_ATTR_RSSI: received signal strength indicator (array, s8)
 * @MT76_TM_RX_ATTR_IB_RSSI: internal inband RSSI (array, s8)
 * @MT76_TM_RX_ATTR_WB_RSSI: internal wideband RSSI (array, s8)
 * @MT76_TM_RX_ATTR_SNR: signal-to-noise ratio (u8)
 */
enum mt76_testmode_rx_attr {
	MT76_TM_RX_ATTR_UNSPEC,

	MT76_TM_RX_ATTR_FREQ_OFFSET,
	MT76_TM_RX_ATTR_RCPI,
	MT76_TM_RX_ATTR_RSSI,
	MT76_TM_RX_ATTR_IB_RSSI,
	MT76_TM_RX_ATTR_WB_RSSI,
	MT76_TM_RX_ATTR_SNR,

	/* keep last */
	NUM_MT76_TM_RX_ATTRS,
	MT76_TM_RX_ATTR_MAX = NUM_MT76_TM_RX_ATTRS - 1,
};

/**
 * enum mt76_testmode_state - phy test state
 *
 * @MT76_TM_STATE_OFF: test mode disabled (normal operation)
 * @MT76_TM_STATE_IDLE: test mode enabled, but idle
 * @MT76_TM_STATE_TX_FRAMES: send a fixed number of test frames
 * @MT76_TM_STATE_RX_FRAMES: receive packets and keep statistics
 * @MT76_TM_STATE_TX_CONT: waveform tx without time gap
 * @MT76_TM_STATE_GROUP_PREK: start group pre-calibration
 * @MT76_TM_STATE_GROUP_PREK_DUMP: dump the data of group pre-calibration
 * @MT76_TM_STATE_GROUP_PREK_CLEAN: clear the data group pre-calibration
 * @MT76_TM_STATE_DPD_2G: start 2G DPD pre-calibration
 * @MT76_TM_STATE_DPD_5G: start 5G DPD pre-calibration
 * @MT76_TM_STATE_DPD_6G: start 6G DPD pre-calibration
 * @MT76_TM_STATE_DPD_DUMP: dump the data of DPD pre-calibration
 * @MT76_TM_STATE_DPD_CLEAN: clear the data of DPD pre-calibration
 * @MT76_TM_STATE_RX_GAIN_CAL: start RX gain calibration
 * @MT76_TM_STATE_RX_GAIN_CAL_DUMP: dump the data of RX gain calibration
 * @MT76_TM_STATE_RX_GAIN_CAL_CLEAN: clear the data of RX gain calibration
 * @MT76_TM_STATE_ON: test mode enabled used in offload firmware
 */
enum mt76_testmode_state {
	MT76_TM_STATE_OFF,
	MT76_TM_STATE_IDLE,
	MT76_TM_STATE_TX_FRAMES,
	MT76_TM_STATE_RX_FRAMES,
	MT76_TM_STATE_TX_CONT,
	MT76_TM_STATE_GROUP_PREK,
	MT76_TM_STATE_GROUP_PREK_DUMP,
	MT76_TM_STATE_GROUP_PREK_CLEAN,
	MT76_TM_STATE_DPD_2G,
	MT76_TM_STATE_DPD_5G,
	MT76_TM_STATE_DPD_6G,
	MT76_TM_STATE_DPD_DUMP,
	MT76_TM_STATE_DPD_CLEAN,
	MT76_TM_STATE_RX_GAIN_CAL,
	MT76_TM_STATE_RX_GAIN_CAL_DUMP,
	MT76_TM_STATE_RX_GAIN_CAL_CLEAN,
	MT76_TM_STATE_ON,

	/* keep last */
	NUM_MT76_TM_STATES,
	MT76_TM_STATE_MAX = NUM_MT76_TM_STATES - 1,
};

/**
 * enum mt76_testmode_tx_mode - packet tx phy mode
 *
 * @MT76_TM_TX_MODE_CCK: legacy CCK mode
 * @MT76_TM_TX_MODE_OFDM: legacy OFDM mode
 * @MT76_TM_TX_MODE_HT: 802.11n MCS
 * @MT76_TM_TX_MODE_VHT: 802.11ac MCS
 * @MT76_TM_TX_MODE_HE_SU: 802.11ax single-user MIMO
 * @MT76_TM_TX_MODE_HE_EXT_SU: 802.11ax extended-range SU
 * @MT76_TM_TX_MODE_HE_TB: 802.11ax trigger-based
 * @MT76_TM_TX_MODE_HE_MU: 802.11ax multi-user MIMO
 * @MT76_TM_TX_MODE_EHT_SU: 802.11be single-user MIMO
 * @MT76_TM_TX_MODE_EHT_TRIG: 802.11be trigger-based
 * @MT76_TM_TX_MODE_EHT_MU: 802.11be multi-user MIMO
 */
enum mt76_testmode_tx_mode {
	MT76_TM_TX_MODE_CCK,
	MT76_TM_TX_MODE_OFDM,
	MT76_TM_TX_MODE_HT,
	MT76_TM_TX_MODE_VHT,
	MT76_TM_TX_MODE_HE_SU,
	MT76_TM_TX_MODE_HE_EXT_SU,
	MT76_TM_TX_MODE_HE_TB,
	MT76_TM_TX_MODE_HE_MU,
	MT76_TM_TX_MODE_EHT_SU,
	MT76_TM_TX_MODE_EHT_TRIG,
	MT76_TM_TX_MODE_EHT_MU,

	/* keep last */
	NUM_MT76_TM_TX_MODES,
	MT76_TM_TX_MODE_MAX = NUM_MT76_TM_TX_MODES - 1,
};

/**
 * enum mt76_testmode_eeprom_action - eeprom setting actions
 *
 * @MT76_TM_EEPROM_ACTION_UPDATE_DATA: update rf values to specific
 *	eeprom data block
 * @MT76_TM_EEPROM_ACTION_UPDATE_BUFFER_MODE: send updated eeprom data to fw
 * @MT76_TM_EEPROM_ACTION_WRITE_TO_EFUSE: write eeprom data back to efuse
 * @MT76_TM_EEPROM_ACTION_WRITE_TO_EXT_EEPROM: write eeprom data back to external eeprom
 */
enum mt76_testmode_eeprom_action {
	MT76_TM_EEPROM_ACTION_UPDATE_DATA,
	MT76_TM_EEPROM_ACTION_UPDATE_BUFFER_MODE,
	MT76_TM_EEPROM_ACTION_WRITE_TO_EFUSE,
	MT76_TM_EEPROM_ACTION_WRITE_TO_EXT_EEPROM,

	/* keep last */
	NUM_MT76_TM_EEPROM_ACTION,
	MT76_TM_EEPROM_ACTION_MAX = NUM_MT76_TM_EEPROM_ACTION - 1,
};

/**
 * enum mt76_testmode_txbf_act - txbf action
 *
 * @MT76_TM_TXBF_ACT_GOLDEN_INIT: init ibf setting for golden device
 * @MT76_TM_TXBF_ACT_INIT: init ibf setting for DUT
 * @MT76_TM_TX_EBF_ACT_GOLDEN_INIT: init ebf setting for golden device
 * @MT76_TM_TX_EBF_ACT_INIT: init ebf setting for DUT
 * @MT76_TM_TXBF_ACT_UPDATE_CH: update channel info
 * @MT76_TM_TXBF_ACT_PHASE_COMP: txbf phase compensation
 * @MT76_TM_TXBF_ACT_TX_PREP: TX preparation for txbf
 * @MT76_TM_TXBF_ACT_IBF_PROF_UPDATE: update ibf profile (pfmu tag, bf sta record)
 * @MT76_TM_TXBF_ACT_EBF_PROF_UPDATE: update ebf profile
 * @MT76_TM_TXBF_ACT_APPLY_TX: apply TX setting for txbf
 * @MT76_TM_TXBF_ACT_PHASE_CAL: perform txbf phase calibration
 * @MT76_TM_TXBF_ACT_PROF_UPDATE_ALL: update bf profile via instrument
 * @MT76_TM_TXBF_ACT_PROF_UPDATE_ALL_CMD: update bf profile via instrument
 * @MT76_TM_TXBF_ACT_E2P_UPDATE: write back txbf calibration result to eeprom
 * @MT76_TM_TXBF_ACT_TRIGGER_SOUNDING: trigger beamformer to send sounding packet
 * @MT76_TM_TXBF_ACT_STOP_SOUNDING: stop sending sounding packet
 * @MT76_TM_TXBF_ACT_PROFILE_TAG_READ: read pfmu tag
 * @MT76_TM_TXBF_ACT_PROFILE_TAG_WRITE: update pfmu tag
 * @MT76_TM_TXBF_ACT_PROFILE_TAG_INVALID: invalidate pfmu tag
 * @MT76_TM_TXBF_ACT_STA_REC_READ: read bf sta record
 * @MT76_TM_TXBF_ACT_TXCMD: configure txcmd bf bit manually
 */
enum mt76_testmode_txbf_act {
	MT76_TM_TXBF_ACT_GOLDEN_INIT,
	MT76_TM_TXBF_ACT_INIT,
	MT76_TM_TX_EBF_ACT_GOLDEN_INIT,
	MT76_TM_TX_EBF_ACT_INIT,
	MT76_TM_TXBF_ACT_UPDATE_CH,
	MT76_TM_TXBF_ACT_PHASE_COMP,
	MT76_TM_TXBF_ACT_TX_PREP,
	MT76_TM_TXBF_ACT_IBF_PROF_UPDATE,
	MT76_TM_TXBF_ACT_EBF_PROF_UPDATE,
	MT76_TM_TXBF_ACT_APPLY_TX,
	MT76_TM_TXBF_ACT_PHASE_CAL,
	MT76_TM_TXBF_ACT_PROF_UPDATE_ALL,
	MT76_TM_TXBF_ACT_PROF_UPDATE_ALL_CMD,
	MT76_TM_TXBF_ACT_E2P_UPDATE,
	MT76_TM_TXBF_ACT_TRIGGER_SOUNDING,
	MT76_TM_TXBF_ACT_STOP_SOUNDING,
	MT76_TM_TXBF_ACT_PROFILE_TAG_READ,
	MT76_TM_TXBF_ACT_PROFILE_TAG_WRITE,
	MT76_TM_TXBF_ACT_PROFILE_TAG_INVALID,
	MT76_TM_TXBF_ACT_STA_REC_READ,
	MT76_TM_TXBF_ACT_TXCMD,

	/* keep last */
	NUM_MT76_TM_TXBF_ACT,
	MT76_TM_TXBF_ACT_MAX = NUM_MT76_TM_TXBF_ACT - 1,
};

#define LIST_SEG_MAX_NUM	100

/**
 * enum mt76_testmode_list_act - list mode action
 *
 * @MT76_TM_LM_ACT_SET_TX_SEGMENT: set the config of a TX segment
 * @MT76_TM_LM_ACT_TX_START: start list TX
 * @MT76_TM_LM_ACT_TX_STOP: stop list TX
 * @MT76_TM_LM_ACT_SET_RX_SEGMENT: set the config of a RX segment
 * @MT76_TM_LM_ACT_RX_START: start list RX
 * @MT76_TM_LM_ACT_RX_STOP: stop list RX
 * @MT76_TM_LM_ACT_SWITCH_SEGMENT: switch TX/RX segment
 * @MT76_TM_LM_ACT_RX_STATUS: get RX status
 * @MT76_TM_LM_ACT_DUT_STATUS: get DUT status
 * @MT76_TM_LM_ACT_CLEAR_SEGMENT: clear all the TX/RX segments
 * @MT76_TM_LM_ACT_DUMP_SEGMENT: dump all the TX/RX segment settings
 */
enum mt76_testmode_list_act {
	MT76_TM_LM_ACT_SET_TX_SEGMENT,
	MT76_TM_LM_ACT_TX_START,
	MT76_TM_LM_ACT_TX_STOP,
	MT76_TM_LM_ACT_SET_RX_SEGMENT,
	MT76_TM_LM_ACT_RX_START,
	MT76_TM_LM_ACT_RX_STOP,
	MT76_TM_LM_ACT_SWITCH_SEGMENT,
	MT76_TM_LM_ACT_RX_STATUS,
	MT76_TM_LM_ACT_DUT_STATUS,
	MT76_TM_LM_ACT_CLEAR_SEGMENT,
	MT76_TM_LM_ACT_DUMP_SEGMENT,

	/* keep last */
	NUM_MT76_TM_LM_ACT,
	MT76_TM_LM_ACT_MAX = NUM_MT76_TM_LM_ACT - 1,
};

/**
 * enum mt76_testmode_fast_cal_type - fast channel calibration type
 *
 * @MT76_TM_FAST_CAL_TYPE_NONE: perform full calibration
 * @MT76_TM_FAST_CAL_TYPE_TX: fast calibration for TX verification
 * @MT76_TM_FAST_CAL_TYPE_RX: fast calibration for RX verification
 * @MT76_TM_FAST_CAL_TYPE_POWER: fast calibration for power calibration
 */
enum mt76_testmode_fast_cal_type {
	MT76_TM_FAST_CAL_TYPE_NONE,
	MT76_TM_FAST_CAL_TYPE_TX,
	MT76_TM_FAST_CAL_TYPE_RX,
	MT76_TM_FAST_CAL_TYPE_POWER,

	/* keep last */
	NUM_MT76_TM_FAST_CAL_TYPE,
	MT76_TM_FAST_CAL_TYPE_MAX = NUM_MT76_TM_FAST_CAL_TYPE - 1,
};

#endif
