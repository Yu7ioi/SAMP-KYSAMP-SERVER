#include <socket>
#include <crashdetect>

#define BIND_WEICHAT_MONEY		(50000)
#define UCP_GM_ACCESS_DENIED	"你没有权限使用该功能"

enum ucptype {
	Socket:HandleId,
	HeartbeatMAX,
	HeartbeatNOW,
	GOCLIENT
};

new UCPInfo[ucptype];

stock UCPInit() {
	UCPInfo[HeartbeatMAX] = 30;
	UCPInfo[HeartbeatNOW] = 0;
	UCPInfo[GOCLIENT] = -1;
	UCPInfo[HandleId] = socket_create(TCP);
	if(is_socket_valid(UCPInfo[HandleId])) {
		socket_set_max_connections(UCPInfo[HandleId], 10);
		socket_listen(UCPInfo[HandleId], 9931);
	}  
}

public onSocketReceiveData(Socket:id, remote_clientid, reivdata[], data_len) {
	if(strlen(reivdata) > 18) {
		strdel(reivdata, 0, 17);
	}
	if(strcmp(reivdata, "UCP_CREATE_CONN") == 0) {
		UCPSendChat("SAMP_CONNECTED");
		return 1;
	}
	if(UCPIsAlive() == 1) {
		for(new i = 0; i < strlen(reivdata); i ++) {
			if(reivdata[i] == 94) {
				reivdata[i] = ' ';
			}
		}
		new tmp[128], idx, name[MAX_PLAYER_NAME];
		tmp = strtok(reivdata, idx);
		if(strcmp(tmp, "UCPTEXT") == 0) {
			new msg[144];
			format(msg, sizeof msg, strrest(reivdata, idx));
			SCMToAll(COLOR_RED, msg);
			format(name, sizeof name, msg);
			idx = strfind(msg, "(UCP):");
			strdel(name, idx, strlen(msg));//删除 name(UCP):
			strdel(msg, 0, idx+7);//删除 (UCP): 
			log_chat(name, "UCP", msg, 0);
			return 1;
		} else if(strcmp(tmp, "UCPCMD") == 0) {
			new cookie[64], isgeth[MAX_PLAYER_NAME];
			sscanf(reivdata, "s[128] s[24] s[64] s[128]", tmp, name, cookie, tmp);
			sscanf(tmp, "s[24] ", isgeth);
			if(strcmp(isgeth, "phsssssssssssss") == 0) {
				new pname[MAX_PLAYER_NAME];
				sscanf(tmp, "s[24] s[24]", isgeth, pname);
				UCPSendPM(name, PHouseGetOwnerForUCP(pname), "GPID");
			} else {
				UCPCMDs(name, cookie, tmp);
			}
			return 1;
		} else if(strcmp(tmp, "UCPSC") == 0) {
			new cmd[MAX_PLAYER_NAME];
			sscanf(reivdata, "s[24] s[24] s[24] s[128]", tmp, cmd, name, tmp);
			UCPSCs(cmd, name, tmp);
			return 1;
		} else if(strcmp(tmp, "UCPVOICE") == 0) {
			new voicefile[256], value[128];
			sscanf(reivdata, "s[24] s[256] s[128]", tmp, voicefile, value);
			format(tmp, sizeof tmp, "%s", value);
			SCMToAll(COLOR_GREEN, tmp);
			PlayerMusicForAll(voicefile);
		}
	}
	return 1;
}

