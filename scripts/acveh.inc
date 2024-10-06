#define BUY_AC_POS_X                (-1956.5216) // 购买爱车坐标x
#define BUY_AC_POS_Y                (304.8918) // 购买爱车坐标y哎
#define BUY_AC_POS_Z                (35.4759) // 购买爱车坐标z
#define BUY_AC_POS_R                (10.0) // 购买爱车坐标范围
#define AC_CREATED_AT_POS_X         (9999.0)//(-1985.0486) //购买是把爱车创建在坐标X
#define AC_CREATED_AT_POS_Y         (9999.0)//(275.5056) //购买是把爱车创建在坐标Y
#define AC_CREATED_AT_POS_Z         (9999.0)//(35.179) //购买是把爱车创建在坐标Z
#define AC_CREATED_AT_POS_A         (0.0)//(271.51) //购买是把爱车创建在坐标A
#define MAX_PLAYER_AC_LIMIT         (6) // 玩家最多拥有爱车数量
#define MAX_VEH_COMPONENTS          (14)
#define MAX_TICKET_LEN              (256)
#define DIALOG_AC                   (900)
#define Text_Strlen                 (256) //默认24
#define ChangeCarTextScore          (2500)
#define DIALOG_ACOBJ                DIALOG_AC + 50
#define DIALOG_AC_BUY               DIALOG_AC + 1
#define DIALOG_AC_SELL              DIALOG_AC + 2
#define DIALOG_AC_SELL_SYS          DIALOG_AC + 3
#define DIALOG_AC_SELL_S1           DIALOG_AC + 4 // 出售爱车给玩家 - 输入对方ID
#define DIALOG_AC_SELL_S2           DIALOG_AC + 5 // 出售爱车给玩家 - 输入出售价格
#define DIALOG_AC_SELL_S3           DIALOG_AC + 6 // 出售爱车给玩家 - 确定出售消息
#define DIALOG_AC_SELL_S4           DIALOG_AC + 7 // 出售爱车给玩家 - 对方是否同意购买
#define DIALOG_AC_BUY_CONFIRM		DIALOG_AC + 8
#define DIALOG_AC_Text + 9

enum acvehtype {
    Id,//数据库Id
    ModelId,//载具模型Id,
    Float:X,//坐标x
    Float:Y,//坐标y
    Float:Z,//坐标z
    Float:A,//角度
    Color1,//颜色1
    Color2,//颜色2
    Lock,//是否锁住
    Owner[MAX_PLAYER_NAME],//载具主人
    Components[MAX_VEH_COMPONENTS],//组件
    Ticket[MAX_TICKET_LEN],
    CreatedAt,
	TextView,//3dtext switch
    Text3D:Label,
	Text[Text_Strlen]
};

new AC_NOW_MAX;
new ACVeh[MAX_VEHICLES][acvehtype], pACWode[MAX_PLAYERS][MAX_PLAYER_AC_LIMIT], pACBuyId[MAX_PLAYERS];
new pACSell[MAX_PLAYERS][3];//0 = pid, 1 = money, 2 = vid

enum acvehbuytype {
    ModelId,
    Money,
    OTime
};

new ACBuy[210][acvehbuytype];
//new ACBuy[][acvehbuytype] = {
    //*{载具模型ID, 购买价格, 需要时间分},*/
	//{522, 500000, 2000},
	//{541, 741000, 3410},
	//{429, 597000, 2610},
    //{411, 600000, 2700},
	//{415, 550550, 2500},
	//{562, 780000, 3000},
	//{426, 490020, 2000},
	//{580, 910000, 4201},
	//{573, 1100000, 5000},
	//{495, 794000, 3223},
	//{560, 880000, 4000},
	//{562, 100000, 4000},
	//{411, 880000, 3000},
	//{432, 990000, 8000}
//};

stock ACVeh_Recycling(uid, name[]) {
    for(new v = 0; v < AC_NOW_MAX; v ++) {
        if(ACVeh[v][Id] != 0) {
            if(strcmp(ACVeh[v][Owner], name) == 0) {
                ACRespawn(v, 1);
                new msg[128], Cache:result, price;
                format(msg, sizeof msg, "UPDATE acveh SET deleted_at='%d' WHERE id='%d'", gettime(), ACVeh[v][Id]);
                mysql_query(mariadb, msg, false);
                format(msg, sizeof msg, "SELECT price FROM acveh WHERE id='%d'", ACVeh[v][Id]);
                result = mysql_query(mariadb, msg);
                cache_get_value_name_int(0, "price", price);
                cache_delete(result);
                GiveMoneyOffline(name, price/2);
				format(msg, sizeof msg, "回收爱车ID:%d", ACVeh[v][Id]);
				log_money(name, "none", uid, "acveh", "回收爱车", 0, price/2, msg);
            }
        }
    }
}

stock GetPlayerMaxAC(playerid) {
    new veh = 0;
    for(new i = 0; i < MAX_PLAYER_AC_LIMIT; i ++) {
        if(pACWode[playerid][i] != 0) {
            veh ++;
        }
    }
    return veh;
}

stock GetPlayerACNullId(playerid) {
    for(new i = 0; i < MAX_PLAYER_AC_LIMIT; i ++) {
        if(pACWode[playerid][i] == 0) {
            return i;
        }
    }
    return -1;
}

stock ACConnect(playerid) {
    for(new i = 0; i < MAX_PLAYER_AC_LIMIT; i ++) {
        pACWode[playerid][i] = 0;
    }
    new myname[MAX_PLAYER_NAME];
    format(myname, sizeof myname, GetName(playerid));
    for(new i = 0, v = GetVehiclePoolSize(); i <= v; i ++) {
        if(ACVeh[i][Id] != 0) {
            if(strcmp(ACVeh[i][Owner], myname) == 0) {
                pACWode[playerid][GetPlayerACNullId(playerid)] = i;
            }
        }
    }
    ACSellInit(playerid);
    ACOBJViewInit(playerid);
}

