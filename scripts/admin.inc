new admin_tmp_veh[MAX_VEHICLES], restartserv, bbabprotection;

stock IsServRestarting() {
    if(restartserv == -1) return 0;
    return 1;
}

stock AdminCommand(string[])//权限使用记录
{
	new str[256],File:hFile,Year, Month, Day,Hour, Minute, Second;
	getdate(Year, Month, Day);
	gettime(Hour, Minute, Second);
	hFile = fopen("Access/AdminCommand.txt", io_append);
	if(hFile)
	{
	    format(str,sizeof(str),"[%d年%d月%d日-时间%d:%d:%d] %s\r\n",Year, Month, Day,Hour, Minute, Second, string);
		pfwrite(hFile,str);
		fclose(hFile);
		//printf(str);
	}
	return 1;
}

adminInit() {
    bbabprotection = 0;
    restartserv = -1;
    for(new i = 0; i < MAX_VEHICLES; i ++) {
        admin_tmp_veh[i] = 0;
    }
}

stock adminRuntime_RestartServ() {
    if(restartserv != -1) {
        if(restartserv == 0) {
            for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
                if(IsPlayerConnected(i) == 1) {
                    KickEx(i, "重启服务器");
                }
            }
            if(bbabprotection == 0) {
                UCPSendChat("samp:restartme_pls");
                SetTimerEx("RestartServDelay", 2000, false, "i", 0);
                SetServName("服务器正在自动重启中...");
            }
            bbabprotection = 1; 
        } else {
            new msg[64];
            format(msg, sizeof msg, "[系统] %d秒后重启服务器.", restartserv);
            SCMToAll(COLOR_RED, msg);
            format(msg, sizeof msg, "%d秒后刷新服务器.", restartserv);
            SetServName(msg);
            restartserv --;
        }
    }
}

forward RestartServDelay(id);
public RestartServDelay(id) {
    printf("exit", id);
    SendRconCommand("exit");
    return 1;
}

stock SCMToAdmin(color, str[]) {
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1 && GetPlayerStatus(i) == 1) {
            if(GetPlayerAccess(i, "gm_1") == 1) {
                SCM(i, color, str);
            }
        }
    }
}

CMD:testveh(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    printf("veh pool size: %d", GetVehiclePoolSize());
    return 1;
}

CMD:lockupserv(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /lockupserv [密码] (0 = 解锁)");
        return 1;
    }
    SetServPassword(params);
    new msg[144];
    format(msg, sizeof msg, "你把服务器锁住了, 密码为: %s", params);
    DIALOG_node(playerid, msg);
    return 1;
}

CMD:restartserv(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /restartserv [倒计时秒数] [原因]");
        return 1;
    }
    new seconds = strval(tmp);
    if(seconds < 3) {
        SCM(playerid, COLOR_RED, "[系统] 倒计时器秒数不能小于3");
        return 1;
    }
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空");
        return 1;
    }
    restartserv = seconds;
    format(tmp, sizeof tmp, "[系统] %d秒后重启服务器, 原因:%s", seconds, tmp);
    SCMToAll(COLOR_RED, tmp);
    return 1;
}

CMD:allgms(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new msg[2048], count, Cache:result, access[128], name[MAX_PLAYER_NAME];
    format(msg, sizeof msg, "SELECT * FROM account");
    result = mysql_query(mariadb, msg);
    cache_get_row_count(count);
    format(msg, sizeof msg, "");
    for(new i = 0; i < count; i ++) {
        cache_get_value_name(i, "access", access);
        if(GetAccess(access, "gm_1") == 1) {
            cache_get_value_name(i, "name", name);
            format(msg, sizeof msg, "%s%s %s\n", msg, name, access);
        }
    }
    cache_delete(result);
    if(count == 0) {
        format(msg, sizeof msg, "当前服务器没有任何管理员");
    }
    DIALOG_node(playerid, msg);
    return 1;
}

CMD:gms(playerid, params[]) {
    new msg[1024], count = 0, access[128];
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1 && GetPlayerStatus(i) == 1 && userInfo[i][GM_Status] == 1) {
            format(msg, sizeof msg, "%s%s", msg, GetNameEx(i), access);
            count ++;
        }
    }
    if(count == 0) {
        format(msg, sizeof msg, "服务器当前没有任何正在执勤的管理员");
    }
    DIALOG_node(playerid, msg, "在线管理员列表");
    return 1;
}