stock UCPSCs(cmd[], name[], params[]) {
	new tmp[128], idx;
	if(strcmp(cmd, "reloadattire") == 0) {
		new playerid = isNameOnline(name);
		new money = strval(strtok(params, idx));
		if(playerid != -1) {
			GiveSavings(playerid, -money);
			log_money(name, GetIp(playerid), GetPlayerUID(playerid), "UCP", "装扮商店", 0, -money, "购买装扮");
			Attire.load(playerid);
			Attire.Spawn(playerid);
		} else {
			GiveSavingsOffline(name, -money);
			log_money(name, "UCP", 0, "UCP", "装扮商店", 0, -money, "购买装扮");
		}
		return 1;
	} else if(strcmp(cmd, "vehiclepayfine") == 0) {
		new sid[128];
		sscanf(params, "s[128] s[24]", sid, tmp);
		new created_at = strval(tmp);
		new vid = GetVehicleIdFromCreatedAt(created_at);
		if(vid == -1) {
			UCPSendPM(name, "ERR:NOT_EXIST_VID");
			return 1;
		}
		new fine = GetVehicleTicketPrice(vid);
		new playerid = isNameOnline(name);
		new Cache:result, savings;
		format(tmp, sizeof tmp, "SELECT savings FROM account WHERE name='%s' AND cookie='%s'", name, sid);
		result = mysql_query(mariadb, tmp);
		cache_get_value_name_int(0, "savings", savings);
		cache_delete(result);
		if(playerid == -1) {
			if(savings < fine) {
				UCPSendPM(name, "你的存款不够");
			} else {
				GiveSavingsOffline(name, -fine);
				log_money(name, "UCP", 0, "UCP", "载具罚单", 0, -fine, "UCP-交载具罚款");
				UCPSendPM(name, "交款成功");
				ACCancelTicket(vid);
			}
		} else {
			if(savings < fine) {
				UCPSendPM(name, "你的存款不够");
			} else {
				GiveSavings(playerid, -fine);
				accountSaveData(playerid);
				log_money(name, GetIp(playerid), GetPlayerUID(playerid), "UCP", "载具罚单", 0, -fine, "UCP-交载具罚款");
				UCPSendPM(name, "交款成功");
				ACCancelTicket(vid);
			}
		}
		return 1;
	} else if(strcmp(cmd, "transfer") == 0) {
		new pname[MAX_PLAYER_NAME], ip[MAX_PLAYER_NAME], uid, money, pid, playerid;
		uid = strval(strtok(params, idx));
		money = strval(strtok(params, idx));
		pid = isUIDOnline(uid);
		playerid = isNameOnline(name);
		format(pname, sizeof pname, "%s", (pid == -1 ? (GetUIDName(uid)) : (GetName(pid))));
		format(ip, sizeof ip, "%s", (pid == -1) ? ("offline") : (GetIp(pid)));
		new myip[MAX_PLAYER_NAME], myuid = GetNameUID(name);
		format(myip, sizeof myip, "%s", (playerid == -1) ? ("offline") : (GetIp(playerid)));
		if(playerid == -1) {
			GiveSavingsOffline(name, -money);
		} else {
			GiveSavings(playerid, -money);
			accountSaveData(playerid);
		}
		log_money(name, myip, myuid, pname, ip, uid, -money, "UCP-转账"); 
		if(pid == -1) {
			GiveSavingsOffline(pname, money);
		} else {
			GiveSavings(pid, money);
			accountSaveData(pid);
			format(tmp, sizeof tmp, "[UCP-转账] %s 转给了你 $%d", name, money);
			SCM(pid, COLOR_BLUE, tmp);
		}
		log_money(pname, ip, uid,name, myip, myuid, money, "UCP-转账获得");
		format(tmp, sizeof tmp, "你转给了 %s $%d", pname, money);
		UCPSendPM(name, tmp);
		return 1;
	} else if(strcmp(cmd, "bindwechat") == 0) {
		new playerid = isNameOnline(name);
		if(playerid == -1) {
			GiveMoneyOffline(name, BIND_WEICHAT_MONEY);
		} else {
			GiveMoney(playerid, BIND_WEICHAT_MONEY);
			accountSaveData(playerid);
		}
		log_money(name, "UCP", 0, "UCP", "绑定微信号获得", 0, BIND_WEICHAT_MONEY, "UCP-绑定微信号获得");
		format(tmp, sizeof tmp, "[系统] %s 获得了 $%d, 原因:绑定微信号成功.", name, BIND_WEICHAT_MONEY);
		SCMToAll(COLOR_RED, tmp);
		format(tmp, sizeof tmp, "绑定成功 +$%d", BIND_WEICHAT_MONEY);
		UCPSendPM(name, tmp);
		return 1;
	}
	return 1;
}