stock ACSellMain(playerid) {
    new vid = GetPlayerVehicleID(playerid);
    if(vid == 0 || IsVIDEqualMyAC(playerid, vid) == -1) {
        DIALOG_node(playerid, "你不在爱车内");
    } else {
        pACSell[playerid][2] = vid;
        ShowPlayerDialog(playerid, DIALOG_AC_SELL, DIALOG_STYLE_LIST, "爱车出售", "出售给系统\n出售给他人", "下一步", "关闭");
    }
}

stock ACSELLS1(playerid, err[] = " ") {
    new msg[128];
    format(msg, sizeof msg, "请输入对方ID\n{FF0000}%s", err);
    ShowPlayerDialog(playerid, DIALOG_AC_SELL_S1, DIALOG_STYLE_INPUT, "爱车出售", msg, "下一步", "返回");
}

stock ACSELLS2(playerid, err[] = " ") {
    new msg[128];
    format(msg, sizeof msg, "请输入出售价格\n{FF0000}%s", err);
    ShowPlayerDialog(playerid, DIALOG_AC_SELL_S2, DIALOG_STYLE_INPUT, "爱车出售", msg, "下一步", "返回");
}

stock ACSellInit(playerid, pid = -1) {
    pACSell[playerid][0] = -1;
    pACSell[playerid][1] = -1;
    pACSell[playerid][2] = -1;
    if(pid != -1) {
        pACSell[pid][0] = -1;
        pACSell[pid][1] = -1;
        pACSell[pid][2] = -1;
    }
}