CMD:ahelp(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_1") == 0) return  DIALOG_node(playerid, "你没有权限使用该命令");
    SCM(playerid, COLOR_RED, " KYSAMP管理系统指令表 欢迎管理员加入狂野大家庭");
	SCM(playerid, COLOR_RED, "1级/2级GM指令 /warn 警告玩家 /say 宣布 /kick 踢出玩家 /goto 传送到玩家 /get 拉玩家到身边  /tv 观察玩家 ; 管理聊天频道 /sxveh 刷新爱车 /clear 清屏");
    SCM(playerid, COLOR_RED, "2级管理指令 /mute 禁言玩家  /jail 放到监狱  /veh 刷车  /giveweapon 给武器  /noveh 禁用载具  /mswq 没收武器");
	SCM(playerid, COLOR_RED, "/djwj 冻结玩家  /muteall 全体禁言  /ban 封号  ");
    SCM(playerid, COLOR_RED, "3级管理指令 /setweather 设置天气  /settime 设置时间");
	SCM(playerid, COLOR_RED, " /muteall 全服禁言 /ban 封禁");
    SCM(playerid, COLOR_RED, "4级管理指令 /deleteplayertitle 删除称号 /giveplayertitle 给称号 /giveallmoney 给所有人钱 /ip 查看玩家IP ");
    SCM(playerid, COLOR_RED, "5级管理指令 /test (命令测试) /givemoney 给现金 /giveallmoney 给所有人钱 /createbillboard 创建公告牌 /deletebillboard 删除公告牌");
    SCM(playerid, COLOR_RED, "新增管理指令 /givescore 给玩家时间分 giveallscore给全服玩家时间分 /IP 查询玩家IP /clear 清屏");
	SCM(playerid, COLOR_RED, "AdvAdmin高层管理指令 /setnamex 伪装 /restartserv 重启服务器 /givebankmoney 给银行钱 /allgms 查看管理员名单 /createteles 创建系统传送点 /createteam 创建团队 /deleteteam 删除团队");
    return 1;
    } 
	
CMD:gm(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /gm login (登录GM)");
        SCM(playerid, COLOR_YELLOW, "[命令] /gm logout (退出GM)");
        SCM(playerid, COLOR_YELLOW, "[命令] /gm help (查看可用命令)");
        return 1;
    }
    new msg[128];
    if(strcmp(params, "login", true) == 0) {
        if(GetGMStatus(playerid) == 1) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你已经进入了执勤状态.");
        SetPlayerColor(playerid, COLOR_RED);
        userInfo[playerid][GM_Status] = 1;
        format(msg, sizeof msg, "[系统] 管理员 %s 已进入执勤状态", GetNameEx(playerid));
        SCMToAll(COLOR_RED, msg);
		AdminCommand(msg);
    } else if(strcmp(params, "logout", true) == 0) {
        if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
        new id = playerid;
        if(id >= 10) {
            id = id / 10;
        }
        SetPlayerColor(playerid, PlayerColors[id]);
        userInfo[playerid][GM_Status] = 0;
        format(msg, sizeof msg, "[系统] 管理员 %s 已退出执勤状态", GetNameEx(playerid));
        SCMToAll(COLOR_RED, msg);
		AdminCommand(msg);
    } else if(strcmp(params, "help", true) == 0) {
        SCM(playerid, COLOR_YELLOW, "1级/2级GM指令 /warn 警告玩家 /say 宣布 /kick 踢出玩家 /goto 传送到玩家 /get 拉玩家到身边  /tv 观察玩家 ; 管理聊天频道 /sxveh 刷新爱车 /clear 清屏");
        if(GetPlayerAccess(playerid, "gm_2") == 1) {
            SCM(playerid, COLOR_YELLOW, "2级管理指令 /mute 禁言玩家  /jail 放到监狱  /veh 刷车  /giveweapon 给武器  /noveh 禁用载具  /mswq 没收武器");
			SCM(playerid, COLOR_YELLOW, "/djwj 冻结玩家  /muteall 全体禁言  /ban 封号  ");
        }
        if(GetPlayerAccess(playerid, "gm_3") == 1) {
            SCM(playerid, COLOR_YELLOW, "3级管理指令 /setweather 设置天气  /settime 设置时间");
			SCM(playerid, COLOR_YELLOW, " /muteall 全服禁言 /ban 封禁");
        }
        if(GetPlayerAccess(playerid, "gm_4") == 1) {
            SCM(playerid, COLOR_YELLOW, "4级管理指令 /deleteplayertitle 删除称号 /giveplayertitle 给称号 /giveallmoney 给所有人钱 /ip 查看玩家IP ");
        }
        if(GetPlayerAccess(playerid, "gm_5") == 1) {
            SCM(playerid, COLOR_YELLOW, "5级管理指令 /test (命令测试) /givemoney 给现金 /giveallmoney 给所有人钱 /createbillboard 创建公告牌 /deletebillboard 删除公告牌");
        }
        if(GetPlayerAccess(playerid, "admin") == 1) {
            SCM(playerid, COLOR_YELLOW, "admin高层管理指令 /setnamex 伪装 /restartserv 重启服务器 /givebankmoney 给银行钱 /allgms 查看管理员名单 /createteles 创建系统传送点 /createteam 创建团队 /deleteteam 删除团队");
        }
    } else {
        SCM(playerid, COLOR_RED, "[系统] 错误的内容.");
    }
    return 1;
}

