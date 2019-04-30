/*
 * telescope_commands.c
 *
 *  Created on: 26 mars 2019
 *      Author: JLH
 */

#include "includes.h"

Serial_UART * pserial = &Serial_BT;

void Process_Receive_Cmd()
{

	static uint8_t stateBT = 0;
	static uint8_t cmdidxBT = 0;
	static char cmdBT[100] =
	{ 0 };
	static uint8_t cmdidx = 0;
	static char commande[100] =
	{ 0 };
	static uint8_t state = 0;

	while (Serial_Available(&Serial_STLK))
	{
		char c = '\0';
		Serial_Read(&Serial_STLK, &c);

		switch (c)
		{
		case '$':
			state = 1;
			cmdidx = 0;
			memset(commande, 0, 100); //reset de la trame
			break;
		case '\n':
			state = 2;
			break;
		default:

			break;
		}

		switch (state)
		{
		case 0:
			//Do nothing
			break;
		case 1:
			if (cmdidx < 100)
				commande[cmdidx++] = c;
			else
				state = 0;
			break;
		case 2:
			commande[cmdidx++] = '\0';

			if (Flag_bt_commands_passthrough)
			{

				if (strstr(commande, "DIS_BT_PT") != NULL)
				{
					Flag_bt_commands_passthrough = 0;
					Flag_enable_periodic_logging = 1;
				}

			}
			else
			{
				if (strstr(commande, "EN_BT_PT") != NULL)
				{
					Flag_bt_commands_passthrough = 1;
					Flag_enable_periodic_logging = 0;
				}

				//Process_Exec_Cmd(commande);
				pserial = &Serial_STLK;

				Process_Exec_CmdV2(commande + 1);

			}

			state = 0;
			memset(commande, 0, 100);
			break;

		}

		if (Flag_bt_commands_passthrough)
		{
			Serial_writeBuf(&Serial_BT, &c, 1);
		}
		else
		{
			Serial_writeBuf(&Serial_STLK, &c, 1);
		}

	}

	while (Serial_Available(&Serial_BT))
	{
		char c = '\0';
		Serial_Read(&Serial_BT, &c);
		switch (c)
		{
		case '$':
			stateBT = 1;
			cmdidxBT = 0;
			memset(cmdBT, 0, 100); //reset de la trame
			break;
		case '\n':
			stateBT = 2;
			break;
		default:

			break;
		}

		switch (stateBT)
		{
		case 0:
			//Do nothing
			break;
		case 1:
			if (cmdidxBT < 100)
				cmdBT[cmdidxBT++] = c;
			else
				stateBT = 0;
			break;
		case 2:
			cmdBT[cmdidxBT++] = '\0';
			//Process_Exec_Cmd(cmdBT);

			pserial = &Serial_BT;

			Process_Exec_CmdV2(cmdBT + 1);
			stateBT = 0;
			memset(cmdBT, 0, 100);
			break;
		}
		Serial_printf(&Serial_STLK, "%c", c);
	}
}
//OBSOLETT
#if 0
void Process_Exec_Cmd(char *cmd)
{
	char * p = NULL;
	char * p1 = NULL;

	if ((p1 = strstr(cmd, "STOP")) != NULL)
	{
		Stepper_stopMove(&stepperAh2);
		Stepper_stopMove(&stepperDec2);
	}
	else if ((p1 = strstr(cmd, "SET")) != NULL)
	{

		if ((p = strstr(p1, "DATE:")) != NULL)
		{
			p = strchr(p, ':') + 1;

			RTC_DateTypeDef d;
			d.Date = atoi(p);

			p = strchr(p, '/') + 1;
			d.Month = atoi(p);

			p = strchr(p, '/') + 1;
			d.Year = atoi(p);

			p = strchr(p, '/') + 1;
			d.WeekDay = atoi(p);

			int c = (d.Month <= 2 ? 1 : 0);
			int a = 2000 + d.Year - c;
			int m = d.Month + 12 * c - 2;
			d.WeekDay = (d.Date + a + a / 4 - a / 100 + a / 400 + (31 * m) / 12)
					% 7;

			d.WeekDay = d.WeekDay == 0 ? 7 : d.WeekDay;

			HAL_RTC_SetDate(&hrtc, &d, RTC_FORMAT_BIN);
		}
		else if ((p = strstr(p1, "TIME:")) != NULL)
		{
			p = strchr(p, ':') + 1;

			RTC_TimeTypeDef d;
			d.Hours = atoi(p);

			p = strchr(p, ':') + 1;
			d.Minutes = atoi(p);

			d.Seconds = 0;
			d.TimeFormat = RTC_HOURFORMAT_24;
			d.StoreOperation = RTC_STOREOPERATION_RESET;
			d.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;

			HAL_RTC_SetTime(&hrtc, &d, RTC_FORMAT_BIN);
		}

	}
	else if ((p1 = strstr(cmd, "TURN")) != NULL)
	{

		//struct StepperTelescope * pstep=NULL;
		struct Stepper_t * pstep = NULL;

		if ((p = strstr(cmd, "AH")) != NULL)
		{
			pstep = &stepperAh2;
		}

		else if ((p = strstr(cmd, "DEC")) != NULL)
		{
			pstep = &stepperDec2;
		}

		if (pstep == NULL)
			return;

		p = strchr(p, ':') + 1;

		int dir = atoi(p);

		p = strchr(p, ';') + 1;

		int en = atoi(p);

		if (en)
			Stepper_continuousMove(pstep, dir);
		//StepperTelescope_moveEnable(pstep, dir);
		else
			//StepperTelescope_stop(pstep);
			Stepper_stopMove(pstep);

	}

	else if ((p1 = strstr(cmd, "CALIB")) != NULL)
	{

		if ((p = strstr(p1, "AH")) != NULL)
		{
			//Calibrer
			Stepper_calibrate(&stepperAh2);
		}

		else if ((p = strstr(p1, "DEC")) != NULL)
		{
			//Calibrer
			Stepper_calibrate(&stepperDec2);

		}

	}

	else if ((p1 = strstr(cmd, "GO")) != NULL)
	{

		if ((p = strstr(p1, "AH:")) != NULL)
		{

			p = strchr(p, ':') + 1;

			double pos = atof(p);

			//pos = stepperAh2.ref_angle + pos;

			Stepper_goTo(&stepperAh2, pos / stepperAh2.degreesPerSteps);
		}

		else if ((p = strstr(p1, "DEC:")) != NULL)
		{
			p = strchr(p, ':') + 1;

			double pos = atof(p);

			Stepper_goTo(&stepperDec2, pos / stepperDec2.degreesPerSteps);
		}
	}
	else if ((p1 = strstr(cmd, "STAR")) != NULL)
	{
		p = strchr(p1, ':') + 1;
		//Mode parsing DEG/DEG
		if (strchr(p, 'h') == NULL)
		{

			if (p == NULL)
				return;

			double ra = atof(p);

			p = strchr(p, '/') + 1;

			if (p == NULL)
				return;

			double dec = atof(p);

			starSkyCoords.ra = ra;
			starSkyCoords.dec = dec;

			Serial_printf(&Serial_STLK, "Star set to ra=%lf dec=%lf\r\n",
					starSkyCoords.ra, starSkyCoords.dec);
		}
		else
		{
			//Mode parsing HMS/DMS
			StarMap_HMS_t ra =
			{	.h = 0, .m = 0, .s = 0};

			StarMap_DMS_t dec =
			{	.d = 0, .m = 0, .s = 0};

			ra.h = atoi(p);

			p = strchr(p, 'h') + 1;

			ra.m = atoi(p);

			p = strchr(p, 'm') + 1;

			ra.s = atof(p);

			p = strchr(p, '/') + 1; //separateur ra/dec

			dec.d = atoi(p);

			p = strchr(p, '°') + 1;

			dec.m = atoi(p);

			p = strchr(p, '\'') + 1;

			dec.s = atof(p);

			//Serial_printf(&Serial_UART2, "Star set to ra=%dh%02dm%0.2lfs dec=%d°%02d'%0.2lf\"\r\n",
			//					ra.h,ra.m,ra.s,dec.d,dec.m,dec.s);

			starSkyCoords.ra = StarMap_hmsToDeg(ra);
			starSkyCoords.dec = StarMap_dmsToDeg(dec);

			Serial_printf(&Serial_STLK, "Star set to ra=%s dec=%s\r\n",
					StarMap_ascAngle(starSkyCoords.ra, HMS),
					StarMap_ascAngle(starSkyCoords.dec, DMS));
		}

	}
	else if ((p1 = strstr(cmd, "FOLLOW")) != NULL)
	{
		p = strchr(p1, ':') + 1;

		uint8_t enable = atoi(p);

		if (enable)
		{
			Flag_enable_star_follow = 1;
		}
		else
		{
			Flag_enable_star_follow = 0;
		}

	}

}

