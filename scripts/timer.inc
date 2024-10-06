#include <timerfix>


new admin_name_color_change[MAX_PLAYERS];
new wtimecount, wroldtime, worldweather;
new dytimerc[2];

timerInit() {
    wroldtime = 0;
    worldweather = -1;
    wtimecount = 0;
    dytimerc[0] = SetTimer("OnTimerOneSec", 1000, true);
	dytimerc[1] = SetTimer("OnTimerTHM", 200, true);
    CreateDynamicObject(19945, 2000.95349, 1534.79712, -46.85384,   0.00000, 0.00000, 0.00000);
}

timerExit() {
	for(new i = 0; i < sizeof dytimerc; i ++) {
		if(IsValidTimer(dytimerc[i])) {
			KillTimer(dytimerc[i]);
		}
    }
}

timer_GMSetWeather(weather) {
    worldweather = weather;
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1 || GetPlayerStatus(i) == 1) {
            SetPlayerWeatherEx(i);
        }
    }
}

timer_GMSetTime(time) {
    wroldtime = time;
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1 || GetPlayerStatus(i) == 1) {
            SetPlayerTimeEx(i);
        }
    }
}

stock SetPlayerTimeEx(playerid, time = -1) {
    if(time == -1) {
        if(userInfo[playerid][SZ_Value][3] == 0) {
            time = wroldtime;
        } else {
            time = userInfo[playerid][SZ_Value][5];
        }
    }
    SetPlayerTime(playerid, time, 0);
    if(GetPlayerTRTimeStatus(playerid) == 0) {
        new strtime[6];
        format(strtime, sizeof strtime, "%02d:00", time);
        PlayerTextDrawSetString(playerid, userInfo[playerid][TRTime], strtime);
    }
}

stock SetPlayerWeatherEx(playerid, weather = -1) {
    if(userInfo[playerid][SZ_Value][4] == 0) {
        weather = worldweather;
        if(weather == -1) {
            weather = 0;
        }
    } else {
        weather = userInfo[playerid][SZ_Value][6];
    }
    SetPlayerWeather(playerid, weather);
}

forward OnTimerTHM();
public OnTimerTHM() {
	for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
		if(IsPlayerConnected(i) == 1 && GetPlayerStatus(i) == 1) {
			accountRuntime_Speedometer(i);
		}
	}
	return 1;
}

forward OnTimerOneSec();
public OnTimerOneSec() {
    new Hour, Minute, Second, Timestamp;
    Timestamp = gettime(Hour, Minute, Second);
    if(Hour == 4 && Minute == 0 && Second == 0) {
        RecyclingResources(Timestamp);
    }
    redPacketRuntime(Timestamp);
    adminRuntime_RestartServ();
    configRuntime_QAs();
    UCPHeartBeat();
    if(wtimecount >= 59) {
        wtimecount = 0;
        wroldtime ++;
        if(wroldtime >= 24) {
            wroldtime = 0;
            worldweather = random(20);
        }
    }
    wtimecount ++;
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1) {
            userInfo[i][ESC] ++;
            if(userInfo[i][LTO] != -1 && GetPlayerStatus(i) == 0) {
                userInfo[i][LTO] --;
                if(userInfo[i][LTO] <= 0) {
                    userInfo[i][LTO] = -1;
                    KickEx(i, "登录超时");
                }
            }
            if(GetPlayerStatus(i) == 1) {
                if(userInfo[i][SZ_Value][1] == 1 && GetPlayerDMStatus(i) == 0) {
                    SetPlayerHealth(i, 95271);
                }
                if(userInfo[i][SZ_Value][2] == 1 && GetPlayerVehicleSeat(i) == 0) {
                    RepairVehicle(GetPlayerVehicleID(i));
                }
                if(userInfo[i][SZ_Value][3] == 0) {
                    if(wtimecount >= 60) {
                        SetPlayerTimeEx(i);
                    } else if(GetPlayerTRTimeStatus(i) == 0) {
                        new strtime[6];
                        format(strtime, sizeof strtime, "%02d:%02d", wroldtime, wtimecount);
                        PlayerTextDrawSetString(i, userInfo[i][TRTime], strtime);
                    }
                }
                if(userInfo[i][SZ_Value][4] == 0 && worldweather != -1) {
                    SetPlayerWeather(i, worldweather);
                }
                userInfo[i][OTPP] ++;
                if(userInfo[i][OTPP] >= 60) {
                    userInfo[i][OTPP] = 0;
                    userInfo[i][OnlineTime] ++;
                    if(userInfo[i][OnlineTime] >= GetPlayerLevelNeed(userInfo[i][Level])) {
                        userInfo[i][Level] ++;
                        accountSetLevel(i, userInfo[i][Level]);
                        new money = GetPlayerLevelMoney(userInfo[i][Level]), msg[128];
		                format(msg, sizeof msg, "[系统] 恭喜你时间分到达 %d 分钟, 当前等级 %d, 升到下一级需要 %d 分钟",
                        userInfo[i][OnlineTime], userInfo[i][Level], GetPlayerLevelNeed(userInfo[i][Level]));
                        SCM(i, COLOR_YELLOW, msg);
		                format(msg, sizeof msg, "[系统] 等级 %d 升级奖励 $%d", userInfo[i][Level], money);
                        SCM(i, COLOR_YELLOW, msg);
                        GiveSavings(i, money);
                        format(msg, sizeof msg, "(升级奖励) $%d Lv.%d", money, userInfo[i][Level]);
                        log_money(GetName(i), GetIp(i), GetPlayerUID(i), "self", "self", 0, money, msg);
                    }
                }
                if(GetPlayerMoney(i) != GetMoney(i)) {
                    ResetPlayerMoney(i);
                    GivePlayerMoney(i, GetMoney(i));
                }
                if(GetPlayerScore(i) != userInfo[i][Level]) {
                    SetPlayerScore(i, userInfo[i][Level]);
                }
                if(IsPlayerInRangeOfPoint(i, 10.0, 2000.66199, 1533.78284, 14.49102)) {
				//SCM(playerid, COLOR_YELLOW, "你已经进入hdc挂机点每秒可以获得3块钱游戏币喔);
                    GiveMoney(i, 3);
		            GiveOnlineTime(i, 1);
                }
                if(IsPlayerInRangeOfPoint(i, 18.0, 3768.91, -583.483, 0.699)) {
                    GiveMoney(i, 5);
					GiveOnlineTime(i, 1);
                }
                if(userInfo[i][GM_Status] == 1) {
                    admin_name_color_change[i] = (admin_name_color_change[i] == 0 ? (1) : (0));
                    SetPlayerColor(i, (admin_name_color_change[i] == 0 ? (COLOR_RED) :  (COLOR_RED)));
                }
                accountCountDown(i);
                accountRuntime_Jail(i);
                accountRuntime_Mute(i);
                accountRuntime_TP(i);
                ACOBJRuntime(i);
            }
        }
    }
    worldweather = -1;
    announcement_sec[1] ++;
    if(announcement_sec[1] >= announcement_sec[0]) {
        announcement_sec[1] = 0;
        SCMToAll(COLOR_RED, announcement[announcement_count[1]]);
        announcement_count[1] ++;
        if(announcement_count[1] >= announcement_count[0]) {
            announcement_count[1] = 0;
        }
    }
    return 1;
}