CMD:deleteplayertitle(playerid, params[]) {
	if(GetPlayerAccess(playerid, "gm_4") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
	new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /deleteplayertitle [PID] [称号名] [原因]");
        return 1;
    }
	new uid = strval(tmp);
	if(IsUIDExist(uid) != 1) {
		SCM(playerid, COLOR_RED, "[系统] 错误的PID.");
        return 1;
	}
	tmp = strtok(params, idx);
	if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 称号名不能为空.");
        return 1;
    }
	if(strlen(tmp) > MAX_PLAYER_NAME) {
		SCM(playerid, COLOR_RED, "[系统] 称号过长.");
        return 1;
	}
	if(IsUIDHasTitle(uid, tmp) == 0) {
		SCM(playerid, COLOR_RED, "[系统] 该玩家没有这个称号");
        return 1;
	}
	new titlename[MAX_PLAYER_NAME];
	format(titlename, sizeof titlename, tmp);
	tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
	DeleteUIDTitle(uid, titlename);
	new msg[144];
	format(msg, sizeof msg, "[系统] {FFFF00}管理员 {FF0000}%s {FFFF00}移除了称号: {FFFF00}%s {FFFF00}玩家名: {00FFFF}%s{FFFF00}, 理由: %s", GetNameEx(playerid), titlename, GetUIDName(uid), tmp);
	SCMToAll(COLOR_RED, msg);
	AdminCommand(msg);
	return 1;
}

CMD:sxveh(playerid, params[]) {
	if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new i, v, a, j, string[144];
    for(i = 1, v = a = 0, j = GetVehiclePoolSize(); i <=j ; i++)//使用GetVehiclePoolSize获取最大车辆编号来增加效率
    {
        if(GetVehicleModel(i) < 400) continue;
        a++;
        if(IsVehicleOccupied(i)) continue;
		SetVehicleToRespawn(i);
		v++;
	}
	format(string, sizeof(string), "[系统]: 管理员刷新了所有无人载具完成,总计重生了共 %d/%d 台",v,a);
	SendClientMessageToAll(COLOR_RED, string);
    initAcMenu();
	AdminCommand(string);
	return 1;
}

CMD:ip(playerid, params[])
	{
		if(GetPlayerAccess(playerid, "gm_4") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
        if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
		new id;
		if(sscanf(params, "i", id))return SendClientMessage(playerid, COLOR_RED, "获取玩家IP /ip [id]");
		new plrIP[64];
		GetPlayerIp(id, plrIP, sizeof(plrIP));
		new pid = strval(params);
        if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
		new string[128];
		format(string, sizeof(string), "该玩家%s的IP地址为:%s ",GetName(pid),plrIP);
		SendClientMessage(playerid,COLOR_RED,string);
		AdminCommand(string);
		return 1;
	}
	
CMD:clear(playerid, params[])
{
	if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
	for(new i = 0; i <= 50; i++)
	{
	SCMToAll(COLOR_WHITE, " ");
	}
	new string[128];
	format(string, sizeof(string), "[管理系统]管理员%s(%d)清空了公屏 ",GetName(playerid));
    SendClientMessageToAll(COLOR_RED,string);
	AdminCommand(string);
	//SCMToAll(COLOR_RED,"[管理系统]公屏聊天已被管理员清屏!");
	return 1;
}
		
CMD:giveplayertitle(playerid, params[]) {
	if(GetPlayerAccess(playerid, "gm_4") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
	new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /giveplayertitle [PID] [称号名] [原因]");
        return 1;
    }
	new uid = strval(tmp);
	if(IsUIDExist(uid) != 1) {
		SCM(playerid, COLOR_RED, "[系统] 错误的PID.");
        return 1;
	}
	tmp = strtok(params, idx);
	if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 称号名不能为空.");
        return 1;
    }
	if(strlen(tmp) > MAX_PLAYER_NAME) {
		SCM(playerid, COLOR_RED, "[系统] 称号名过长.");
        return 1;
	}
	new titlename[MAX_PLAYER_NAME];
	format(titlename, sizeof titlename, tmp);
	tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
	GiveUIDTitle(uid, titlename, GetName(playerid));
	new msg[144];
	format(msg, sizeof msg, "[系统] {FFFF00}管理员 {FF0000}%s {FFFF00}给了{FFFF00} %s  {FFFF00}称号:{00FF00}%s{FFFF00}, 原因:%s", GetNameEx(playerid), GetUIDName(uid), titlename, tmp);
	SCMToAll(COLOR_RED, msg);
	AdminCommand(msg);
	return 1;
}