#endif

void Process_Axis_Cmd(char * cmd)
{

	switch (cmd[0])
	{
	case 'w': //Write parameter
	{

		struct Stepper_t * pstep = NULL;
		switch (cmd[2])
		//Selected axis
		{
		case 'a': //Ah axis
			pstep = &stepperAh2;
			break;
		case 'd': //Dec axis
			pstep = &stepperDec2;
			break;

		default:
			//Serial_printf(pserial, "ERR\r\n");
			goto err;
			return;
			break;
		}

		switch (cmd[1])
		//Selected parameter
		{
		case 'p': //Write position
		{
			double pos = atof(cmd + 3);

			if (cmd[2] == 'a')
			{

				pos = getRelativeAngleFromAh(pos, needsReturnedMode(pos));

			}
			else if (cmd[2] == 'd')
			{

				pos = getRelativeAngleFromDec(pos, 0);

			}

			pos = Stepper_getAbsoluteFromRelative(pstep, pos);
			Stepper_goTo(pstep, pos / pstep->degreesPerSteps);

			Flag_enable_star_follow = 0; //Disable automatic star following.

			//Serial_printf(pserial, "OK\r\n");
			goto ok;

		}
		break;
		default:
			break;
		}

	}
	break;

	case 'c': //Calibrate axis
	{

		switch (cmd[1])
		{
		case 'a': //Ah axis

			Stepper_calibrate(&stepperAh2);

			break;
		case 'd': //Dec axis

			Stepper_calibrate(&stepperDec2);
			break;

		default:
			//Serial_printf(pserial, "ERR\r\n");
			goto err;

			return;
			break;
		}

		Flag_enable_star_follow = 0; //Disable automatic star following.
		//Serial_printf(pserial, "OK\r\n");
		goto ok;

	}
	break;

	case 'i': //Increment position
	{

		//int steps = atoi(cmd + 2);

		double angle = strtod(cmd+2,NULL);


		switch (cmd[1])
		{
		case 'a': //Ah axis

			//Stepper_move(&stepperAh2, steps);
			Stepper_move(&stepperAh2, angle/stepperAh2.degreesPerSteps);

			break;
		case 'd': //Dec axis

			//Stepper_move(&stepperDec2, steps);
			Stepper_move(&stepperDec2, angle/stepperDec2.degreesPerSteps);
			break;

		default:
			//Serial_printf(pserial, "ERR\r\n");
			goto err;

			return;
			break;
		}

		Flag_enable_star_follow = 0; //Disable automatic star following.
		//Serial_printf(pserial, "OK\r\n");

		goto ok;

	}
	break;

	case 'm': //Rotate axis
	{

		struct Stepper_t * pstepper = NULL;

		switch (cmd[1])
		//Axis selected
		{
		case 'a': //Ah axis
			pstepper = &stepperAh2;
			break;
		case 'd': //Dec axis
			pstepper = &stepperDec2;
			break;

		default: //No stepper selected, return here
			//Serial_printf(pserial, "ERR\r\n");
			goto err;
			return;
			break;
		}

		switch (cmd[2])
		//Axis direction
		{
		case '+': //direction axis +
			Stepper_continuousMove(pstepper, 1);
			break;
		case '-': //direction axis -
			Stepper_continuousMove(pstepper, 0);
			break;
		default:
			//Serial_printf(pserial, "ERR\r\n");
			goto err;

			return;
			break;
		}

		Flag_enable_star_follow = 0; //Disable automatic star following.
		//Serial_printf(pserial, "OK\r\n");

		goto ok;

	}
	break;

	case 'p': //Read Position
	{

		struct Stepper_t * pstepper = NULL;

		double relativePosDec=Stepper_getRelativePos(&stepperDec2);
		double relativePosAh=Stepper_getRelativePos(&stepperAh2);

		double ah=getAhAngleFromRelative(relativePosAh, relativePosDec >= 0);

		double dec=getDecAngleFromRelative(relativePosDec, relativePosDec >= 0);

		switch (cmd[1])
		//Axis selected
		{
		case 'a': //Ah axis
			Serial_printf(pserial, "%lf\r\n", ah);
			break;
		case 'd': //Dec axis

			Serial_printf(pserial, "%lf\r\n", dec);
			break;

		default: //No stepper selected, return here
			//Serial_printf(pserial, "ERR\r\n");
			goto err;

			return;
			break;
		}






	}
	break;

	case 's': //Stop axis rotation
	{

		switch (cmd[1])
		{
		case 'a': //Ah axis
			Stepper_stopMove(&stepperAh2);
			break;
		case 'd': //Dec axis
			Stepper_stopMove(&stepperDec2);
			break;

		default: //Both axies
			Stepper_stopMove(&stepperAh2);
			Stepper_stopMove(&stepperDec2);
			break;
		}

		Flag_enable_star_follow = 0; //Disable automatic star following.
		//Serial_printf(pserial, "OK\r\n");
		goto ok;

	}
	break;

	}

	return;

	ok: Serial_printf(pserial, "OK\r\n");
	return;

	err: Serial_printf(pserial, "ERR\r\n");
	return;
}