forward ACODR(playerid, dialogid, response, listitem, inputtext[]);
public ACODR(playerid, dialogid, response, listitem, inputtext[]) {
    if(dialogid >= DIALOG_ACOBJ) {
        ACOBJODR(playerid, dialogid, response, listitem, inputtext);
        return 1;
    }
    if(dialogid == DIALOG_AC_SELL) {
        if(!response) {
            return 1;
        }
        if(listitem == 0) {
            ShowPlayerDialog(playerid, DIALOG_AC_SELL_SYS, DIALOG_STYLE_MSGBOX, "爱车出售", "你确定要将这辆爱车半价出售给系统吗?", "确定", "返回");
        } else if(listitem == 1) {
            ACSELLS1(playerid);
        }
        return 1;
    }
    if(dialogid == DIALOG_AC_SELL_S1) {
        if(!response) {
            ACSellMain(playerid);
            return 1;
        }
        new cnm[128];
        format(cnm, sizeof cnm, inputtext);
        if(strcmp(cnm, defaultIT) == 0) {
            ACSELLS1(playerid, "对方ID不能为空");
            return 1;
        }
        new pid = strval(cnm);
        if(pid < 0 || pid > MAX_PLAYERS || IsPlayerConnected(pid) == 0) {
            ACSELLS1(playerid, "错误的对方ID");
            return 1;
        }
		if(pid == playerid) {
			ACSELLS1(playerid, "你不可以卖给你自己");
            return 1;
		}
        pACSell[playerid][0] = pid;
        ACSELLS2(playerid);
        return 1;
    }
    if(dialogid == DIALOG_AC_SELL_S2) {
        if(!response) {
            ACSELLS1(playerid);
            return 1;
        }
        new cnm[128];
        format(cnm, sizeof cnm, inputtext);
        if(strcmp(cnm, defaultIT) == 0) {
            ACSELLS2(playerid, "对方ID不能为空");
            return 1;
        }
        new money = strval(cnm);
        if(money < 1 || money > 99999999) {
            ACSELLS2(playerid, "价格不能小于1或者大于1亿");
            return 1;
        }
        pACSell[playerid][1] = money;
        new msg[128];
        format(msg, sizeof msg, "你是否确定将该爱车已价格 $%d 出售给 %s", money, GetName(pACSell[playerid][0]));
        ShowPlayerDialog(playerid, DIALOG_AC_SELL_S3, DIALOG_STYLE_MSGBOX, "爱车出售", msg, "确定出售", "关闭");
        return 1;
    }
    if(dialogid == DIALOG_AC_SELL_S3) {
        if(!response) {
            DIALOG_node(playerid, "你取消了爱车出售");
            return 1;
        }
        if(IsPlayerConnected(pACSell[playerid][0]) == 0 || GetPlayerStatus(pACSell[playerid][0]) == 0) {
            DIALOG_node(playerid, "对方已离线");
            return 1;
        }
        pACSell[pACSell[playerid][0]][0] = playerid;
        pACSell[pACSell[playerid][0]][1] = 0;
        DIALOG_node(playerid, "[爱车系统]出售消息已发送给对方, 请等待对方的回复..");
        new msg[128];
        format(msg, sizeof msg, "%s 想将这辆爱车已价格 $%d 出售给你, 是否购买?", GetName(playerid), pACSell[playerid][1]);
        ShowPlayerDialog(pACSell[playerid][0], DIALOG_AC_SELL_S4, DIALOG_STYLE_MSGBOX, "爱车购买", msg, "确定购买", "关闭");
        return 1;
    }
    if(dialogid == DIALOG_AC_SELL_S4) {
        new msg[128], pid = pACSell[playerid][0];
        if(!response) {
            format(msg, sizeof msg, "%s 拒绝购买这辆爱车", GetName(playerid));
            DIALOG_node(pid, msg);
            ACSellInit(playerid, pid);
            format(msg, sizeof msg, " 已拒绝........");
            SCM(playerid, COLOR_ORANGE, msg);
            return 1;
        }
        new money = pACSell[pid][1];
        if(money < 1 && pACSell[playerid][1] != 0) {
            ACSellInit(playerid, pid);
            DIALOG_node(playerid, "对方已取消本次交易!");
            return 1;
        }
        if(IsPlayerConnected(pid) == 0 || GetPlayerStatus(pid) == 0) {
            ACSellInit(playerid, pid);
            DIALOG_node(playerid, "对方已离线, 交易取消!");
            return 1;
        }
        if(GetMoney(playerid) < money) {
            ACSellInit(playerid, pid);
            DIALOG_node(playerid, "你的钱不够, 交易取消!");
            return 1;
        }
		if(GetPlayerMaxAC(playerid) >= MAX_PLAYER_AC_LIMIT) {
			ACSellInit(playerid, pid);
            DIALOG_node(playerid, "你的爱车已到达上限, 交易取消!");
            return 1;
		}
        new vid = pACSell[pid][2];
        if(IsVIDEqualMyAC(pid, vid) == -1) {
            ACSellInit(playerid, pid);
            DIALOG_node(playerid, "该爱车不属于出售者, 交易取消!");
            return 1;
        }
        new idx = -1, acmenu_count;
        acmenu_count = acMenuCount();
        for(new i = 0; i < acmenu_count; i ++) {
            //if(ACBuy[i][ModelId] == ACVeh[vid][ModelId]) {
            if(loadAcMenu(i, 0) == ACVeh[vid][ModelId]) {
                idx = i;
                break;
            }
        }
        new needtime = (idx == -1 ? (5000) : (loadAcMenu(idx, 2)));
        if(userInfo[playerid][OnlineTime] < needtime) {
            DIALOG_node(playerid, "你的在线时间不够");
            return 1;
        }
        new Float:x, Float:y, Float:z;
        GetPlayerPos(pid, x, y, z);
        if(GetPlayerDistanceFromPoint(playerid, x, y, z) >= 5.0) {
            ACSellInit(playerid, pid);
            DIALOG_node(playerid, "你离对方太远, 交易取消!");
            return 1;
        }
        format(msg, sizeof msg, "[爱车系统] 爱车出售正在交易中, 请稍等...");
        SCM(pid, COLOR_ORANGE, msg);
        format(msg, sizeof msg, "[爱车系统] 正在交易爱车中, 请稍等...");
        SCM(playerid, COLOR_ORANGE, msg);
        format(ACVeh[vid][Owner], MAX_PLAYER_NAME, GetName(playerid));
        ACLabel(vid);
        format(msg, sizeof msg, "UPDATE acveh SET owner='%s' WHERE id='%d';", ACVeh[vid][Owner], ACVeh[vid][Id]);
        mysql_query(mariadb, msg, false);
        GiveMoney(playerid, -money);
        GiveMoney(pid, money);
        accountSaveData(pid);
        accountSaveData(playerid);
        format(msg, sizeof msg, "购买爱车, 模型ID:%d", ACVeh[vid][ModelId]);
        log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), GetName(pid), GetIp(pid), GetPlayerUID(pid), -money, msg);
        format(msg, sizeof msg, "出售爱车, 模型ID:%d", ACVeh[vid][ModelId]);
        log_money(GetName(pid), GetIp(pid), GetPlayerUID(pid), GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), money, msg);
		ACConnect(playerid);
		ACConnect(pid);
        format(msg, sizeof msg, "[爱车系统] 爱车出售成功 +$%d", money);
        SCM(pid, COLOR_ORANGE, msg);
        format(msg, sizeof msg, "[爱车系统] 爱车购买成功 -$%d", money);
        SCM(playerid, COLOR_ORANGE, msg);
        return 1;
    }
    if(dialogid == DIALOG_AC_SELL_SYS) {
        if(!response) {
            ACSellMain(playerid);
            return 1;
        }
        new vid = GetPlayerVehicleID(playerid);
        if(IsVIDEqualMyAC(playerid, vid) == -1) {
            DIALOG_node(playerid, "你不在爱车内");
            return 1;
        }
        new money = -1, acmenu_count;
        acmenu_count = acMenuCount();
        for(new i = 0; i < acmenu_count; i ++) {
            if(loadAcMenu(i, 0) == ACVeh[vid][ModelId]) {
                money = loadAcMenu(i, 1) / 2;
                break;
            }
        }
        if(money == -1) return 1;
        GiveMoney(playerid, money);
        accountSaveData(playerid);
        new msg[128];
        format(msg, sizeof msg, "出售爱车给(系统), 模型ID:%d", ACVeh[vid][ModelId]);
        log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "acveh", "acveh", 0, money, msg);
        ACDeleteDB(vid);
        ACRespawn(vid, 1);
        format(msg, sizeof msg, "你已将该爱车出售给了系统, +$%d", money);
        DIALOG_node(playerid, msg);
        ACConnect(playerid);
        return 1;
    }
    if(dialogid == DIALOG_AC_BUY) {
        if(!response) {
            return 1;
        }
        if(AC_NOW_MAX >= MAX_VEHICLES) {
            DIALOG_node(playerid, "当前不能购买爱车, 原因:爱车数量已达至上限");
            return 1;
        }
        if(GetPlayerMaxAC(playerid) >= MAX_PLAYER_AC_LIMIT) {
            DIALOG_node(playerid, "你的个人爱车已达至上限, 不能拥有更多爱车了.");
            return 1;
        }
        pACBuyId[playerid] = listitem;
		new msg[128];
		format(msg, sizeof msg, "你确定要购买该爱车吗? 模型ID:%d, 价格:$%d", loadAcMenu(listitem,0), loadAcMenu(listitem,1));
		ShowPlayerDialog(playerid, DIALOG_AC_BUY_CONFIRM, DIALOG_STYLE_MSGBOX, "购买爱车", msg, "确定", "关闭");
        return 1;
    }
	if(dialogid == DIALOG_AC_BUY_CONFIRM) {
		if(!response) return 1;
		new id = pACBuyId[playerid];
		if(userInfo[playerid][OnlineTime] < loadAcMenu(id,2)) {
            DIALOG_node(playerid, "你的在线时间不够");
            return 1;
        }
        if(GetMoney(playerid) < loadAcMenu(id,1)) {
            DIALOG_node(playerid, "你的钱不够");
            return 1;
        }
        if(GetPlayerMaxAC(playerid) >= MAX_PLAYER_AC_LIMIT) {
            DIALOG_node(playerid, "你的个人爱车已达至上限, 不能拥有更多爱车了.");
            return 1;
        }
        new msg[144], cost_money;
        cost_money = loadAcMenu(id, 1);
        GiveMoney(playerid, -cost_money);
        accountSaveData(playerid);
        format(msg, sizeof msg, "购买爱车, 模型ID:%d", loadAcMenu(id,0));
        log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "acveh", "acveh", 0, -cost_money, msg);
        new vid = ACInsertInto(loadAcMenu(id, 0), AC_CREATED_AT_POS_X, AC_CREATED_AT_POS_Y, AC_CREATED_AT_POS_Z,
        AC_CREATED_AT_POS_A, 0, 0, 0, GetName(playerid), cost_money);
        pACWode[playerid][GetPlayerACNullId(playerid)] = vid;
        format(msg, sizeof msg, "[爱车系统] 购买成功 -${00FF00}%d{FEA979},输入{FF00FF}/ac{FEA979}查看爱车帮助", cost_money);
        SCM(playerid, COLOR_ORANGE, msg);
        SCM(playerid, COLOR_ORANGE, "[爱车系统] 请使用{FF00FF}/ac wode{FEA979}提取您的爱车");
		return 1;
	}
    return 1;
}

