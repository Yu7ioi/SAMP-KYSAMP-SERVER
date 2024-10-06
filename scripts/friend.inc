#define MAX_FRIEND_ADD_REQUEST      (30)
#define FRIEND_DIALOG_ID            (400)
#define FRIEND_DIALOG_MAIN          FRIEND_DIALOG_ID + 1
#define FRIEND_DIALOG_REQUEST       FRIEND_DIALOG_ID + 2
#define FRIEND_DIALOG_ADD           FRIEND_DIALOG_ID + 3
#define FRIEND_DIALOG_LIST          FRIEND_DIALOG_ID + 4
#define FRIEND_DIALOG_OPTION        FRIEND_DIALOG_ID + 5
#define FRIEND_TRANSFER_DIALOGID    (203)

new pF_Request[MAX_PLAYERS][MAX_FRIEND_ADD_REQUEST][MAX_PLAYER_NAME];
new pF_Page[MAX_PLAYERS], pF_SName[MAX_PLAYERS][MAX_PLAYER_NAME];

stock FRIEND_changeNtoN(oldname[], newname[]) {
    new msg[256];
    format(msg, sizeof msg, "UPDATE friend SET mname='%s' WHERE mname='%s'", newname, oldname);
    mysql_query(mariadb, msg, false);
    format(msg, sizeof msg, "UPDATE friend SET fname='%s' WHERE fname='%s'", newname, oldname);
    mysql_query(mariadb, msg, false);
}

stock FRIEND_Connect(playerid) {
    new msg[128], Cache:result, friends, myname[MAX_PLAYER_NAME];
	format(myname, sizeof myname, "%s", GetName(playerid));
	format(msg, sizeof msg, "SELECT * FROM friend WHERE mname = '%s'", myname);
	result = mysql_query(mariadb, msg);
	cache_get_row_count(friends);
    if(friends == 0) {
        //SCM(playerid, COLOR_RED, "[好友系统]您还没有好友");
    } else {
        new fname[MAX_PLAYER_NAME];
		for(new fc = 0; fc < friends; fc ++) {
			cache_get_value_name(fc, "fname", fname);
            for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
                if(IsPlayerConnected(i) == 1 && i != playerid) {
                    if(strcmp(GetName(i), fname) == 0) {
						format(msg, sizeof msg, "[好友系统]您的好友 %s(%d) 上线了!", myname, i);
                        SCM(i, COLOR_YELLOW, msg);
                    }
                }
            }
		}
	}
    cache_delete(result);
    for(new i = 0; i < MAX_FRIEND_ADD_REQUEST; i ++) {
        format(pF_Request[playerid][i], 2, "");
    }
}

stock showFriendDailogMain(playerid, err[] = " ") {
    new title[128], msg[128];
    format(title, sizeof title, "{FF0000}%s", err);
    format(msg, sizeof msg, "我的好友\n添加好友\n好友请求");
    ShowPlayerDialog(playerid, FRIEND_DIALOG_MAIN, DIALOG_STYLE_LIST, title, msg, "选择", "关闭");
}

stock showFriendDailogRequest(playerid, err[] = " ") {
    new title[128], msg[1024], count = 0;
    format(title, sizeof title, "{FF0000}%s", err);
    for(new i = 0; i < MAX_FRIEND_ADD_REQUEST; i ++) {
        if(strlen(pF_Request[playerid][i]) > 2) {
            format(msg, sizeof msg, "%s%s 想成为你的好友\n", msg, pF_Request[playerid][i]);
            count ++;
        }
    }
    if(count == 0) {
        format(msg, sizeof msg, "暂无好友验证消息!");
    }
    ShowPlayerDialog(playerid, FRIEND_DIALOG_REQUEST, DIALOG_STYLE_LIST, title, msg, "同意", "返回");
}

stock showFriendDailogAdd(playerid, err[] = " ") {
    new title[128];
    format(title, sizeof title, "{FF0000}%s", err);
    ShowPlayerDialog(playerid, FRIEND_DIALOG_ADD, DIALOG_STYLE_INPUT, title, "请输入对方的ID", "确定", "返回");
}