void Process_GPS_Param_Cmd(char * cmd)
{

	switch (cmd[0])
	{
	case 'l': //loc, lon,lat, locs commands

		if (strstr(cmd, "loc") != NULL)
		{
			if (cmd[3] == 's') //Locs command
			{
				if (strstr(cmd + 3, "CMD") != NULL)
				{
					Flag_enable_LOC_GPS_sync = 0;
					goto ok;
					return;
				}
				else if (strstr(cmd + 3, "GPS") != NULL)
				{
					Flag_enable_LOC_GPS_sync = 1;
					goto ok;
					return;
				}
				else
				{
					goto err;
					return;
				}
			}
			else //Loc command
			{

				//Position geographic DMS/DMS
				//Write the star location
				char * p = cmd;

				if ((p = strchr(p, ':')) == NULL)
					goto err;
				p++;

				//Mode parsing DEG/DEG
				if (strchr(p, '°') == NULL)
				{

					double lon = atof(p);

					if ((p = strchr(p, '/')) == NULL)
						goto err;

					p++;
					double lat = atof(p);

					Serial_printf(pserial, "OK lon=%lf lat=%lf\r\n", lon, lat);

					geographical_location.lon = lon;
					geographical_location.lat = lat;

					RTCUtil_saveData(RTC_BKP_DR0,
							(uint8_t*) &geographical_location,
							sizeof(geographical_location));
					StarMap_UpdateLocation(geographical_location);

				}
				else
				{
					//Mode parsing DMS/DMS
					StarMap_DMS_t lon =
					{ .d = 0, .m = 0, .s = 0 };

					StarMap_DMS_t lat =
					{ .d = 0, .m = 0, .s = 0 };

					if (StarMap_parseDMS(p, &lon) == STARMAP_ERR)
						goto err;

					if ((p = strchr(p, '/')) == NULL)
						goto err;

					p++;

					if (StarMap_parseDMS(p, &lat) == STARMAP_ERR)
						goto err;

					Serial_printf(pserial, "OK lon=%s", StarMap_ascDMS(lon));
					Serial_printf(pserial, " lat=%s\r\n", StarMap_ascDMS(lat));

					geographical_location.lon = StarMap_dmsToDeg(lon);
					geographical_location.lat = StarMap_dmsToDeg(lat);

					RTCUtil_saveData(RTC_BKP_DR0,
							(uint8_t*) &geographical_location,
							sizeof(geographical_location));
					StarMap_UpdateLocation(geographical_location);
				}

			}
		}
		else
		{

		}

		break;

	case 'u': //utc or utcs command

		if (cmd[3] == 's') //utcs
		{
			if (strstr(cmd + 3, "CMD") != NULL)
			{
				Flag_enable_RTC_GPS_sync = 0;
				goto ok;
				return;
			}
			else if (strstr(cmd + 3, "GPS") != NULL)
			{
				Flag_enable_RTC_GPS_sync = 1;
				goto ok;
				return;
			}
			else
			{
				goto err;
				return;
			}
		}
		else //utc
		{

			//1994-11-05T13:15:30Z
			char * p = cmd + 3;
			p = strchr(p, ':') + 1;

			RTC_DateTypeDef d;
			d.Year = atoi(p) % 100;

			p = strchr(p, '-') + 1;
			d.Month = atoi(p);

			p = strchr(p, '-') + 1;
			d.Date = atoi(p);

			int c = (d.Month <= 2 ? 1 : 0);
			int a = 2000 + d.Year - c;
			int m = d.Month + 12 * c - 2;
			d.WeekDay = (d.Date + a + a / 4 - a / 100 + a / 400 + (31 * m) / 12)
							% 7;

			d.WeekDay = d.WeekDay == 0 ? 7 : d.WeekDay;

			HAL_RTC_SetDate(&hrtc, &d, RTC_FORMAT_BIN);

			RTC_TimeTypeDef t;

			p = strchr(p, 'T') + 1;

			t.Hours = atoi(p);

			p = strchr(p, ':') + 1;
			t.Minutes = atoi(p);

			p = strchr(p, ':') + 1;

			t.Seconds = atoi(p);

			t.TimeFormat = RTC_HOURFORMAT_24;
			t.StoreOperation = RTC_STOREOPERATION_RESET;
			t.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;

			HAL_RTC_SetTime(&hrtc, &t, RTC_FORMAT_BIN);

			goto ok;
		}

		break;

	default:
		break;

	}

	return;

	ok: Serial_printf(pserial, "OK\r\n");
	return;

	err: Serial_printf(pserial, "ERR\r\n");
	return;
}