CMD:ac(playerid, params[]) {
    if(!strlen(params)) {
        SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac wode [id] (召唤爱车(id 不填的话会默认召唤第一辆爱车))");
        SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac lock [id] (锁住/解锁爱车(id 不填的话会默认锁住/解锁当前座驾))");
        SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac park (爱车泊车)");
        SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac goto [id] (传送到爱车)");
        SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac buy (购买爱车)");
        SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac sell (出售爱车)");
        SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac buyobj (购买爱车装扮)");
        SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac list (我的爱车装扮)");
		SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac dl (显示/隐藏文字)");
		SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac color [颜色1] [颜色2]");
        if(GetPlayerAccess(playerid, "jiancha") == 1) {
            SCM(playerid, COLOR_ORANGE, "[爱车系统] /ac ticket [载具ID] [罚款金额] [原因] (贴罚单)");
        }
        return 1;
    }
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(strcmp(tmp, "ticket", true) == 0 && GetPlayerAccess(playerid, "jiancha") == 1) {
        tmp = strtok(params, idx);
        if(!strlen(tmp)) {
            DIALOG_node(playerid, "载具ID不能为空");
            return 1;
        }
        new vid = strval(tmp);
        if(ACVeh[vid][Id] == 0) {
            DIALOG_node(playerid, "错误的载具ID");
            return 1;
        }
        tmp = strtok(params, idx);
        if(!strlen(tmp)) {
            DIALOG_node(playerid, "罚款金额不能为空");
            return 1;
        }
        new money = strval(tmp);
        if(money < 1000) {
            DIALOG_node(playerid, "罚款金额不能小于1000");
            return 1;
        }
        tmp = strrest(params, idx);
        if(!strlen(tmp)) {
            DIALOG_node(playerid, "罚款原因不能为空");
            return 1;
        }
        if(IsVehHasTicket(vid) == 1) {
            DIALOG_node(playerid, "该载具已有罚单");
            return 1;
        }
        new msg[256];
        format(msg, sizeof msg, "[系统] 交通监察员 %s 给 %s 的载具: %d 开了一张罚单.", GetNameEx(playerid), ACVeh[vid][Owner], vid);
        SCMToAll(COLOR_BLUE, msg);
        format(msg, sizeof msg, "[系统] 罚款金额: $%d, 原因: %s", money, tmp);
        SCMToAll(COLOR_BLUE, msg);
        ACUpdateTicket(vid, money, GetNameEx(playerid), tmp);
        return 1;
    } else if(strcmp(tmp, "color", true) == 0) {
		if(GetPlayerMaxAC(playerid) == 0) {
            DIALOG_node(playerid, "你没有爱车");
            return 1;
        }
        new vid = GetPlayerVehicleID(playerid);
        if(vid == 0) {
            DIALOG_node(playerid, "你不在爱车中");
            return 1;
        }
        new id = IsVIDEqualMyAC(playerid, vid);
        if(id == -1) {
            DIALOG_node(playerid, "这不是你的爱车");
            return 1;
        }
		new color1, color2;
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			color1 = random(255);
		} else {
			color1 = strval(tmp);
			if(color1 < 0 || color1 > 255) {
				DIALOG_node(playerid, "颜色不能小于0或者大于255");
				return 1;
			}
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			color2 = random(255);
		} else {
			color2 = strval(tmp);
			if(color2 < 0 || color2 > 255) {
				DIALOG_node(playerid, "颜色不能小于0或者大于255");
				return 1;
			}
		}
		new msg[144];
        ACVeh[vid][Color1] = color1, ACVeh[vid][Color2] = color2;
        format(msg, sizeof msg, "UPDATE acveh SET color1='%d', color2='%d' WHERE id='%d'", color1, color2, ACVeh[vid][Id]);
        mysql_query(mariadb, msg, false);
		ChangeVehicleColor(vid, color1, color2);
		return 1;
	}
	if(strcmp(tmp, "dl", true) == 0) {
		if(GetPlayerMaxAC(playerid) == 0) {
            DIALOG_node(playerid, "你没有爱车");
            return 1;
        }
        new vid = GetPlayerVehicleID(playerid);
        if(vid == 0) {
            DIALOG_node(playerid, "你不在爱车中");
            return 1;
        }
        new id = IsVIDEqualMyAC(playerid, vid);
        if(id == -1) {
            DIALOG_node(playerid, "这不是你的爱车");
            return 1;
        }
		ACTextView(playerid, vid);
		return 1;
	}
	if(strcmp(tmp, "list", true) == 0) {
        ACOBJList(playerid);
        return 1;
    }
    if(strcmp(tmp, "sell", true) == 0) {
        ACSellMain(playerid);
        return 1;
    } else if(strcmp(tmp, "park", true) == 0) {
        if(GetPlayerMaxAC(playerid) == 0) {
            DIALOG_node(playerid, "你没有爱车");
            return 1;
        }
        new vid = GetPlayerVehicleID(playerid);
        if(vid == 0) {
            DIALOG_node(playerid, "你不在爱车中");
            return 1;
        }
        new id = IsVIDEqualMyAC(playerid, vid);
        if(id == -1) {
            DIALOG_node(playerid, "这不是你的爱车");
            return 1;
        }
        new Float:x, Float:y, Float:z, Float:a = GetPlayerAngle(playerid);
        GetPlayerPos(playerid, x, y, z);
        ACParked(vid, x, y, z, a);
        DIALOG_node(playerid, "泊车成功");
        return 1;
    } else if(strcmp(tmp, "goto", true) == 0) {
        if(GetPlayerMaxAC(playerid) == 0) {
            DIALOG_node(playerid, "你没有爱车");
            return 1;
        }
        tmp = strtok(params, idx);
        new id = (strlen(tmp) == 0 ? (1) : (strval(tmp)));
        if(id < 1 || id > MAX_PLAYER_AC_LIMIT) {
            DIALOG_node(playerid, "错误的爱车召唤ID");
            return 1;
        }
        id --;
        if(ACVeh[pACWode[playerid][id]][Id] == 0) {
            format(tmp, sizeof tmp, "你没有%d辆爱车", id+1);
            DIALOG_node(playerid, tmp);
            return 1;
        }
        if(ACVeh[pACWode[playerid][id]][X] == AC_CREATED_AT_POS_X/* && ACVeh[pACWode[playerid][id]][Y] == AC_CREATED_AT_POS_Y && ACVeh[pACWode[playerid][id]][Z] == AC_CREATED_AT_POS_Z*/)
        {
            format(tmp, sizeof tmp, "你的爱车还未初始化 请使用/ac wode召唤并/ac park停泊 方可正常使用goto");
            DIALOG_node(playerid, tmp);
            return 1;
        }
        new Float:x, Float:y, Float:z;
        GetVehiclePos(pACWode[playerid][id], x, y, z);
        SetPlayerVirtualWorld(playerid, GetVehicleVirtualWorld(pACWode[playerid][id]));
        SetPlayerPos(playerid, x+1, y+1, z+1);
        format(tmp, sizeof tmp, "[系统] 你传送到了你的第%d辆爱车.", id+1);
        SCM(playerid, COLOR_ORANGE, tmp);
        return 1;
    } else if(strcmp(tmp, "lock", true) == 0) {
        new vid = GetPlayerVehicleID(playerid);
        tmp = strtok(params, idx);
        if(!strlen(tmp)) {
            if(vid == 0) {
                DIALOG_node(playerid, "你不在爱车中");
            } else {
                if(IsVIDEqualMyAC(playerid, vid) != -1) {
                    ACLocked(vid);
                    format(tmp, sizeof tmp, "%s", (ACVeh[vid][Lock] == 0 ? ("[爱车系统]已解锁该爱车") : ("[爱车系统]已锁住该爱车")));
                    SCM(playerid, COLOR_ORANGE, tmp);
                } else {
                    DIALOG_node(playerid, "这不是你的爱车");
                }
            }
            return 1;
        }
        new id = strval(tmp);
        if(id < 1 || id > MAX_PLAYER_AC_LIMIT) {
            DIALOG_node(playerid, "错误的爱车召唤ID");
            return 1;
        }
        id --;
        ACLocked(pACWode[playerid][id]);
        format(tmp, sizeof tmp, "%s", (ACVeh[vid][Lock] == 0 ? ("已解锁该爱车") : ("已锁住该爱车")));
        SCM(playerid, COLOR_WHITE, tmp);
        return 1;
    } else if(strcmp(tmp, "wode", true) == 0) {
        if(GetPlayerMaxAC(playerid) == 0) {
            DIALOG_node(playerid, "你没有爱车");
            return 1;
        }
        tmp = strtok(params, idx);
        new id = (strlen(tmp) == 0 ? (1) : (strval(tmp)));
        if(id < 1 || id > MAX_PLAYER_AC_LIMIT) {
            DIALOG_node(playerid, "错误的爱车召唤ID");
            return 1;
        }
        id --;
        if(ACVeh[pACWode[playerid][id]][Id] == 0) {
            format(tmp, sizeof tmp, "你没有%d辆爱车", id+1);
            DIALOG_node(playerid, tmp);
            return 1;
        }
        new Float:x, Float:y, Float:z;
        GetPlayerPos(playerid, x, y, z);
        if(IsVehHasTicket(pACWode[playerid][id]) == 1) {
            new str[512];
            format(str, sizeof str, "原因:%s\n罚款:%d\n交通监查员:%s\n请关注微信公众号samp狂野之城\n进入UCP缴纳罚款", 
            GetVehicleTicketReason(pACWode[playerid][id]), 
            GetVehicleTicketPrice(pACWode[playerid][id]),
            GetVehicleTicketName(pACWode[playerid][id]));
            DIALOG_node(playerid, str, "载具罚单");
            SetPlayerPos(playerid, x+1, y+1, z +1);
            return 1;
        }
        SetVehiclePos(pACWode[playerid][id], x+1, y+1, z+1);
        // SetVehicleVirtualWorld(pACWode[playerid][id], GetPlayerVirtualWorld(playerid));
        PutPlayerInVehicle(playerid, pACWode[playerid][id], 0);
        AddVehicleComponent(pACWode[playerid][id], 1010);
        format(tmp, sizeof tmp, "[爱车系统] 你召唤了你的第%d辆爱车.", id+1);
        SCM(playerid, COLOR_ORANGE, tmp);
        return 1;
    } else if(strcmp(tmp, "buy", true) == 0) {
        if(GetPlayerDistanceFromPoint(playerid, BUY_AC_POS_X, BUY_AC_POS_Y, BUY_AC_POS_Z) >= BUY_AC_POS_R) {
            SCM(playerid, COLOR_ORANGE, "[爱车系统]你不在爱车购买点,请输入{FF00FF} /MCD{FEA979} 传送到买车点.");
            return 1;
        }
        new msg[2048], menu_count;
        menu_count = acMenuCount();
        format(msg, sizeof msg, "模型ID\t购买价格\t需在线时间\n");
        for(new i = 0; i < menu_count; i ++) {
            format(msg, sizeof msg, "%s%d\t$%d\t%d\n", msg, loadAcMenu(i, 0), loadAcMenu(i, 1), loadAcMenu(i, 2));
        }
        ShowPlayerDialog(playerid, DIALOG_AC_BUY, DIALOG_STYLE_TABLIST_HEADERS, "购买爱车", msg, "购买", "关闭");
        return 1;
    } else if(strcmp(tmp, "buyobj", true) == 0) {
        ACOBJBuyCMD(playerid);
        return 1;
    }
    return 1;
}