CMD:deletebillboard(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_5") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /deletebillboard [公告牌ID] [原因]");
        return 1;
    }
    new bid = strval(tmp);
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(BillBoard_Delete(bid) == 0) {
        DIALOG_node(playerid, "删除失败, 原因:公告牌ID错误");
    } else {
        format(tmp, sizeof tmp, "你删除了公告牌ID:%d", bid);
        DIALOG_node(playerid, tmp);
		AdminCommand(tmp);
    }
    return 1;
}

CMD:createbillboard(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_5") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /createbillboard [OBJID:9314(小牌子)/7606(大牌子)] [0] [价格]");
        return 1;
    }
    new objid = strval(tmp);
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 索引不能为空.");
        return 1;
    }
    new bbidx = strval(tmp);
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 价格不能为空.");
        return 1;
    }
    new price = strval(tmp);
    if(price < 1) {
        SCM(playerid, COLOR_RED, "[系统] 价格不能小于1.");
        return 1;
    }
    BillBoard_Create(playerid, objid, bbidx, price);
    return 1;
}

CMD:giveallmoney(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_4") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /giveallmoney [金额] [原因]");
        return 1;
    }
    new money = strval(tmp);
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1 && GetPlayerStatus(i) == 1) {
            GiveMoney(i, money);
        }
    }
    new msg[144];
    format(msg, sizeof msg, "[系统] {FFFF00}管理员{FF0000} %s {FFFF00}给了所有人 ${00FF00}%d{FFFF00}, 原因:%s", GetNameEx(playerid), money, tmp);
    SCMToAll(COLOR_RED, msg);
    format(tmp, sizeof tmp, "(权限-现金-所有人) %s", tmp);
    AdminCommand(msg);
	log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "none", "none", 0, money, tmp);
    return 1;
}

CMD:setweather(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_3") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /setweather [天气ID0-20]");
        return 1;
    }
    new weather = strval(params);
    if(weather < 0 || weather > 20) {
        SCM(playerid, COLOR_RED, "[系统] 错误的天气ID.");
        return 1;
    }
    timer_GMSetWeather(weather);
    new msg[144];
    format(msg, sizeof msg, "[系统] {FFFF00}管理员 {FF0000}%s {FFFF00}把系统模拟天气设置为了ID:%d (%s).", GetNameEx(playerid), weather, GetWeatherNames[weather]);
    SCMToAll(COLOR_RED, msg);
	AdminCommand(msg);
    return 1;
}

CMD:settime(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_3") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /settime [时间0-23]");
        return 1;
    }
    new time = strval(params);
    if(time < 0 || time > 23) {
        SCM(playerid, COLOR_RED, "[系统] 错误的时间.");
        return 1;
    }
    timer_GMSetTime(time);
    new msg[144];
    format(msg, sizeof msg, "[系统] {FFFF00}管理员 {FF0000}%s {FFFF00}把系统模拟时间设置为了{00FF00}%d{FFFF00}点.(/SZ 可屏蔽该设置)", GetNameEx(playerid), time);
    SCMToAll(COLOR_RED, msg);
	AdminCommand(msg);
    return 1;
}

CMD:ban(playerid, params[]) {
    //if(!IsPlayerAdmin(playerid)) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /ban [对方ID] [时间(1h(1小时), 1d(1天), -1(永久)] [类型(ip/name)] [原因]");
        return 1;
    }
    new pid = strval(tmp);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 时间不能为空.");
        return 1;
    }
    new bantime = strval(tmp), hod = -1;
    if(bantime == -1) {
        hod = 2;
    } else {
        if(tmp[strlen(tmp)-1] == 'h') {
            strdel(tmp, strlen(tmp)-1, strlen(tmp));
            bantime = strval(tmp);
            if(bantime < 1) {
                SCM(playerid, COLOR_RED, "[系统] 封号时间不能小于1小时.");
                return 1;
            }
            hod = 0;
        } else if(tmp[strlen(tmp)-1] == 'd') {
            strdel(tmp, strlen(tmp)-1, strlen(tmp));
            bantime = strval(tmp);
            if(bantime < 1) {
                SCM(playerid, COLOR_RED, "[系统] 封号时间不能小于1天.");
                return 1;
            }
            hod = 1;
        } else {
            SCM(playerid, COLOR_RED, "[系统] 错误的封禁时间.");
            return 1;
        }
    }
    new type[128];
    type = strtok(params, idx);
    if(!strlen(type)) {
        SCM(playerid, COLOR_RED, "[系统] 类型不能为空.");
        return 1;
    }
    if(strcmp(type, "ip") != 0 && strcmp(type, "name") != 0) {
        SCM(playerid, COLOR_RED, "[系统] 类型只能为ip或name.");
        return 1;
    }
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    new msg[144], seconds;
    if(hod == 0) {
        seconds = bantime * 3600;
        format(msg, sizeof msg, "[系统] 管理员 %s 封杀了 %s 期限:%d小时, 原因:%s", GetNameEx(playerid), GetName(pid), bantime, tmp);
    } else if(hod == 1) {
        seconds = bantime * 86400;
        format(msg, sizeof msg, "[系统] 管理员 %s 封杀了 %s 期限:%d天, 原因:%s", GetNameEx(playerid), GetName(pid), bantime, tmp);
    } else if(hod == 2) {
        seconds = -1;
        format(msg, sizeof msg, "[系统] 管理员 %s 封杀了 %s 期限:永久, 原因:%s", GetNameEx(playerid), GetName(pid), tmp);
    }
    SCMToAll(COLOR_RED, msg);
    accountSetBanTime(pid, seconds, tmp);
    format(tmp, sizeof tmp, "(ban-%s) %s", type, tmp);
    log_ban(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), GetName(pid), GetIp(pid), GetPlayerUID(pid), seconds, tmp);
	AdminCommand(msg);
    if(strcmp(type, "ip") == 0) {
        BanExx(pid, "被封号");
    } else {
        KickEx(pid, "被封号");
    }
    return 1;
}