void Process_GPS_Cmd(char * cmd)
{

	switch (cmd[0])
	{
	case 'l': //loc, lon,lat, locs commands

		if (strstr(cmd, "loc") != NULL)
		{
			if (cmd[3] == 's') //Locs command
			{
				Serial_printf(pserial,
						Flag_enable_LOC_GPS_sync ? "GPS\r\n" : "CMD\r\n");
			}
			else //Loc command
			{
				Serial_printf(pserial, "lon=%lf lat=%lf\r\n",
						geographical_location.lon, geographical_location.lat);
			}
		}
		else if (strstr(cmd, "lon") != NULL)
		{
			Serial_printf(pserial, "lon=%lf\r\n", geographical_location.lon);
		}
		else if (strstr(cmd, "lat") != NULL)
		{
			Serial_printf(pserial, "lat=%lf\r\n", geographical_location.lat);
		}
		else
		{

		}

		break;

	case 'u': //utc or utcs command

		if (cmd[3] == 's') //utcs
		{
			Serial_printf(pserial,
					Flag_enable_RTC_GPS_sync ? "GPS\r\n" : "CMD\r\n");
		}
		else //utc
		{
			Serial_printf(pserial, "UTC:%02d/%02d/%02d %02d:%02d\r\n",
					RTCUtil_dateRtc.Date, RTCUtil_dateRtc.Month,
					RTCUtil_dateRtc.Year, RTCUtil_timeRtc.Hours,
					RTCUtil_timeRtc.Minutes, RTCUtil_timeRtc.Seconds);
		}

		break;
	case 's': //gst command

		break;
	case 'w': //Write parameter
	{
		Process_GPS_Param_Cmd(cmd + 1);
	}

	break;
	default:
		break;

	}

}