/*stock AC_OnPlayerLogin(playerid)
{
    //20240520 By Selend1s :D
    for(new i = 0; i < MAX_PLAYER_AC_LIMIT; i++)
    {
        pACWode[playerid][i] = 0;
    }
    new pName[MAX_PLAYER_NAME];
    format(pName, sizeof pName, GetName(playerid));
    for(new i = 0; i <= MAX_VEHICLES; i++)
    {
        if(ACVeh[i][Id] != 0)
        {
            if(strcmp(ACVeh[i][Owner], pName) == 0)
            {
                pACWode[playerid][GetPlayerACNullId(playerid)] = i;
            }
        }
    }
    ACSellInit(playerid);
    ACOBJViewInit(playerid);
    return 1;
}*/

stock IsVIDEqualMyAC(playerid, vid) {
    if(vid == 0) return -1;
    for(new i = 0; i < MAX_PLAYER_AC_LIMIT; i ++) {
        if(pACWode[playerid][i] != 0 && pACWode[playerid][i] == vid && ACVeh[vid][Id] != 0) {
            return i;
        }
    }
    return -1;
}

stock ACTextView(playerid, vid) {
	if(ACVeh[vid][Id] != 0) {
		ACVeh[vid][TextView] = (ACVeh[vid][TextView] == 0 ? (1) : (0));
		new msg[128];
		format(msg, sizeof msg, "UPDATE acveh SET textview='%d' WHERE id='%d'", ACVeh[vid][TextView], ACVeh[vid][Id]);
		mysql_query(mariadb, msg, false);
		format(msg, sizeof msg, "你%s了该爱车的文字显示", (ACVeh[vid][TextView] == 0 ? ("隐藏") : ("开启")));
		DIALOG_node(playerid, msg);
        ACLabel(vid);
	}
}