public onSocketRemoteConnect(Socket:id, remote_client[], remote_clientid) {
	if(strcmp(remote_client, "127.0.0.1") == 0) {
		if(UCPInfo[GOCLIENT] != -1) {
			socket_close_remote_client(UCPInfo[HandleId], UCPInfo[GOCLIENT]);
		}
		UCPInfo[GOCLIENT] = remote_clientid;
		printf("UCP_CONNECTED");
	}
	return 1;
}

UCPHeartBeat() {
	if(UCPInfo[HeartbeatNOW] >= UCPInfo[HeartbeatMAX]) {
		UCPSendChat("SAMP_CONNECTED");
        UCPInfo[HeartbeatNOW] = 0;
    }
    UCPInfo[HeartbeatNOW] ++;
}

/*
public onUDPReceiveData(Socket:id, reivdata[], data_len, remote_client_ip[], remote_client_port)  {
	return 1;
}*/

stock UCPIsAlive() {
	if(UCPInfo[GOCLIENT] == -1) {
		return 0;
	}
	return 1;
}

stock UCPSendPM(name[], str[], type[] = " ") {
	if(UCPIsAlive() == 1) {
		if(strcmp(type, " ") == 0) {
			format(type, 2, "");
		}
		new msg[144];
		format(msg, sizeof msg, "pm-~-%s-~-%s%s", name, type, str);
		socket_sendto_remote_client(UCPInfo[HandleId], UCPInfo[GOCLIENT], msg);
	}
}

stock UCPSendChat(str[]) {
	if(UCPIsAlive() == 1) {
		socket_sendto_remote_client(UCPInfo[HandleId], UCPInfo[GOCLIENT], str);
	}
}