CMD:givebankmoney(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /givebankmoney [对方ID] [金额] [原因](扣钱用负数)");
        return 1;
    }
    new pid = strval(tmp);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    if(GetPlayerStatus(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 对方未登录.");
        return 1;
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 金额不能为空.");
        return 1;
    }
    new money = strval(tmp);
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    new msg[144];
    format(msg, sizeof msg, "[系统] {FFFF00}管理员 {FF0000}%s {FFFF00}给了 %s ${00FF00}%d {FFFF00}银行存款, 原因:%s", GetNameEx(playerid), GetName(pid), money, tmp);
    SCMToAll(COLOR_RED, msg);
    GiveSavings(pid, money);
    format(tmp, sizeof tmp, "(权限-银行余额) %s", tmp);
    log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), GetName(pid), GetIp(pid), GetPlayerUID(pid), money, tmp);
	AdminCommand(msg);
    return 1;
}

CMD:givemoney(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_5") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /givemoney [对方ID] [金额] [原因](扣钱用负数)");
        return 1;
    }
    new pid = strval(tmp);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    if(GetPlayerStatus(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 对方未登录.");
        return 1;
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 金额不能为空.");
        return 1;
    }
    new money = strval(tmp);
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    new msg[144];
    format(msg, sizeof msg, "{FF0000}[系统] {FFFF00}高层管理员 {FF0000}%s {FFFF00}给了 %s ${00FF00}%d{FFFF00}, 原因:%s", GetNameEx(playerid), GetName(pid), money, tmp);
    SCMToAll(COLOR_RED, msg);
    GiveMoney(pid, money);
    format(tmp, sizeof tmp, "(权限-现金) %s", tmp);
    log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), GetName(pid), GetIp(pid), GetPlayerUID(pid), money, tmp);
    AdminCommand(msg);
	return 1;
}

CMD:deletetmpveh(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /deletetmpveh [载具ID]");
        SCM(playerid, COLOR_YELLOW, "[命令] /deletetmpveh all (可删除所有已创建的载具)");
        return 1;
    }
    new msg[144];
    if(strcmp(params, "all", true) == 0) {
        for(new i = 0; i < MAX_VEHICLES; i ++) {
            if(admin_tmp_veh[i] == 1) {
                admin_tmp_veh[i] = 0;
                DestroyVehicle(i);
            }
        }
        format(msg, sizeof msg, "[系统] 管理员 %s 删除了所有的临时载具.", GetNameEx(playerid));
        SCMToAll(COLOR_RED, msg);
        return 1;
    }
    new vid = strval(params);
    if(vid < 0 || vid > MAX_VEHICLES || admin_tmp_veh[vid] == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的载具ID.");
        return 1;
    }
    admin_tmp_veh[vid] = 0;
    DestroyVehicle(vid);
    format(msg, sizeof msg, "[系统] 管理员 %s 删除了临时载具ID:%d", GetNameEx(playerid), vid);
    SCMToAll(COLOR_RED, msg);
    AdminCommand(msg);
	return 1;
}

CMD:veh(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] veh [载具ID]");
        return 1;
    }
    new modelid = strval(params);
    if(modelid < 400 || modelid > 611) {
        SCM(playerid, COLOR_RED, "[系统] 错误的载具ID.");
        return 1;
    }
    new Float:x, Float:y, Float:z, Float:a = GetPlayerAngle(playerid);
    GetPlayerPos(playerid, x, y, z);
    new vid = CreateVehicle(modelid, x+1, y+1, z+1, a, 0, 0, -1);
    if(vid == INVALID_VEHICLE_ID) {
        SCM(playerid, COLOR_RED, "[系统] 创建失败, 原因:服务器载具已达到上限.");
        return 1;
    }
    admin_tmp_veh[vid] = 1;
    new msg[144];
    format(msg, sizeof msg, "[系统] 管理员 %s 创建了一辆名为 %s 的载具.", GetNameEx(playerid), GetVehicleModelName[modelid]);
    SCMToAll(COLOR_RED, msg);
    AdminCommand(msg);
	return 1;
}