stock ACParked(vid, Float:x, Float:y, Float:z, Float:a) {
    if(ACVeh[vid][Id] != 0) {
        new msg[144];
        ACVeh[vid][X] = x, ACVeh[vid][Y] = y, ACVeh[vid][Z] = z, ACVeh[vid][A] = a;
        format(msg, sizeof msg, "UPDATE acveh SET x='%.3f',y='%.3f',z='%.3f',a='%.3f' WHERE id='%d'", x, y, z, a, ACVeh[vid][Id]);
        mysql_query(mariadb, msg, false);
    }
}

stock ACLocked(vid) {
    if(ACVeh[vid][Id] != 0) {
        ACVeh[vid][Lock] = (ACVeh[vid][Lock] == 0 ? (1) : (0));
        new msg[128];
        format(msg, sizeof msg, "UPDATE acveh SET `lock`='%d' WHERE id='%d'", ACVeh[vid][Lock], ACVeh[vid][Id]);
        mysql_query(mariadb, msg, false);
        if(ACVeh[vid][Lock] == 1) {
            for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
                if(GetPlayerVehicleID(i) == vid) {
					if(strcmp(GetName(i), ACVeh[vid][Owner]) != 0) {
						LockedMSG(i);
					}
				}
            }
        }
    }
}

stock ACInsertInto(modelid, Float:x, Float:y, Float:z, Float:a, color1, color2, lock, owner[], price) {
    new query[512], Cache:result, dbid, created_at = gettime();
    format(query, sizeof query, "INSERT INTO acveh (modelid,x,y,z,a,color1,color2,`lock`,owner,components,created_at,price) VALUES ('%d','%.3f','%.3f','%.3f','%.3f','%d','%d','%d','%s','0','%d','%d');", 
    modelid, x, y, z, a, color1, color2, lock, owner, created_at, price);
    result = mysql_query(mariadb, query);
    dbid = cache_insert_id();
    cache_delete(result);
    AC_NOW_MAX ++;
    return ACVehicle(dbid, modelid, x, y, z, a, color1, color2, lock, owner, created_at, 1, 0);
}

stock AC_changeNtoN(oldname[], newname[]) {
    new query[128];
    for(new v = 0, j = GetVehiclePoolSize(); v <= j; v ++) {
        if(ACVeh[v][Id] != 0) {
            if(strcmp(ACVeh[v][Owner], oldname) == 0) {
                format(ACVeh[v][Owner], MAX_PLAYER_NAME, newname);
                ACLabel(v);
            }
        }
    }
    format(query, sizeof query, "UPDATE acveh SET owner='%s' WHERE owner='%s';", newname, oldname);
    mysql_query(mariadb, query, false);
}

stock ACDeleteDB(vid) {
    if(ACVeh[vid][Id] == 0) return 0;
    new query[128];
    format(query, sizeof query, "UPDATE acveh SET deleted_at='%d' WHERE id='%d'", gettime(), ACVeh[vid][Id]);
    mysql_query(mariadb, query, false);
    return 1;
}

stock ACRespawn(vid, typeid = 0) {
    if(ACVeh[vid][Id] != 0) {
        if(IsValidDynamic3DTextLabel(ACVeh[vid][Label]) == 1) {
            DestroyDynamic3DTextLabel(ACVeh[vid][Label]);
        }
        DestroyVehicle(vid);
        if(typeid == 0) {
            new cc = ACVehicle(ACVeh[vid][Id], ACVeh[vid][ModelId], ACVeh[vid][X], ACVeh[vid][Y], ACVeh[vid][Z], ACVeh[vid][A], ACVeh[vid][Color1],
            ACVeh[vid][Color2], ACVeh[vid][Lock], ACVeh[vid][Owner], ACVeh[vid][CreatedAt], ACVeh[vid][TextView], vid); 
			if(cc != vid) {
				ACVeh[vid][Id] = 0;
			}
		} else {
			ACVeh[vid][Id] = 0;
		}
    }
}

stock ACLabel(vid) {
    if(ACVeh[vid][Id] != 0) {
        if(IsValidDynamic3DTextLabel(ACVeh[vid][Label]) == 1) {
			DestroyDynamic3DTextLabel(ACVeh[vid][Label]);
		}
		if(ACVeh[vid][TextView] == 1) {
			new msg[64];
			format(msg, sizeof msg, "%s的爱车", ACVeh[vid][Owner]);
			ACVeh[vid][Label] = CreateDynamic3DTextLabel(msg, COLOR_WHITE, 0.0, 0.0, 0.0, 10.0, INVALID_PLAYER_ID, vid);
		}
    }
}

