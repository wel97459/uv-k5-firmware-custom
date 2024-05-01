/* Original work Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Modified work Copyright 2024 kamilsss655
 * https://github.com/kamilsss655
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <string.h>

#ifdef ENABLE_FMRADIO
	#include "app/fm.h"
#endif
#include "driver/eeprom.h"
#include "driver/uart.h"
#include "driver/bk4819.h"
#include "misc.h"
#include "settings.h"
#include "board.h"

#ifdef ENABLE_ENCRYPTION
	#include "helper/crypto.h"
#endif

EEPROM_Config_t gEeprom;

void SETTINGS_SaveVfoIndices(void)
{
	uint8_t State[8];

	#ifndef ENABLE_NOAA
		EEPROM_ReadBuffer(0x0E80, State, sizeof(State));
	#endif

	State[0] = gEeprom.ScreenChannel[0];
	State[1] = gEeprom.MrChannel[0];
	State[2] = gEeprom.FreqChannel[0];
	State[3] = gEeprom.ScreenChannel[1];
	State[4] = gEeprom.MrChannel[1];
	State[5] = gEeprom.FreqChannel[1];
	#ifdef ENABLE_NOAA
		State[6] = gEeprom.NoaaChannel[0];
		State[7] = gEeprom.NoaaChannel[1];
	#endif

	EEPROM_WriteBuffer(0x0E80, State, true);
}

void SETTINGS_SaveSettings(void)
{
	uint8_t  State[8];

	State[0] = gEeprom.CHAN_1_CALL;
	State[1] = gEeprom.SQUELCH_LEVEL;
	State[2] = gEeprom.TX_TIMEOUT_TIMER;
	#ifdef ENABLE_NOAA
		State[3] = gEeprom.NOAA_AUTO_SCAN;
	#else
		State[3] = false;
	#endif
	State[4] = gEeprom.KEY_LOCK;
	#ifdef ENABLE_VOX
		State[5] = gEeprom.VOX_SWITCH;
		State[6] = gEeprom.VOX_LEVEL;
	#else
		State[5] = false;
		State[6] = 0;
	#endif
	State[7] = gEeprom.MIC_SENSITIVITY;
	EEPROM_WriteBuffer(0x0E70, State, true);

	State[0] = (gEeprom.BACKLIGHT_MIN << 4) + gEeprom.BACKLIGHT_MAX;
	State[1] = gEeprom.CHANNEL_DISPLAY_MODE;
	State[2] = gEeprom.CROSS_BAND_RX_TX;
	State[3] = gEeprom.BATTERY_SAVE;
	State[4] = gEeprom.DUAL_WATCH;
	State[5] = gEeprom.BACKLIGHT_TIME;
	State[7] = gEeprom.VFO_OPEN;
	EEPROM_WriteBuffer(0x0E78, State, true);

	State[0] = gEeprom.BEEP_CONTROL;
	State[0] |= gEeprom.KEY_M_LONG_PRESS_ACTION << 1;
	State[1] = gEeprom.KEY_1_SHORT_PRESS_ACTION;
	State[2] = gEeprom.KEY_1_LONG_PRESS_ACTION;
	State[3] = gEeprom.KEY_2_SHORT_PRESS_ACTION;
	State[4] = gEeprom.KEY_2_LONG_PRESS_ACTION;
	State[5] = gEeprom.SCAN_RESUME_MODE;
	State[6] = gEeprom.AUTO_KEYPAD_LOCK;
	State[7] = gEeprom.POWER_ON_DISPLAY_MODE;
	EEPROM_WriteBuffer(0x0E90, State, true);

	// 0x0E98..0x0E9F
	memset(State, 0xFF, sizeof(State));
	EEPROM_ReadBuffer(0x0E98, State, 8);
	#ifdef ENABLE_PWRON_PASSWORD
		memcpy(&State[0], &gEeprom.POWER_ON_PASSWORD, 4);
			#endif
	memcpy(&State[4], &gEeprom.RX_OFFSET, 4);
	EEPROM_WriteBuffer(0x0E98, State, true);

	memset(State, 0xFF, sizeof(State));
	#ifdef ENABLE_VOX
		State[0] = gEeprom.VOX_DELAY;
	#endif
	State[1] = gEeprom.RX_AGC;
	#ifdef ENABLE_PWRON_PASSWORD
		State[2] = gEeprom.PASSWORD_WRONG_ATTEMPTS;
	#endif
	#ifdef ENABLE_MESSENGER
		State[3] = gEeprom.MESSENGER_CONFIG.__val;
	#endif
	EEPROM_WriteBuffer(0x0EA0, State, true);

	memset(State, 0xFF, sizeof(State));
	#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
		State[0] = gEeprom.ALARM_MODE;
	#else
		State[0] = false;
	#endif
	State[1] = gEeprom.ROGER;
	// State[2] = empty slot
	State[3] = gEeprom.TX_VFO;
	State[4] = gEeprom.BATTERY_TYPE;
	State[5] = gEeprom.SQL_TONE;
	EEPROM_WriteBuffer(0x0EA8, State, true);

	State[0] = gEeprom.DTMF_SIDE_TONE;
#ifdef ENABLE_DTMF_CALLING
	State[1] = gEeprom.DTMF_SEPARATE_CODE;
	State[2] = gEeprom.DTMF_GROUP_CALL_CODE;
	State[3] = gEeprom.DTMF_DECODE_RESPONSE;
	State[4] = gEeprom.DTMF_auto_reset_time;
#endif
	State[5] = gEeprom.DTMF_PRELOAD_TIME / 10U;
	State[6] = gEeprom.DTMF_FIRST_CODE_PERSIST_TIME / 10U;
	State[7] = gEeprom.DTMF_HASH_CODE_PERSIST_TIME / 10U;
	EEPROM_WriteBuffer(0x0ED0, State, true);

	memset(State, 0xFF, sizeof(State));
	State[0] = gEeprom.DTMF_CODE_PERSIST_TIME / 10U;
	State[1] = gEeprom.DTMF_CODE_INTERVAL_TIME / 10U;
#ifdef ENABLE_DTMF_CALLING
	State[2] = gEeprom.PERMIT_REMOTE_KILL;
#endif
	EEPROM_WriteBuffer(0x0ED8, State, true);

	State[0] = gEeprom.SCAN_LIST_DEFAULT;
	State[1] = gEeprom.SCAN_LIST_ENABLED[0];
	State[2] = gEeprom.SCANLIST_PRIORITY_CH1[0];
	State[3] = gEeprom.SCANLIST_PRIORITY_CH2[0];
	State[4] = gEeprom.SCAN_LIST_ENABLED[1];
	State[5] = gEeprom.SCANLIST_PRIORITY_CH1[1];
	State[6] = gEeprom.SCANLIST_PRIORITY_CH2[1];
	State[7] = 0xFF;
	EEPROM_WriteBuffer(0x0F18, State, true);

	memset(State, 0xFF, sizeof(State));
	State[0]  = gSetting_F_LOCK;
	State[1]  = gSetting_350TX;
#ifdef ENABLE_DTMF_CALLING
	State[2]  = gSetting_KILLED;
#endif
	State[3]  = gSetting_200TX;
	State[4]  = gSetting_500TX;
	State[5]  = gSetting_350EN;
	State[6]  = (gSetting_ScrambleEnable & 0xf) | ((gSetting_MURSTX & 0xf) << 4);
	//if (!gSetting_TX_EN)             State[7] &= ~(1u << 0);
	if (!gSetting_live_DTMF_decoder) State[7] &= ~(1u << 1);
	State[7] = (State[7] & ~(3u << 2)) | ((gSetting_battery_text & 3u) << 2);
	State[7] = (State[7] & ~(3u << 6)) | ((gSetting_backlight_on_tx_rx & 3u) << 6);
	EEPROM_WriteBuffer(0x0F40, State, true);

	#ifdef ENABLE_FMRADIO
		//0x0E88..0x0E8F
		memset(State, 0xFF, sizeof(State));
		memcpy(&State[0], &gEeprom.FM_FrequencyPlaying, 2);
		EEPROM_WriteBuffer(0x0E88, State, true);
	#endif

	#ifdef ENABLE_ENCRYPTION
		SETTINGS_SaveEncryptionKey();
	#endif

	#ifdef ENABLE_MESSENGER_ID
		EEPROM_WriteBuffer(0x0F20, gEeprom.MSG_ID, true);
		EEPROM_WriteBuffer(0x0F28, gEeprom.MSG_ID + 8, true);
	#endif
}

void SETTINGS_SaveChannel(uint8_t Channel, uint8_t VFO, const VFO_Info_t *pVFO, uint8_t Mode)
{
	#ifdef ENABLE_NOAA
		if (!IS_NOAA_CHANNEL(Channel))
	#endif
	{
		uint16_t OffsetVFO = Channel * 16;

		if (!IS_MR_CHANNEL(Channel))
		{	// it's a VFO, not a channel
			OffsetVFO  = (VFO == 0) ? 0x0C80 : 0x0C90;
			OffsetVFO += (Channel - FREQ_CHANNEL_FIRST) * 32;
		}

		if (Mode >= 2 || !IS_MR_CHANNEL(Channel))
		{	// copy VFO to a channel

			uint8_t State[8];

			((uint32_t *)State)[0] = pVFO->freq_config_RX.Frequency;
			((uint32_t *)State)[1] = pVFO->TX_OFFSET_FREQUENCY;
			EEPROM_WriteBuffer(OffsetVFO + 0, State, true);

			State[0] =  pVFO->freq_config_RX.Code;
			State[1] =  pVFO->freq_config_TX.Code;
			State[2] = (pVFO->freq_config_TX.CodeType << 4) | pVFO->freq_config_RX.CodeType;
			State[3] = (pVFO->Modulation << 4) | pVFO->TX_OFFSET_FREQUENCY_DIRECTION;
			State[4] = 0
				| (pVFO->BUSY_CHANNEL_LOCK << 4)
				| (pVFO->OUTPUT_POWER      << 2)
				| ((pVFO->CHANNEL_BANDWIDTH != BK4819_FILTER_BW_WIDE) << 1)
				| (pVFO->FrequencyReverse  << 0);
			if(pVFO->CHANNEL_BANDWIDTH != BK4819_FILTER_BW_WIDE)
				State[4] |= ((pVFO->CHANNEL_BANDWIDTH - 1) << 5);
			State[5] = ((pVFO->DTMF_PTT_ID_TX_MODE & 7u) << 1)
#ifdef ENABLE_DTMF_CALLING
				| ((pVFO->DTMF_DECODING_ENABLE & 1u) << 0)
#endif
			;
			State[6] =  pVFO->STEP_SETTING;
			State[7] =  pVFO->SCRAMBLING_TYPE;
			EEPROM_WriteBuffer(OffsetVFO + 8, State, true);

			SETTINGS_UpdateChannel(Channel, pVFO, true);

			if (IS_MR_CHANNEL(Channel))
			{	// it's a memory channel

				#ifndef ENABLE_KEEP_MEM_NAME
					// clear/reset the channel name
					SETTINGS_SaveChannelName(Channel, "");
				#else
					if (Mode >= 3) {
						SETTINGS_SaveChannelName(Channel, pVFO->Name);

						#ifdef ENABLE_SPECTRUM_SHOW_CHANNEL_NAME
							//update channel names stored in memory
							BOARD_gMR_LoadChannels();
						#endif
					}
				#endif
			}
		}
	}
}

void SETTINGS_SaveBatteryCalibration(const uint16_t * batteryCalibration)
{
	uint16_t buf[4];
	EEPROM_WriteBuffer(0x1F40, batteryCalibration, false);
	EEPROM_ReadBuffer( 0x1F48, buf, sizeof(buf));
	buf[0] = batteryCalibration[4];
	buf[1] = batteryCalibration[5];
	EEPROM_WriteBuffer(0x1F48, buf, false);
}

void SETTINGS_SaveChannelName(uint8_t channel, const char * name)
{
	uint16_t offset = channel * 16;
	uint8_t  buf[16];
	memset(&buf, 0x00, sizeof(buf));
	memcpy(buf, name, MIN(strlen(name),10u));
	EEPROM_WriteBuffer(0x0F50 + offset, buf, true);
	EEPROM_WriteBuffer(0x0F58 + offset, buf + 8, true);
}

#ifdef ENABLE_ENCRYPTION
void SETTINGS_SaveEncryptionKey()
{
	EEPROM_WriteBuffer(0x0F30, gEeprom.ENC_KEY, true);
	EEPROM_WriteBuffer(0x0F38, gEeprom.ENC_KEY + 8, true);
	gRecalculateEncKey = true;
}
#endif

void SETTINGS_FetchChannelName(char *s, const int channel)
{
	int i;

	if (s == NULL)
		return;

	memset(s, 0, 11);  // 's' had better be large enough !

	if (channel < 0)
		return;

	if (!RADIO_CheckValidChannel(channel, false, 0))
		return;


	EEPROM_ReadBuffer(0x0F50 + (channel * 16), s + 0, 8);
	EEPROM_ReadBuffer(0x0F58 + (channel * 16), s + 8, 2);

	for (i = 0; i < 10; i++)
		if (s[i] < 32 || s[i] > 127)
			break;                // invalid char

	s[i--] = 0;                   // null term

	while (i >= 0 && s[i] == 32)  // trim trailing spaces
		s[i--] = 0;               // null term
}

void SETTINGS_UpdateChannel(uint8_t channel, const VFO_Info_t *pVFO, bool keep)
{
#ifdef ENABLE_NOAA
	if (!IS_NOAA_CHANNEL(channel))
#endif
	{
		uint8_t  state[8];
		ChannelAttributes_t  att = {
			.band = 0xf,
			.compander = 0,
			.scanlist1 = 0,
			.scanlist2 = 0,
			};        // default attributes

		uint16_t offset = 0x0D60 + (channel & ~7u);
		EEPROM_ReadBuffer(offset, state, sizeof(state));

		if (keep) {
			att.band = pVFO->Band;
			att.scanlist1 = pVFO->SCANLIST1_PARTICIPATION;
			att.scanlist2 = pVFO->SCANLIST2_PARTICIPATION;
			att.compander = pVFO->Compander;
			if (state[channel & 7u] == att.__val)
				return; // no change in the attributes
		}

		state[channel & 7u] = att.__val;
		EEPROM_WriteBuffer(offset, state, true);

		gMR_ChannelAttributes[channel] = att;

		if (IS_MR_CHANNEL(channel)) {	// it's a memory channel
			if (!keep) {
				// clear/reset the channel name
				SETTINGS_SaveChannelName(channel, "");
			}
		}
	}
}

void SETTINGS_SetVfoFrequency(uint32_t frequency) {
	const uint8_t Vfo = gEeprom.TX_VFO;
	// clamp the frequency entered to some valid value
	if (frequency < RX_freq_min())
	{
		frequency = RX_freq_min();
	}
	else
	if (frequency >= BX4819_band1.upper && frequency < BX4819_band2.lower)
	{
		const uint32_t center = (BX4819_band1.upper + BX4819_band2.lower) / 2;
		frequency = (frequency < center) ? BX4819_band1.upper : BX4819_band2.lower;
	}
	else
	if (frequency > frequencyBandTable[ARRAY_SIZE(frequencyBandTable) - 1].upper)
	{
		frequency = frequencyBandTable[ARRAY_SIZE(frequencyBandTable) - 1].upper;
	}

	{
		const FREQUENCY_Band_t band = FREQUENCY_GetBand(frequency);

		#ifdef ENABLE_VOICE
			gAnotherVoiceID = (VOICE_ID_t)Key;
		#endif

		if (gTxVfo->Band != band)
		{
			gTxVfo->Band               = band;
			gEeprom.ScreenChannel[Vfo] = band + FREQ_CHANNEL_FIRST;
			gEeprom.FreqChannel[Vfo]   = band + FREQ_CHANNEL_FIRST;

			SETTINGS_SaveVfoIndices();

			RADIO_ConfigureChannel(Vfo, VFO_CONFIGURE_RELOAD);
		}

		// Autoset stepFrequency based on step setting
		gTxVfo->StepFrequency = gStepFrequencyTable[gTxVfo->STEP_SETTING];

		frequency = FREQUENCY_RoundToStep(frequency, gTxVfo->StepFrequency);

		if (frequency >= BX4819_band1.upper && frequency < BX4819_band2.lower)
		{	// clamp the frequency to the limit
			const uint32_t center = (BX4819_band1.upper + BX4819_band2.lower) / 2;
			frequency = (frequency < center) ? BX4819_band1.upper - gTxVfo->StepFrequency : BX4819_band2.lower;
		}

		gTxVfo->freq_config_RX.Frequency = frequency;
	}
}