CMD:giveweapon(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new idx, tmp[128];
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /giveweapon [对方ID] [武器ID]");
        return 1;
    }
    new pid = strval(tmp);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 武器ID不能为空.");
        return 1;
    }
    new wid = strval(tmp);
    if(wid < 0 || wid > sizeof GetWeaponNames) {
        SCM(playerid, COLOR_RED, "[系统] 错误的武器ID.");
        return 1;
    }
    GivePlayerWeapon(pid, wid, 5000);
    new msg[144];
    format(msg, sizeof msg, "[系统] {FFFF00}管理员 {FF0000}%s {FFFF00}给了{FFFF00} %s {FFFF00}一把名为 {00FF00}%s {FFFF00}的武器.", GetNameEx(playerid), GetName(pid), GetWeaponNames[wid]);
    SCMToAll(COLOR_RED, msg);
    AdminCommand(msg);
	return 1;
}

CMD:givescore(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_5") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "给玩家时间分/givescore [对方ID] [时间分] [原因](扣钱用负数)");
        return 1;
    }
    new pid = strval(tmp);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    if(GetPlayerStatus(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 对方未登录.");
        return 1;
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 时间分不能为空.");
        return 1;
    }
    new onlinetime = strval(tmp);
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    new msg[144];
    format(msg, sizeof msg, "[管理系统] {FF00FF}高层管理员 %s 给予玩家 %s %d时间分, 原因:%s", GetNameEx(playerid), GetName(pid), onlinetime, tmp);
    SCMToAll(COLOR_RED, msg);
    GiveOnlineTime(pid, onlinetime);
    format(tmp, sizeof tmp, "(权限-时间分) %s", tmp);
	AdminCommand(msg);
    //log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), GetName(pid), GetIp(pid), GetPlayerUID(pid), money, tmp);
    return 1;
}

CMD:giveallscore(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_5") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[给全服玩家时间分] /giveallscore [时间分] [原因]");
        return 1;
    }
    new onlinetime = strval(tmp);
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1 && GetPlayerStatus(i) == 1) {
            GiveOnlineTime(i, onlinetime);
        }
    }
    new msg[144];
    format(msg, sizeof msg, "[系统] {FF00FF}高层管理员 给全服玩家发了%d时间分, 原因:%s", onlinetime, tmp);
    SCMToAll(COLOR_RED, msg);
    format(tmp, sizeof tmp, "(权限-时间分-所有人) %s", tmp);
    AdminCommand(msg);
	//log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "none", "none", 0, money, tmp);
    return 1;
}

CMD:noveh(playerid, params[]) {
    //forbid use veh
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /noveh [对方ID]");
        return 1;
    }
    new pid = strval(params);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    new msg[144];
    if(userInfo[pid][NoVeh] == 0) {
        format(msg, sizeof msg, "[系统] 管理员 %s 禁止了 %s 使用载具.", GetNameEx(playerid), GetName(pid));
    } else {
        format(msg, sizeof msg, "[系统] 管理员 %s 解除了 %s 的禁止使用载具.", GetNameEx(playerid), GetName(pid));
    }
    accountSetNoVEH(pid);
    SCMToAll(COLOR_RED, msg);
    AdminCommand(msg);
	return 1;
}

CMD:mswq(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /mswq [对方ID]");
        return 1;
    }
    new pid = strval(params);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    new msg[144];
    format(msg, sizeof msg, "[系统] 管理员 %s 没收了 %s 的武器.", GetNameEx(playerid), GetName(pid));
    SCMToAll(COLOR_RED, msg);
    ResetPlayerWeapons(pid);
    AdminCommand(msg);
	return 1;
}

CMD:djwj(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /djwj [对方ID]");
        return 1;
    }
    new pid = strval(params);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    new msg[144];
    if(userInfo[pid][Freeze] == 0) {
        userInfo[pid][Freeze] = 1;
        format(msg, sizeof msg, "[系统] 管理员 %s 解冻了玩家 %s", GetNameEx(playerid), GetName(pid));
    } else {
        userInfo[pid][Freeze] = 0;
        format(msg, sizeof msg, "[系统] 管理员 %s 冻结了玩家 %s", GetNameEx(playerid), GetName(pid));
    }
    SCMToAll(COLOR_RED, msg);
    TogglePlayerControllable(pid, userInfo[pid][Freeze]);
    AdminCommand(msg);
	return 1;
}