stock showFriendDailogList(playerid, page = 1) {
    new msg[512], Cache:result, friends;
	format(msg, sizeof msg, "SELECT * FROM friend WHERE mname = '%s'", GetName(playerid));
	result = mysql_query(mariadb, msg);
	cache_get_row_count(friends);
    if(friends == 0) {
        SCM(playerid, COLOR_WHITE, "[好友系统]您还没有好友.");
    } else {
		format(msg, sizeof msg, "");
        if(page != 1) {
            format(msg, sizeof msg, "上一页\n");
        }
        new pagemax = 20;
        new maxpage = GetMaxPage(friends);
		for(new nowid = 0; nowid < friends; nowid ++) {
            if(nowid >= (page - 1) * pagemax && nowid < (pagemax * page)) {
                new fname[MAX_PLAYER_NAME], online = -1;
                cache_get_value_name(nowid, "fname", fname);
                for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
					if(IsPlayerConnected(i) == 1 && strcmp(GetName(i), fname) == 0) {
						online = i;
						break;
                    }
                }
				if(online == -1) {
					format(msg, sizeof msg, "%s{CFCFCF}%s\n", msg, fname);
				} else {
					format(msg, sizeof msg, "%s{00FF00}%s\n", msg, fname);
				}
            }
		}
        if(page != maxpage && maxpage != 1) {
            format(msg, sizeof msg, "%s\n下一页", msg);
        }
        ShowPlayerDialog(playerid, FRIEND_DIALOG_LIST, DIALOG_STYLE_LIST, "我的好友", msg, "选择", "返回");
	}
    cache_delete(result);
}

stock showFriendDailogOption(playerid, name[], err[] = " ") {
    new title[128], msg[128];
    format(title, sizeof title, "{FF0000}%s", err);
    format(msg, sizeof msg, "名字:[%s]\n转账\n传送\n{FF0000}删除好友", name);
    ShowPlayerDialog(playerid, FRIEND_DIALOG_OPTION, DIALOG_STYLE_LIST, title, msg, "确定", "返回");
}