stock UCPCMDs(name[], cookie[], params[]) {
	new Cache:result, tmp[128], access[128], idx;
	format(tmp, sizeof tmp, "SELECT access FROM account WHERE name='%s' AND cookie='%s'", name, cookie);
	result = mysql_query(mariadb, tmp);
	cache_get_value_name(0, "access", access);
	cache_delete(result);
	tmp = strtok(params, idx);
	if(strcmp(tmp, "houseorc_load") == 0) {
		if(GetAccess(access, "gm_3") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strrest(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "文档名不能为空");
			return 1;
		}
		if(mk_strlen(tmp)<PHOUSE_TXTLEN) {
			if(IsTxtInstallation(tmp)==0) {
				UCPSendPM(name, "[PHouse]文档加载失败,因为他没被安装");
				return 1;
			}
			LoadTxt(GetTxtInstallationId(tmp), tmp);
		} else {
			UCPSendPM(name, "[PHouse]文档名字过长");
		}
		return 1;
	}
	if(strcmp(tmp, "changeservname") == 0) {
		if(GetAccess(access, "admin") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strrest(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "新名称不能为空");
			return 1;
		}
		if(strlen(tmp) > 128) {
			UCPSendPM(name, "服务器名称不能大于128字符");
			return 1;
		}
		new msg[256];
		format(msg, sizeof msg, "UPDATE config SET value='%s' keyword='servname'", tmp);
		mysql_query(mariadb, msg, false);
		SetServName(tmp);
		UCPSendPM(name, "服务器名称设置成功");
		return 1;
	} else if(strcmp(tmp, "RESTARTSERV") == 0) {
		if(GetAccess(access, "admin") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "秒数不能为空");
			return 1;
		}
		new sec = strval(tmp);
		if(sec < 3) {
			UCPSendPM(name, "秒数不能小于3");
			return 1;
		}
		tmp = strrest(params, idx);
		if(!strlen(tmp) || strlen(tmp) > 64) {
			UCPSendPM(name, "原因不能为空或者大于64字符");
			return 1;
		}
		format(tmp, sizeof tmp, "[系统] %d秒后重启服务器, 原因:%s", sec, tmp);
		SCMToAll(COLOR_RED, tmp);
		UCPSendChat(tmp);
		format(tmp, sizeof tmp, "%d秒后重启服务器, 原因:%s", sec, tmp);
		UCPSendPM(name, tmp);
		restartserv = sec;
		return 1;
	} else if(strcmp(tmp, "MUSICCARDS") == 0) {
		if(GetAccess(access, "admin") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		new pname[MAX_PLAYER_NAME];
		format(pname, sizeof pname, strtok(params, idx));
		if(accountIsNameExist(pname) == 0) {
			format(tmp, sizeof tmp, "玩家名字 %s 并不存在", pname);
			UCPSendPM(name, tmp);
			return 1;
		}
		tmp = strtok(params, idx);
		new cards = strval(tmp);
		format(tmp, sizeof tmp, "SELECT music_cards FROM account WHERE name='%s'", pname);
		result = mysql_query(mariadb, tmp);
		new music_cards;
		cache_get_value_name_int(0, "music_cards", music_cards);
		cache_delete(result);
		music_cards = music_cards + cards;
		format(tmp, sizeof tmp, "UPDATE account SET music_cards='%d' WHERE name='%s'", music_cards, pname);
		mysql_query(mariadb, tmp, false);
		format(tmp, sizeof tmp, "你给了 %s %d张点歌卡", pname, cards);
		UCPSendPM(name, tmp);
		return 1;
	} else if(strcmp(tmp, "CPWD") == 0) {
		if(GetAccess(access, "admin") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		new pname[MAX_PLAYER_NAME];
		format(pname, sizeof pname, strtok(params, idx));
		if(accountIsNameExist(pname) == 0) {
			format(tmp, sizeof tmp, "玩家名字 %s 并不存在", pname);
			UCPSendPM(name, tmp);
			return 1;
		}
		tmp = strtok(params, idx);
		if(strlen(tmp) < 3 || strlen(tmp) > 20) {
            UCPSendPM(name, "密码长度3-20");
            return 1;
        }
        if(charCheck(tmp) != 1) {
            UCPSendPM(name, "密码只能用 A-Z a-z 0-9 组合");
            return 1;
        }
		new pid = isNameOnline(pname);
		if(pid != -1) {
			UCPSendPM(name, "更改失败该玩家正在游戏中");
            return 1;
		}
		format(tmp, sizeof tmp, "UPDATE account SET password=MD5('%s') WHERE name='%s'", tmp, pname);
		mysql_query(mariadb, tmp, false);
		UCPSendPM(name, "密码更改成功");
		return 1;
	} else if(strcmp(tmp, "OLSSAVINGS") == 0) {
		if(GetAccess(access, "admin") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		new pname[MAX_PLAYER_NAME], amount;
		tmp = strtok(params, idx);
		format(pname, sizeof pname, tmp);
		tmp = strtok(params, idx);
		amount = strval(tmp);
		if(amount == 0) {
			UCPSendPM(name, "金额不能为0");
			return 1;
		}
		new pid = isNameOnline(pname);
		if(pid == -1) {
			if(accountIsNameExist(pname) == 0) {
				format(tmp, sizeof tmp, "玩家名字 %s 并不存在", pname);
				UCPSendPM(name, tmp);
				return 1;
			} else {
				format(tmp, sizeof tmp, "SELECT savings FROM account WHERE name='%s'", pname);
				result = mysql_query(mariadb, tmp);
				new savings;
				cache_get_value_name_int(0, "savings", savings);
				cache_delete(result);
				savings += amount;
				format(tmp, sizeof tmp, "UPDATE account SET savings='%d' WHERE name='%s'", savings, pname);
				mysql_query(mariadb, tmp, false);
			}
		} else {
			GiveSavings(pid, amount);
			accountSaveData(pid);
		}
		log_money(name, "UCP", 0, pname, "none", 0, amount, "(权限-余额)");
		format(tmp, sizeof tmp, "余额:你给了 %s $%d", pname, amount);
		UCPSendPM(name, tmp);
		return 1;
	} else if(strcmp(tmp, "OLSMONEY") == 0) {
		if(GetAccess(access, "admin") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		new pname[MAX_PLAYER_NAME], amount;
		tmp = strtok(params, idx);
		format(pname, sizeof pname, tmp);
		tmp = strtok(params, idx);
		amount = strval(tmp);
		new pid = isNameOnline(pname);
		if(pid == -1) {
			if(accountIsNameExist(pname) == 0) {
				format(tmp, sizeof tmp, "玩家名字 %s 并不存在", pname);
				UCPSendPM(name, tmp);
				return 1;
			} else {
				format(tmp, sizeof tmp, "SELECT money FROM account WHERE name='%s'", pname);
				result = mysql_query(mariadb, tmp);
				new money;
				cache_get_value_name_int(0, "money", money);
				cache_delete(result);
				money += amount;
				format(tmp, sizeof tmp, "UPDATE account SET money='%d' WHERE name='%s'", money, pname);
				mysql_query(mariadb, tmp, false);
			}
		} else {
			GiveMoney(pid, amount);
		}
		log_money(name, "UCP", 0, pname, "none", 0, amount, "(权限-金钱)");
		format(tmp, sizeof tmp, "金钱:你给了 %s $%d", pname, amount);
		UCPSendPM(name, tmp);
		return 1;
	} else if(strcmp(tmp, "giveallmoney") == 0) {
		if(GetAccess(access, "gm_4") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "金额不能为空");
			return 1;
		}
		new money = strval(tmp);
		tmp = strrest(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "原因不能为空");
			return 1;
		}
		if(strlen(tmp) > 64) {
			UCPSendPM(name, "原因不能大于64字符");
			return 1;
		}
		new playerpp = 0;
		for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
			if(IsPlayerConnected(i) == 1 && GetPlayerStatus(i) == 1) {
				playerpp ++;
				GiveMoney(i, money);
			}
		}
		if(playerpp == 0) {
			UCPSendPM(name, "当前服务器没人呢");
			return 1;
		}
		new msg[144];
		format(msg, sizeof msg, "[系统] 管理员 %s 给了所有人 $%d, 原因:%s", name, money, tmp);
		SCMToAll(COLOR_RED, msg);
		UCPSendChat(msg);
		format(msg, sizeof msg, "(权限-现金-所有人) %s", tmp);
		log_money(name, "UCP", 0, "none", "none", 0, money, msg);
		format(msg, sizeof msg, "已以理由:%s 给了所有人 $%d", tmp, money);
		UCPSendPM(name, msg);
		return 1;
	} else if(strcmp(tmp, "bannnnnnn") == 0) {
		if(GetAccess(access, "gm_3") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		UCPCMDBan(name, params);
		return 1;
	} else if(strcmp(tmp, "givemoney") == 0) {
		if(GetAccess(access, "gm_3") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "玩家名字不能为空");
			return 1;
		}
		new pid = isNameOnline(tmp);
		if(pid == -1) {
			UCPSendPM(name, "错误的玩家名字");
			return 1;
		}
		if(GetPlayerStatus(pid) == 0) {
			UCPSendPM(name, "对方未登录");
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "金额不能为空");
			return 1;
		}
		new money = strval(tmp);
		tmp = strrest(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "原因不能为空");
			return 1;
		}
		if(strlen(tmp) > 64) {
			UCPSendPM(name, "原因不能大于64字符");
			return 1;
		}
		new msg[144];
		format(msg, sizeof msg, "[系统] 管理员 %s 给了 %s $%d, 原因:%s", name, GetName(pid), money, tmp);
		SCMToAll(COLOR_RED, msg);
		UCPSendChat(msg);
		GiveMoney(pid, money);
		format(msg, sizeof msg, "(权限-现金) %s", tmp);
		log_money(name, "UCP", 0, GetName(pid), GetIp(pid), GetPlayerUID(pid), money, msg);
		format(msg, sizeof msg, "已以理由:%s 给了 %s $%d", tmp, GetName(pid), money);
		UCPSendPM(name, msg, "s");
		return 1;
	} else if(strcmp(tmp, "disableveh") == 0) {
		if(GetAccess(access, "gm_2") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "玩家名字不能为空");
			return 1;
		}
		new pid = isNameOnline(tmp);
		if(pid == -1) {
			UCPSendPM(name, "错误的玩家名字");
			return 1;
		}
		new msg[144];
		if(userInfo[pid][NoVeh] == 0) {
			format(msg, sizeof msg, "[系统] 管理员 %s 禁止了 %s 使用载具.", name, GetName(pid));
			format(tmp, sizeof tmp, "已禁止玩家 %s 使用载具", GetName(pid));
		} else {
			format(msg, sizeof msg, "[系统] 管理员 %s 解除了 %s 的禁止使用载具.", name, GetName(pid));
			format(tmp, sizeof tmp, "已允许玩家 %s 使用载具", GetName(pid));
		}
		accountSetNoVEH(pid);
		SCMToAll(COLOR_RED, msg);
		UCPSendChat(msg);
		UCPSendPM(name, tmp);
		return 1;
	} else if(strcmp(tmp, "mute") == 0) {
		if(GetAccess(access, "gm_2") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "玩家名字不能为空");
			return 1;
		}
		new pid = isNameOnline(tmp);
		if(pid == -1) {
			UCPSendPM(name, "错误的玩家名字");
			return 1;
		}
		if(GetPlayerStatus(pid) == 0) {
			UCPSendPM(name, "对方未登录");
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "时间不能为空");
			return 1;
		}
		new time = strval(tmp);
		if(time < 1) {
			UCPSendPM(name, "时间不能小于1秒");
			return 1;
		}
		tmp = strrest(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "原因不能为空");
			return 1;
		}
		if(strlen(tmp) > 64) {
			UCPSendPM(name, "原因不能大于64字符");
			return 1;
		}
		log_mute(name, "UCP", 0, GetName(pid), GetIp(pid), GetPlayerUID(pid), time, tmp);
		new msg[144];
		format(msg, sizeof msg, "[系统] 管理员 %s 禁言了 %s %d秒, 原因:%s", name, GetName(pid), time, tmp);
		UCPSendChat(msg);
		SCMToAll(COLOR_RED, msg);
		MutePlayer(pid, time);
		format(msg, sizeof msg, "已以理由:%s 禁言了玩家 %s %d秒", tmp, GetName(pid), time);
		UCPSendPM(name, msg, "s");
		return 1;
	} else if(strcmp(tmp, "jail") == 0) {
		if(GetAccess(access, "gm_2") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "玩家名字不能为空");
			return 1;
		}
		new pid = isNameOnline(tmp);
		if(pid == -1) {
			UCPSendPM(name, "错误的玩家名字");
			return 1;
		}
		if(GetPlayerStatus(pid) == 0) {
			UCPSendPM(name, "该玩家未登录");
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "监狱时间不能为空");
			return 1;
		}
		new time = strval(tmp);
		if(time < 1) {
			UCPSendPM(name, "监狱时间不能小于1秒");
			return 1;
		}
		tmp = strrest(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "原因不能为空");
			return 1;
		}
		if(strlen(tmp) > 64) {
			UCPSendPM(name, "原因不能大于64字符");
			return 1;
		}
		log_jail(name, "UCP", 0, GetName(pid), GetIp(pid), GetPlayerUID(pid), time, tmp);
		new msg[144];
		format(msg, sizeof msg, "[系统] 管理员 %s 把 %s 关进了监狱 %d秒, 原因:%s", name, GetName(pid), time, tmp);
		SCMToAll(COLOR_RED, msg);
		UCPSendChat(msg);
		throwInJail(pid, time);
		format(msg, sizeof msg, "已以理由:%s 把玩家 %s 关进了监狱 %d秒", tmp, GetName(pid), time);
		UCPSendPM(name, msg, "s");
		return 1;
	} else if(strcmp(tmp, "bwbaarbn") == 0) {
		if(GetAccess(access, "gm_1") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "玩家名字不能为空");
			return 1;
		}
		new pid = isNameOnline(tmp);
		if(pid == -1) {
			UCPSendPM(name, "错误的玩家名字");
			return 1;
		}
		if(GetPlayerStatus(pid) == 0) {
			UCPSendPM(name, "对方未登录");
			return 1;
		}
		if(userInfo[pid][Warn] >= 3) {
			format(tmp, sizeof tmp, "[系统] 管理员 %s 提醒玩家 %s 注意游戏规则, 警告次数:%d", name, GetName(pid), userInfo[pid][Warn]);
			SCM(pid, COLOR_RED, "[系统] 屡次警告无效, 你被关进了禁闭室10分钟");
			userInfo[pid][Warn] = 0;
			throwInJail(pid);
		} else {
			userInfo[pid][Warn] ++;
			format(tmp, sizeof tmp, "[系统] 管理员 %s 提醒玩家 %s 注意游戏规则, 警告次数:%d", name, GetName(pid), userInfo[pid][Warn]);
			new Float:x, Float:y, Float:z;
			GetPlayerPos(pid, x, y, z);
			SetPlayerPos(pid, x, y, z + 3);
		}
		SCMToAll(COLOR_RED, tmp);
		UCPSendChat(tmp);
		format(tmp, sizeof tmp, "已提醒玩家 %s 警告次数:%d", GetName(pid), userInfo[pid][Warn]);
		UCPSendPM(name, tmp, "s");
		return 1;
	} else if(strcmp(tmp, "kick") == 0) {
		if(GetAccess(access, "gm_1") == 0) {
			UCPSendPM(name, UCP_GM_ACCESS_DENIED);
			return 1;
		}
		tmp = strtok(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "玩家名字不能为空");
			return 1;
		}
		new pid = isNameOnline(tmp);
		if(pid == -1) {
			UCPSendPM(name, "错误的玩家名字");
			return 1;
		}
		tmp = strrest(params, idx);
		if(!strlen(tmp)) {
			UCPSendPM(name, "原因不能为空");
			return 1;
		}
		if(strlen(tmp) > 64) {
			UCPSendPM(name, "原因不能大于64字符");
			return 1;
		}
		log_kick(name, "UCP", 0, GetName(pid), GetIp(pid), GetPlayerUID(pid), tmp);
		new msg[144];
		format(msg, sizeof msg, "[系统] 管理员 %s 把 %s 踢出了服务器, 原因:%s", name, GetName(pid), tmp);
		SCMToAll(COLOR_RED, msg);
		KickEx(pid, "被管理员踢出");
		UCPSendChat(msg);
		format(msg, sizeof msg, "已以理由: %s 把玩家 %s 踢出了服务器", tmp, GetName(pid));
		UCPSendPM(name, msg, "s");
		return 1;
	}
	return 1;
}