void Process_Star_Cmd(char * cmd)
{

	switch (cmd[0])
	{
	case 'l': //Star Loc command

		Serial_printf(pserial, "Star ra=%s dec=%s\r\n",
				StarMap_ascAngle(starSkyCoords.ra, HMS),
				StarMap_ascAngle(starSkyCoords.dec, DMS));
		break;
	case 'c':
		Serial_printf(pserial, "Corr ra=%s dec=%s\r\n",
				StarMap_ascAngle(starCorrectionCoords.ra, HMS),
				StarMap_ascAngle(starCorrectionCoords.dec, DMS));
		break;
	case 'w': //Star param write
	{
		char * p = NULL;

		//Write the star location
		if ((p = strstr(cmd, "loc")) != NULL)
		{

			p = strchr(p, ':') + 1;
			//Mode parsing DEG/DEG
			if (strchr(p, 'h') == NULL)
			{

				if (p == NULL)
					goto err;

				double ra = atof(p);

				p = strchr(p, '/') + 1;

				if (p == NULL)
					goto err;

				double dec = atof(p);

				starSkyCoords.ra = ra;
				starSkyCoords.dec = dec;

				Serial_printf(pserial, "Star loc SET ra=%lf dec=%lf\r\n",
						starSkyCoords.ra, starSkyCoords.dec);
			}
			else
			{
				//Mode parsing HMS/DMS
				StarMap_HMS_t ra =
				{ .h = 0, .m = 0, .s = 0 };

				StarMap_DMS_t dec =
				{ .d = 0, .m = 0, .s = 0 };

#if 0
				ra.h = atoi(p);

				if ((p = strchr(p, 'h')) == NULL)
					goto err;

				p++;
				ra.m = atoi(p);

				if ((p = strchr(p, 'm')) == NULL)
					goto err;

				p++;
				ra.s = atof(p);

				if ((p = strchr(p, '/')) == NULL)
					goto err;

				p++;

				dec.d = atoi(p);

				if ((p = strchr(p, '°')) == NULL)
					goto err;

				p++;

				dec.m = atoi(p);

				if ((p = strchr(p, '\'')) == NULL)
					goto err;

				p++;

				dec.s = atof(p);
#endif


				if (StarMap_parseHMS(p, &ra) == STARMAP_ERR)
					goto err;

				if ((p = strchr(p, '/')) == NULL)
					goto err;

				p++;

				if (StarMap_parseDMS(p, &dec) == STARMAP_ERR)
					goto err;

				starSkyCoords.ra = StarMap_hmsToDeg(ra);
				starSkyCoords.dec = StarMap_dmsToDeg(dec);

				Serial_printf(pserial, "Star loc SET ra=%s dec=%s\r\n",
						StarMap_ascAngle(starSkyCoords.ra, HMS),
						StarMap_ascAngle(starSkyCoords.dec, DMS));
			}

		}
		else if ((p = strstr(cmd, "cor")) != NULL)
		{

			p = strchr(p, ':') + 1;
			//Mode parsing DEG/DEG
			if (strchr(p, 'h') == NULL)
			{

				if (p == NULL)
					goto err;

				double ra = atof(p);

				p = strchr(p, '/') + 1;

				if (p == NULL)
					goto err;

				double dec = atof(p);

				starCorrectionCoords.ra = ra;
				starCorrectionCoords.dec = dec;

				Serial_printf(pserial, "Star corr SET ra=%lf dec=%lf\r\n",
						starCorrectionCoords.ra, starCorrectionCoords.dec);

			}
			else
			{
				//Mode parsing HMS/DMS
				StarMap_HMS_t ra =
				{ .h = 0, .m = 0, .s = 0 };

				StarMap_DMS_t dec =
				{ .d = 0, .m = 0, .s = 0 };

				ra.h = atoi(p);

				if ((p = strchr(p, 'h')) == NULL)
					goto err;

				p++;
				ra.m = atoi(p);

				if ((p = strchr(p, 'm')) == NULL)
					goto err;

				p++;
				ra.s = atof(p);

				if ((p = strchr(p, '/')) == NULL)
					goto err;

				p++;

				dec.d = atoi(p);

				if ((p = strchr(p, '°')) == NULL)
					goto err;

				p++;

				dec.m = atoi(p);

				if ((p = strchr(p, '\'')) == NULL)
					goto err;

				p++;

				dec.s = atof(p);

				starCorrectionCoords.ra = StarMap_hmsToDeg(ra);
				starCorrectionCoords.dec = StarMap_dmsToDeg(dec);

				Serial_printf(pserial, "Star corr SET ra=%s dec=%s\r\n",
						StarMap_ascAngle(starCorrectionCoords.ra, HMS),
						StarMap_ascAngle(starCorrectionCoords.dec, DMS));

			}

		}
	}
	break;
	case 'f': //Start follow enable/disable

		if (cmd[1] == 'd')
		{
			Flag_enable_star_follow = 0;

			goto ok;
		}
		else if (cmd[1] == 'e')
		{
			Flag_enable_star_follow = 1;

			if (Flag_needs_axis_flipping)
			{
				Flag_authorize_axis_flipping = 1;
			}

			goto ok;
		}
		else
		{
			goto err;
		}
		break;
	}

	return;

	ok: Serial_printf(pserial, "OK\r\n");
	return;

	err: Serial_printf(pserial, "ERR\r\n");
	return;
}

