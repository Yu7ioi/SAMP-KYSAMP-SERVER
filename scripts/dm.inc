enum dmtypes {
    Name[MAX_PLAYER_NAME],
    Float:MaxX,
    Float:MinX,	
    Float:MaxY,
    Float:MinY,
    Weapons[10],
	Float:X,
    Float:Y,
    Float:Z,
    Float:A
};

new DMInfos[][dmtypes] = {
    {"沙鹰战", 952.5, 806.5, -1072.0, -1130.0, {24,0,0,0,0,0,0,0,0,0}, 815.6,-1096.9,25.8},
	{"狙击战", 2733.5, 2567.5, 2826.0, 2678.0, {34,0,0,0,0,0,0,0,0,0}, 2617.8,2722.5,36.6}
	/* DM名称  右边     左边     上边    下边             武器1-10         出生x   出生y   出生z*/
};

new pDMStatus[MAX_PLAYERS], pDMKDA[MAX_PLAYERS][3];

stock GetPlayerDMStatus(playerid) {
    if(pDMStatus[playerid] == -1) return 0;
    return 1;
}

stock DMDKA(playerid, killerid) {
	if(killerid != INVALID_PLAYER_ID) {
		if(pDMStatus[playerid] != -1 && pDMStatus[killerid] != -1) {
			pDMKDA[playerid][1] ++;
			pDMKDA[killerid][0] ++;
			new msg[144];
			format(msg, sizeof msg, "[DM] %s(%d) 在%s里 (%d杀%d亡)", GetName(playerid), playerid, pDMKDA[playerid][0], pDMKDA[playerid][1]);
			SCMToAll(COLOR_BLUE, msg);
			format(msg, sizeof msg, "[DM] %s(%d) 在%s里 (%d杀%d亡)", GetName(killerid), killerid, pDMKDA[killerid][0], pDMKDA[killerid][1]);
			SCMToAll(COLOR_BLUE, msg);
		}
	}
}

stock SetPlayerIntoDM(playerid, dmid) {
    pDMStatus[playerid] = dmid;
    pDMKDA[playerid][0] = 0, pDMKDA[playerid][1] = 0, pDMKDA[playerid][2] = 0;
    /*     杀戮　　　　　　　　　　　　　死亡　　　　　　　　　　　　助攻，这玩意懒得写了*/
    if(dmid != -1) {
        SetPlayerWorldBounds(playerid, DMInfos[dmid][MaxX], DMInfos[dmid][MinX], DMInfos[dmid][MaxY], DMInfos[dmid][MinY]);
        new msg[144];
        format(msg, sizeof msg, "[系统]　%s(%d) 进入了 /dm %d (%s)", GetName(playerid), playerid, dmid+1, DMInfos[dmid][Name]);
        SCMToAll(COLOR_BLUE, msg);
        DMSetPAW(playerid);
    }
}

stock DMSetPAW(playerid) {
    if(pDMStatus[playerid] != -1) {
        for(new i = 0; i < 10; i ++) {
            if(DMInfos[pDMStatus[playerid]][Weapons][i] != 0) {
                GivePlayerWeapon(playerid, DMInfos[pDMStatus[playerid]][Weapons][i], 9999);
            }
        }
        SetPlayerPos(playerid, DMInfos[pDMStatus[playerid]][X], DMInfos[pDMStatus[playerid]][Y], DMInfos[pDMStatus[playerid]][Z]);
        SetPlayerFacingAngle(playerid, DMInfos[pDMStatus[playerid]][A]);
    }
}

CMD:dm(playerid, params[]) {
    if(!strlen(params)) {
        new msg[144];
        for(new i = 0; i < sizeof DMInfos; i ++) {
            format(msg, sizeof msg, "[系统] /dm %d (%s)", i + 1, DMInfos[i][Name]);
            SCM(playerid, COLOR_WHITE, msg);
        }
        SCM(playerid, COLOR_WHITE, "[系统]　离开DM /dm exit 或者　/dm out");
        return 1;
    }
    if(strcmp(params, "exit", true) == 0 || strcmp(params, "out", true) == 0) {
        if(pDMStatus[playerid] == -1) {
            SCM(playerid, COLOR_RED, "[系统] 你没在DM里面");
            return 1;
        }
		new msg[128];
		format(msg, sizeof msg, "[系统] 你离开了 DM:%s", DMInfos[pDMStatus[playerid]][Name]);
		SCM(playerid, COLOR_WHITE, msg);
        SetPlayerWorldBounds(playerid, 20000.0000, -20000.0000, 20000.0000, -20000.0000);
        pDMStatus[playerid] = -1;
        configPSPRandom(playerid);
        return 1;
    }
    if(pDMStatus[playerid] != -1) return SCM(playerid, COLOR_RED, "[系统] 如果你想离开DM可输入 /dm exit 或者 /dm out");
    new id = strval(params)-1;
    if(id < 0 || id > sizeof DMInfos - 1) return SCM(playerid, COLOR_RED, "[系统] 错误的DMID");
    SetPlayerIntoDM(playerid, id);
    return 1;
}