CMD:muteall(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new idx, tmp[128];
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] / [时间/秒] [原因]");
        return 1;
    }
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 时间不能为空.");
        return 1;
    }
    new time = strval(tmp);
    if(time < 1) {
        SCM(playerid, COLOR_RED, "[系统] 时间不能小于1秒.");
        return 1;
    }
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    for(new i = 0, all = GetPlayerPoolSize(); i <= all; i ++) {
        if(IsPlayerConnected(i) == 1 && GetPlayerStatus(i) == 1 && GetPlayerAccess(i, "gm_1") == 0) {
            MutePlayer(i, time);
        }
    }
    new msg[144];
    format(msg, sizeof msg, "[系统] 管理员 %s 把所有人禁言了 %d秒, 原因:%s", GetNameEx(playerid), time, tmp);
    SCMToAll(COLOR_RED, msg);
    format(msg, sizeof msg, "(muteall) %s", tmp);
    log_mute(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "muteall", "muteall", 0, time, msg);
    AdminCommand(msg);
	return 1;
}

CMD:mute(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new idx, tmp[128];
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /mute [对方ID] [时间/秒] [原因]");
        return 1;
    }
    new pid = strval(tmp);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 时间不能为空.");
        return 1;
    }
    new time = strval(tmp);
    if(time < 1) {
        SCM(playerid, COLOR_RED, "[系统] 时间不能小于1秒.");
        return 1;
    }
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    log_mute(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), GetName(pid), GetIp(pid), GetPlayerUID(pid), time, tmp);
    new msg[144];
    format(msg, sizeof msg, "[系统] 管理员 %s 把 %s 禁言了 %d秒, 原因:%s", GetNameEx(playerid), GetName(pid), time, tmp);
    SCMToAll(COLOR_RED, msg);
    MutePlayer(pid, time);
    AdminCommand(msg);
	return 1;
}

CMD:jail(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new idx, tmp[128];
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /jail [对方ID] [时间/秒] [原因]");
        return 1;
    }
    new pid = strval(tmp);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 时间不能为空.");
        return 1;
    }
    new time = strval(tmp);
    if(time < 1) {
        SCM(playerid, COLOR_RED, "[系统] 时间不能小于1秒.");
        return 1;
    }
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    log_jail(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), GetName(pid), GetIp(pid), GetPlayerUID(pid), time, tmp);
    new msg[144];
    format(msg, sizeof msg, "[系统] 管理员 %s 把 %s 关进了监狱 %d秒, 原因:%s", GetNameEx(playerid), GetName(pid), time, tmp);
    SCMToAll(COLOR_RED, msg);
    throwInJail(pid, time);
    AdminCommand(msg);
	return 1;
}

CMD:warn(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "警告玩家 /warn [对方ID][原因]");
        return 1;
    }
    new pid = strval(params);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
	tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    new msg[144];
    if(userInfo[pid][Warn]) {
        format(msg, sizeof msg, "[系统] 管理员 %s 警告玩家 %s 原因:%s", GetNameEx(playerid), GetName(pid), tmp, userInfo[pid][Warn]);
        SCMToAll(COLOR_RED, msg);
        //SCM(pid, COLOR_RED, "[系统] 屡次警告无效, 你被关进了禁闭室10分钟");
        userInfo[pid][Warn] = 0;
        //throwInJail(pid);
    } else {
        userInfo[pid][Warn] ++;
        format(msg, sizeof msg, "[系统] 管理员 %s 警告玩家 %s 原因%s", GetNameEx(playerid), GetName(pid), tmp,  userInfo[pid][Warn]);
        SCMToAll(COLOR_RED, msg);
		AdminCommand(msg);
        new Float:x, Float:y, Float:z;
        GetPlayerPos(pid, x, y, z);
        SetPlayerPos(pid, x, y, z + 3);
    }
    return 1;
}

CMD:say(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /say [内容]");
        return 1;
    }
    new msg[144];
    format(msg, sizeof msg, "** 管理员 %s 宣布: %s", GetNameEx(playerid), params);
    SCMToAll(COLOR_RED, msg);
	AdminCommand(msg);
    return 1;
}

CMD:kick(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_2") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    new idx, tmp[128];
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /kick [对方ID] [原因]");
        return 1;
    }
    new pid = strval(tmp);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    tmp = strrest(params, idx);
    if(!strlen(tmp)) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能为空.");
        return 1;
    }
    if(strlen(tmp) > 64) {
        SCM(playerid, COLOR_RED, "[系统] 原因不能大于64字符.");
        return 1;
    }
    log_kick(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), GetName(pid), GetIp(pid), GetPlayerUID(pid), tmp);
    new msg[144];
    format(msg, sizeof msg, "[系统] 管理员 %s 把 %s 踢出了服务器, 原因:%s", GetNameEx(playerid), GetName(pid), tmp);
    SCMToAll(COLOR_RED, msg);
	AdminCommand(msg);
    KickEx(pid, "被管理员踢出");
    return 1;
}