void Process_Sys_Cmd(char * cmd)
{

	switch (cmd[0])
	{
	case 'm': //bmv command
		if (cmd[1] == 'v')
		{
			Serial_printf(pserial, "%dmV\r\n", Get_BatteryMilliVolts());
		}
		break;
	case 's': //bmv command
		if (cmd[1] == 't')
		{
			unsigned char sysFlags = (Flag_needs_axis_flipping ? 1 : 0)
							| (nmea_rcvr.flag_is_gps_active ? 1 : 0) << 1
							| (nmea_location_rmc.valid == 'A' ? 1 : 0) << 2
							| (Flag_enable_periodic_logging ? 1 : 0) << 3
							| (Flag_enable_RTC_GPS_sync ? 1 : 0) << 4
							| (Flag_enable_LOC_GPS_sync ? 1 : 0) << 5
							| (Flag_enable_star_follow ? 1 : 0) << 6;

			Serial_printf(pserial, "%X\r\n", sysFlags);
		}
		break;

	}

	return;

	ok: Serial_printf(pserial, "OK\r\n");
	return;

	err: Serial_printf(pserial, "ERR\r\n");
	return;
}

void Process_Exec_CmdV2(char * cmd)
{

	switch (cmd[0])
	{
	case 'a': //Axis commands

		Process_Axis_Cmd(cmd + 1);
		return;
		break;

	case 'g': //GPS commands

		Process_GPS_Cmd(cmd + 1);
		return;
		break;
	case 's': //Star commands

		Process_Star_Cmd(cmd + 1);
		return;
		break;
	case 'b':
		Process_Sys_Cmd(cmd + 1);
		break;
	default:
		break;
	}

}