stock ACVehicle(dbid, modelid, Float:x, Float:y, Float:z, Float:a, color1, color2, lock, owner[], created_at, tview, ctypeid = 1, components[] = "0", ticket[] = "0") {
    new vid = CreateVehicle(modelid, x, y, z, a, color1, color2, -1), idx;
    ACVeh[vid][Id] = dbid, ACVeh[vid][ModelId] = modelid, ACVeh[vid][X] = x, ACVeh[vid][Y] = y, ACVeh[vid][Z] = z, ACVeh[vid][CreatedAt] = created_at,
    ACVeh[vid][A] = a, ACVeh[vid][Color1] = color1, ACVeh[vid][Color2] = color2, ACVeh[vid][Lock] = lock, ACVeh[vid][TextView] = tview;
    format(ACVeh[vid][Owner], MAX_PLAYER_NAME, owner);
    for(new i = 0; i < MAX_VEH_COMPONENTS; i ++) {
        if(ctypeid == 0) {
            ACVeh[vid][Components][i] = strval(strtok(components, idx));
        } else {
            ACVeh[vid][Components][i] = ACVeh[ctypeid][Components][i];
        }
        if(ACVeh[vid][Components][i] != 0) {
            AddVehicleComponent(vid, ACVeh[vid][Components][i]);
        }
    }
    if(ctypeid == 0) {
        format(ACVeh[vid][Ticket], MAX_TICKET_LEN, ticket);
    } else {
        format(ACVeh[vid][Ticket], MAX_TICKET_LEN, ACVeh[ctypeid][Ticket]);
    }
    ACLabel(vid);
    ACOBJAttach(vid);
    return vid;
}

stock ACVEHOVM(playerid, vehicleid, componentid) {
    new vid = IsVIDEqualMyAC(playerid, vehicleid);
    if(vid != -1) {
        for(new i = 0; i < MAX_VEH_COMPONENTS; i ++) {
            if(ACVeh[vehicleid][Components][i] == 0) {
               ACVeh[vehicleid][Components][i] =  componentid;
               break;
            }
        }
    }
}

stock ACVEHOVR(playerid, vehicleid, color1, color2) {
    new vid = IsVIDEqualMyAC(playerid, vehicleid);
    if(vid != -1) {
        ACVeh[vehicleid][Color1] = color1, ACVeh[vehicleid][Color2] = color2;
    }
}

stock ACVEHOEEMS(playerid, enterexit) {
    if(enterexit == 0) {
        new vid = IsVIDEqualMyAC(playerid, GetPlayerVehicleID(playerid));
        if(vid != -1) {
            ACVehUpdateCC(GetPlayerVehicleID(playerid));
        }
    }
}

// update components and colors
stock ACVehUpdateCC(vid) {
    new query[256], components[128];
    for(new i = 0; i < MAX_VEH_COMPONENTS; i ++) {
        format(components, sizeof components, "%d %s", ACVeh[vid][Components][i], components);
    }
    format(query, sizeof query, "UPDATE acveh SET color1='%d', color2='%d', components='%s' WHERE id='%d'", 
    ACVeh[vid][Color1], ACVeh[vid][Color2], components, ACVeh[vid][Id]);
    mysql_query(mariadb, query, false);
}

// 从数据库加载爱车菜单列表 /ac buy
/**
* row: 
* type: 
*/
stock loadAcMenu(row, type){
    new Cache:result, query[128], model_id, price, online_time;
    format(query, sizeof query, "SELECT * FROM acveh_menu");
    result = mysql_query(mariadb, query);
    cache_get_value_name_int(row, "model_id", model_id);
    cache_get_value_name_int(row, "prize", price);
    cache_get_value_name_int(row, "online_time", online_time);
    cache_delete(result);
    if(type == 0){
        return model_id;
    }else if(type == 1){
        return price;
    }else if(type == 2){
        return online_time;
    }
    return 0;
}

stock initAcMenu(){
    new Cache:result, query[128], model_id, price, online_time, count;
    format(query, sizeof query, "SELECT * FROM acveh_menu");
    result = mysql_query(mariadb, query);
    cache_get_row_count(count);
    for(new row = 0; row < count; row ++) {
        cache_get_value_name_int(row, "model_id", model_id);
        cache_get_value_name_int(row, "prize", price);
        cache_get_value_name_int(row, "online_time", online_time);
        ACBuy[row][ModelId] = model_id;
        ACBuy[row][Money] = price;
        ACBuy[row][OTime] = online_time;
    }
    cache_delete(result);
}

/**
    获取爱车菜单列表数量
*/
stock acMenuCount(){
    new Cache:result, query[128], count;
    format(query, sizeof query, "SELECT * FROM acveh_menu");
    result = mysql_query(mariadb, query);
    cache_get_row_count(count);
    cache_delete(result);
    return count;
}

/**
    获取用户数量
*/
stock getUserCount(){
    new Cache:result, msg[128], userCount;
    format(msg, sizeof msg, "SELECT COUNT(uid) as userCount FROM account");
    result = mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "exist", userCount);
    cache_delete(result);
    return exist;
}

/**
    2024/1/28再次修复最后遗留的吞车问题
*/
stock CountOwnerOccurrences(const owner[], currentIdx) {
    new count = 0;
    for (new i = 0; i < currentIdx; i++) {
        new deleted_at;
        new tempOwner[MAX_PLAYER_NAME];
        cache_get_value_name_int(i, "deleted_at", deleted_at);
        if(deleted_at == 0) {
            cache_get_value_name(i, "owner", tempOwner, MAX_PLAYER_NAME);
            if (!strcmp(tempOwner, owner, true)) {
                count++;
            }
        }
    }
    return count;
}