stock UCPCMDBan(name[], params[]) {
	new tmp[128], idx;
	tmp = strtok(params, idx);
	tmp = strtok(params, idx);
	if(!strlen(tmp)) {
		UCPSendPM(name, "玩家名字不能为空");
		return 1;
	}
	new Cache:result, ojbk, query[128];
	format(query, sizeof query, "SELECT COUNT(name) AS ojbk FROM account WHERE name='%s'", tmp);
	result = mysql_query(mariadb, query);
	cache_get_value_name_int(0, "ojbk", ojbk);
	cache_delete(result);
	if(ojbk == 0) {
		UCPSendPM(name, "错误的玩家名字");
		return 1;
	}
	new pname[MAX_PLAYER_NAME];
	format(pname, sizeof pname, tmp);
	tmp = strtok(params, idx);
	if(!strlen(tmp)) {
		UCPSendPM(name, "时间不能为空");
		return 1;
	}
	if(strfind(tmp, "STARTEDP") != -1 && strfind(tmp, "ENDP") != -1) {
		strdel(tmp, 0, 8);//STARTEDP
		strdel(tmp, strlen(tmp)-4, strlen(tmp));//ENDP
	}
	new bantime = strval(tmp), hod = -1;
	if(bantime < -1) {
		UCPSendPM(name, "错误的时间");
		return 1;
	} else if(bantime == -1) {
		hod = 2;
	} else if(bantime == 0) {
		hod = 3;
	} else {
		hod = 1;
	}
	new type[128];
	type = strtok(params, idx);
	if(!strlen(type)) {
		UCPSendPM(name, "类型不能为空");
		return 1;
	}
	if(strcmp(type, "d") != 0 && strcmp(type, "h") != 0) {
		UCPSendPM(name, "类型只能为d h");
		return 1;
	}
	tmp = strrest(params, idx);
	if(!strlen(tmp)) {
		UCPSendPM(name, "原因不能为空");
		return 1;
	}
	if(strlen(tmp) > 64) {
		UCPSendPM(name, "原因不能大于64字符");
		return 1;
	}
	new msg[144], seconds;
	if(hod == 1) {
		if(strcmp(type, "d") == 0) {
			seconds = bantime * 86400;
			format(msg, sizeof msg, "[系统] 管理员 %s 封杀了 %s 期限:%d天, 原因:%s", name, pname, bantime, tmp);
		} else if(strcmp(type, "h") == 0) {
			seconds = bantime * 3600;
			format(msg, sizeof msg, "[系统] 管理员 %s 封杀了 %s 期限:%d小时, 原因:%s", name, pname, bantime, tmp);
		}
	} else if(hod == 2) {
		seconds = -1;
		format(msg, sizeof msg, "[系统] 管理员 %s 封杀了 %s 期限:永久, 原因:%s", name, pname, tmp);
	} else if(hod == 3) {
		seconds = 0;
		format(msg, sizeof msg, "[系统] 管理员 %s 解封了 %s 原因:%s", name, pname, tmp);
	}
	SCMToAll(COLOR_RED, msg);
	UCPSendChat(msg);
	format(msg, sizeof msg, "(ban-%s) %s", type, tmp);
	log_ban(name, "UCP", 0, pname, "none", 0, seconds, msg);
	if(seconds != 0) {
		new pid = isNameOnline(pname);
		if(pid != -1) {
			KickEx(pid, "被封号");
		}
	}
	if(seconds == -1) {
		format(msg, sizeof msg, "UPDATE account SET bantime='%d',ban_reason='%s' WHERE name='%s'", seconds, tmp, pname);
	} else {
		format(msg, sizeof msg, "UPDATE account SET bantime='%d',ban_reason='%s' WHERE name='%s'", gettime() + seconds, tmp, pname);
	}
    mysql_query(mariadb, msg, false);
	if(hod == 3) {
		format(msg, sizeof msg, "已以理由:%s 把玩家 %s 解封了", tmp, pname);
	} else if(hod == 2) {
		format(msg, sizeof msg, "已以理由:%s 把玩家 %s 永久封杀了", tmp, pname);
	} else {
		format(msg, sizeof msg, "已以理由:%s 把玩家 %s 封杀了 %d秒", tmp, pname, seconds);
	}
	UCPSendPM(name, msg);
	return 1;
}