CMD:goto(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /goto [对方ID]");
        return 1;
    }
    new pid = strval(params);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    new Float:x, Float:y, Float:z, msg[128];
    GetPlayerPos(pid, x, y, z);
    SetPlayerPosEx(playerid, x, y + 1, z + 1);
    format(msg, sizeof msg, "[系统] 管理员 %s 传送到了你的身边.", GetNameEx(playerid));
    SCM(pid, COLOR_BLUE, msg);
    format(msg, sizeof msg, "[系统] 你传送到了 %s 的身边.", GetName(pid));
    SCM(playerid, COLOR_BLUE, msg);
	AdminCommand(msg);
    return 1;
}

CMD:get(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /get [对方ID]");
        return 1;
    }
    new pid = strval(params);
    if(IsPlayerConnected(pid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
    new Float:x, Float:y, Float:z, msg[128];
    GetPlayerPos(playerid, x, y, z);
    SetPlayerPosEx(pid, x, y + 1, z + 1);
    format(msg, sizeof msg, "[系统] 管理员 %s 把你拉到了他的身边.", GetNameEx(playerid));
    SCM(pid, COLOR_BLUE, msg);
    format(msg, sizeof msg, "[系统] 你把 %s 拉到了你的身边.", GetName(pid));
    SCM(playerid, COLOR_BLUE, msg);
	AdminCommand(msg);
    return 1;
}

CMD:shuache(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
	if(!strlen(params)) {
		SCM(playerid, COLOR_YELLOW, "[命令] /shuache [载具ID]");
		return 1;
	}
    new vid = strval(params);
    if(vid < 1 || vid > GetVehiclePoolSize()) {
        SCM(playerid, COLOR_RED, "[系统] 错误的载具ID.");
        return 1;
    }
    ACRespawn(vid);
    new msg[128];
    format(msg, sizeof msg, "[系统] 你刷新了载具ID: %d", vid);
    SCM(playerid, COLOR_BLUE, msg);
    return 1;
}

CMD:tv(playerid, params[]) {
    if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
	if(!strlen(params)) {
		SCM(playerid, COLOR_YELLOW, "[命令] 开启: /tv [玩家id]");
		SCM(playerid, COLOR_YELLOW, "[命令] 关闭: /tv off");
		return 1;
	}
    if(strcmp(params, "off", true) == 0) {
        TogglePlayerSpectating(playerid, 0);
        SetPlayerVirtualWorld(playerid, 0);
        SetPlayerInterior(playerid, 0);
        return 1;
    }
	new targetid = strval(params);
    if(IsPlayerConnected(targetid) == 0) {
        SCM(playerid, COLOR_RED, "[系统] 错误的对方ID.");
        return 1;
    }
	TogglePlayerSpectating(playerid, 1);
	SetPlayerVirtualWorld(playerid,GetPlayerVirtualWorld(targetid));
	SetPlayerInterior(playerid,GetPlayerInterior(targetid));
	if(IsPlayerInAnyVehicle(targetid)) {
        PlayerSpectateVehicle(playerid, GetPlayerVehicleID(targetid));
    } else {
        PlayerSpectatePlayer(playerid, targetid);
	}
    return 1;
}

CMD:resetteamleader(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /deleteteam [名称] [新领导人]");
        return 1;
    }
    new idx;
    teamResetLeader(playerid, strtok(params, idx), strtok(params, idx));
    return 1;
}

CMD:deleteteam(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /deleteteam [名称]");
        return 1;
    }
    teamDelete(playerid, params);
    return 1;
}

CMD:createteam(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /createteam [名称] [标签] [领导人]");
        SCM(playerid, COLOR_YELLOW, "例子: /createteam Shadows 俱乐部 [Shadows]Dylan");
        return 1;
    }
    new idx;
    teamCreate(playerid, strtok(params, idx), strtok(params, idx), strtok(params, idx));
    return 1;
}

CMD:createteles(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
    if(!strlen(params)) {
        SCM(playerid, COLOR_YELLOW, "[命令] /createteles [传送点] [介绍]");
        SCM(playerid, COLOR_YELLOW, "例子: /createteles ldz 雷达站");
        return 1;
    }
    new idx;
    teleCSlash(playerid, strtok(params, idx), strtok(params, idx));
    return 1;
}

CMD:setnamex(playerid, params[]) {
    if(GetPlayerAccess(playerid, "admin") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
    if(!strlen(params)) {
        SCM(playerid, COLOR_WHITE, "[命令] /setnamex [name] (rollback = oldname)");
        return 1;
    }
    new msg[128], name[MAX_PLAYER_NAME];
    format(name, sizeof name, (strcmp(params, "rollback") == 0 ? (GetName(playerid)) : (params)));
    SetPlayerNameEx(playerid, name);
    format(msg, sizeof msg, "你把NameEx设置为了: %s", name);
    DIALOG_node(playerid, msg);
	AdminCommand(msg);
    return 1;

}
//Script