/**
* 爱车系统初始化
*/
stock ACVehInit() {
    CreateDynamic3DTextLabel("在此输入/ac buy\n购买爱车", COLOR_WHITE, BUY_AC_POS_X, BUY_AC_POS_Y, BUY_AC_POS_Z, 25.0);
    ACOBJ3DTEXT();
    new Cache:result, query[128], counts;
    format(query, sizeof query, "SELECT * FROM acveh");
    result = mysql_query(mariadb, query);
    cache_get_row_count(AC_NOW_MAX);
    for(new i = 0; i < AC_NOW_MAX; i ++) {
        new id, modelid, Float:x, Float:y, Float:z, Float:a, color1, color2, lock, owner[MAX_PLAYER_NAME], created_at, components[128], tview, deleted_at, ticket[MAX_TICKET_LEN];
        cache_get_value_name_int(i, "deleted_at", deleted_at);
        if(deleted_at == 0) {
            cache_get_value_name_int(i, "id", id);
            cache_get_value_name_int(i, "modelid", modelid);
            cache_get_value_name_float(i, "x", x);
            cache_get_value_name_float(i, "y", y);
            cache_get_value_name_float(i, "z", z);
            cache_get_value_name_float(i, "a", a);
            cache_get_value_name_int(i, "color1", color1);
            cache_get_value_name_int(i, "color2", color2);
            cache_get_value_name_int(i, "lock", lock);
            cache_get_value_name(i, "owner", owner, MAX_PLAYER_NAME);
            cache_get_value_name_int(i, "created_at", created_at);
            cache_get_value_name(i, "components", components, sizeof components);
            cache_get_value_name_int(i, "textview", tview);
            cache_get_value_name(i, "ticket", ticket, sizeof ticket);
            // 检查owner是否超过6次
            if (CountOwnerOccurrences(owner, i) >= MAX_PLAYER_AC_LIMIT) {
                // printf("for i = %d, id = %d, modelid = %d, x = %f, y = %f, z = %f, a = %f, color1 = %d, color2 = %d, lock = %d, owner = %s, created_at = %d, components = %s, tview = %d, deleted_at = %d, ticket = %s", i, id, modelid, x, y, z, a, color1, color2, lock, owner, created_at, components, tview, deleted_at, ticket);
                continue; // 跳到循环的下一次迭代
            }
            //printf("for i = %d, id = %d, modelid = %d, x = %f, y = %f, z = %f, a = %f, color1 = %d, color2 = %d, lock = %d, owner = %s, created_at = %d, components = %s, tview = %d, deleted_at = %d, ticket = %s", i, id, modelid, x, y, z, a, color1, color2, lock, owner, created_at, components, tview, deleted_at, ticket);
            ACOBJInit(ACVehicle(id, modelid, x, y, z, a, color1, color2, lock, owner, created_at, tview, 0, components, ticket));
            counts ++;
        }
    }
    cache_delete(result);
    AC_NOW_MAX = counts;
    printf("acveh -> 已加载%d辆私人载具", AC_NOW_MAX);

    initAcMenu(); // 初始化爱车购买菜单
}

stock ACOPSC(playerid, newstate) {
    if(newstate == PLAYER_STATE_DRIVER || newstate == PLAYER_STATE_PASSENGER) {
        LockedMSG(playerid);
    }
}

stock LockedMSG(playerid) {
    new vid = GetPlayerVehicleID(playerid), Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    if(ACVeh[vid][Id] != 0) {
        if(strcmp(ACVeh[vid][Owner], GetName(playerid)) == 0) {
            new wodeid = IsVIDEqualMyAC(playerid, vid);
            if(wodeid != -1 && GetPlayerVehicleSeat(playerid) == 0) {
                if(IsVehHasTicket(vid) == 1) {
                    new str[512];
                    format(str, sizeof str, "原因:%s\n罚款:%d\n交通监查员:%s", 
                    GetVehicleTicketReason(vid), 
                    GetVehicleTicketPrice(vid),
                    GetVehicleTicketName(vid));
                    DIALOG_node(playerid, str, "载具罚单");
                    SetPlayerPos(playerid, x+1, y+1, z+1);
                } else {
                    new msg[128];
                    format(msg, sizeof msg, "[爱车系统] 欢迎回到您的%d号爱车, 车锁状态:%s", wodeid+1, (ACVeh[pACWode[playerid][wodeid]][Lock] == 0 ? ("未上锁") : ("已锁住")));
                    SCM(playerid, COLOR_ORANGE, msg);
                }
            }
        } else if(ACVeh[vid][Lock] == 1) {
            DIALOG_node(playerid, "该爱车已被其主人锁住");
            SetPlayerPos(playerid, x+1, y+1, z+1);
        } else if(IsVehHasTicket(vid) == 1) {
            DIALOG_node(playerid, "该爱车暂时无法驾驶");
            SetPlayerPos(playerid, x+1, y+1, z+1);
        }
    }
}

stock ACCancelTicket(vid) {
    format(ACVeh[vid][Ticket], MAX_TICKET_LEN, "0");
    new msg[128];
    format(msg, sizeof msg, "UPDATE acveh SET ticket='0' WHERE id='%d'", ACVeh[vid][Id]);
    mysql_query(mariadb, msg, false);
}

stock ACUpdateTicket(vid, money, name[], reason[]) {
    format(ACVeh[vid][Ticket], MAX_TICKET_LEN, "%d %s %s", money, name, reason);
    new msg[128];
    format(msg, sizeof msg, "UPDATE acveh SET ticket='%s' WHERE id='%d'", ACVeh[vid][Ticket], ACVeh[vid][Id]);
    mysql_query(mariadb, msg, false);
}

stock GetVehicleTicketPrice(vid) {
    new price[24];
    sscanf(ACVeh[vid][Ticket], "s[24] ", price);
    return strval(price);
}

stock GetVehicleTicketName(vid) {
    new name[MAX_PLAYER_NAME];
    sscanf(ACVeh[vid][Ticket], "s[24] s[24] ", name, name);
    return name;
}

stock GetVehicleTicketReason(vid) {
    new reason[128];
    sscanf(ACVeh[vid][Ticket], "s[24] s[24] s[128]", reason, reason, reason);
    return reason;
}

stock IsVehHasTicket(vid) {
    if(mk_strcmp(ACVeh[vid][Ticket], "0") == 0) return 0;
    return 1;
}

stock GetVehicleIdFromCreatedAt(created_at) {
    for(new v = 0, vehs = GetVehiclePoolSize(); v <= vehs; v ++) {
        if(ACVeh[v][Id] != 0 && ACVeh[v][CreatedAt] == created_at) {
            return v;
        }
    }
    return -1;
}