forward friendODR(playerid, dialogid, response, listitem, inputtext[]);
public friendODR(playerid, dialogid, response, listitem, inputtext[]) {
    if(dialogid == FRIEND_DIALOG_MAIN) {
        if(!response) {
            return 1;
        }
        if(listitem == 0) {
            pF_Page[playerid] = 1;
            showFriendDailogList(playerid, pF_Page[playerid]);
        } else if(listitem == 1) {
            showFriendDailogAdd(playerid);
        } else if(listitem == 2) {
            showFriendDailogRequest(playerid);
        }
        return 1;
    }
    if(dialogid == FRIEND_DIALOG_OPTION) {
        if(!response) {
            showFriendDailogList(playerid, pF_Page[playerid]);
            return 1;
        }
        new pid = isNameOnline(pF_SName[playerid]);
        if(strcmp(inputtext, "转账") == 0) {
            new msg[128];
            if(pid == -1) {
				format(msg, sizeof msg, "[好友系统] %s 不在游戏.", pF_SName[playerid]);
                SCM(playerid, COLOR_RED, msg);
                return 1;
            }
			format(msg, sizeof msg, "%d", pid);
			CallRemoteFunction("bankODR", "iiiis", playerid, FRIEND_TRANSFER_DIALOGID, 1, -1, msg);
        } else if(strcmp(inputtext, "传送") == 0) {
			new str[128];
            if(pid == -1) {
				format(str, sizeof str, "[好友系统] %s 不在游戏.", pF_SName[playerid]);
                SCM(playerid, COLOR_RED, str);
                return 1;
            }
            new Float:x, Float:y, Float:z;
            GetPlayerPos(pid, x, y, z);
            SetPlayerPos(playerid, x +1, y + 1, z + 1);
			format(str, sizeof str, "[好友系统] 你的好友 %s 传送到了你的身边.", GetName(playerid));
			SCM(pid, COLOR_YELLOW, str);
			format(str, sizeof str, "[好友系统] 你传送到了 %s 的身边.", pF_SName[playerid]);
			SCM(playerid, COLOR_YELLOW, str);
        } else if(strcmp(inputtext, "删除好友") == 0) {
            new msg[128];
            format(msg, sizeof msg, "DELETE FROM friend WHERE mname = '%s' AND fname = '%s'", GetName(playerid), pF_SName[playerid]);
    		mysql_query(mariadb, msg, false);
            format(msg, sizeof msg, "DELETE FROM friend WHERE mname = '%s' AND fname = '%s'", pF_SName[playerid], GetName(playerid));
    		mysql_query(mariadb, msg, false);
            if(pid != -1) {
				format(msg, sizeof msg, "[好友系统] %s 删除了你的好友", GetName(playerid));
				SCM(pid, COLOR_WHITE, msg);
            }
			format(msg, sizeof msg, "[好友系统] 你删除了 %s 好友", pF_SName[playerid]);
			SCM(playerid, COLOR_YELLOW, msg);
        } else {
            showFriendDailogOption(playerid, pF_SName[playerid]);
        }
        return 1;
    }
    if(dialogid == FRIEND_DIALOG_LIST) {
        if(!response) {
            showFriendDailogMain(playerid);
            return 1;
        }
        if(strcmp(inputtext, "上一页") == 0) {
            pF_Page[playerid] --;
            showFriendDailogList(playerid, pF_Page[playerid]);
            return 1;
        }
        if(strcmp(inputtext, "下一页") == 0) {
            pF_Page[playerid] ++;
            showFriendDailogList(playerid, pF_Page[playerid]);
            return 1;
        }
        format(pF_SName[playerid], MAX_PLAYER_NAME, inputtext);
        showFriendDailogOption(playerid, inputtext);
        return 1;
    }
    if(dialogid == FRIEND_DIALOG_ADD) {
        if(!response) {
            showFriendDailogMain(playerid);
            return 1;
        }
        new pid = strval(inputtext);
        if(IsPlayerConnected(pid) == 0 || pid == playerid) {
            showFriendDailogAdd(playerid, "错误的对方ID,请输入PID");
            return 1;
        }
        new nullid = -1;
        for(new i = 0; i < MAX_FRIEND_ADD_REQUEST; i ++) {
            if(strlen(pF_Request[pid][i]) < 2) {
                nullid = i;
                break;
            }
        }
        if(nullid == -1) {
            showFriendDailogMain(playerid, "(对方好友请求过多)");
            return 1;
        }
        new msg[128], Cache:result, exist;
    	format(msg, sizeof msg, "SELECT COUNT(fname) AS exist FROM friend WHERE mname = '%s' AND fname = '%s'", GetName(playerid), GetName(pid));
        result = mysql_query(mariadb, msg);
    	cache_get_value_name_int(0, "exist", exist);
        cache_delete(result);
        if(exist != 0) {
            showFriendDailogAdd(playerid, "[好友系统]你们已经是好友了");
            return 1;
        }
        for(new i = MAX_FRIEND_ADD_REQUEST-1; i > 0; i --) {
            format(pF_Request[pid][i], MAX_PLAYER_NAME, "%s", pF_Request[pid][i-1]);
        }
        format(pF_Request[pid][0], MAX_PLAYER_NAME, "%s", GetName(playerid));
		format(msg, sizeof msg, "[好友系统]好友请求已发送给 %s, 请等待ta的回复.", GetName(pid));
		SCM(playerid, COLOR_WHITE, msg);
        SCM(pid, COLOR_WHITE, "[好友系统]您收到了一个好友请求，请输入/hy 进入好友请求 查看.");
        return 1;
    }
    if(dialogid == FRIEND_DIALOG_REQUEST) {
        if(!response) {
            showFriendDailogMain(playerid);
            return 1;
        }
        if(strlen(pF_Request[playerid][listitem]) < 2) {
            showFriendDailogMain(playerid, "没有想跟你交朋友!!");
            return 1;
        }
        new pid = -1;
        for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
            if(IsPlayerConnected(i) == 1) {
                if(strcmp(GetName(i), pF_Request[playerid][listitem]) == 0) {
                    pid = i;
                    break;
                }
            }
        }
        new msg[144];
        if(pid == -1) {
            format(msg, sizeof msg, "%s 突然又不想和你做朋友了!(对方已离线)", pF_Request[playerid][listitem]);
            format(pF_Request[playerid][listitem], 2, "");
            showFriendDailogMain(playerid, msg);
            return 1;
        }
        format(pF_Request[playerid][listitem], 2, "");
        format(msg, sizeof msg, "INSERT INTO friend (mname,fname) VALUES ('%s','%s')", GetName(pid), GetName(playerid));
		mysql_query(mariadb, msg, false);
		format(msg, sizeof msg, "[好友系统] %s 已同意你成为他的好友.", GetName(playerid));
		SCM(pid, COLOR_WHITE, msg);
        format(msg, sizeof msg, "INSERT INTO friend (mname,fname) VALUES ('%s','%s')", GetName(playerid), GetName(pid));
		mysql_query(mariadb, msg, false);
		format(msg, sizeof msg, "[好友系统] 你已同意 %s 成为你的好友.", GetName(pid));
		SCM(playerid, COLOR_WHITE, msg);
        return 1;
    }
    return 1;
}

CMD:hy(playerid, params[]) {
    showFriendDailogMain(playerid);
    return 1;
}
