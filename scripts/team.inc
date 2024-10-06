#define DIALOG_TEAM                 (500)
#define DIALOG_TEAM_WODE            DIALOG_TEAM + 1
#define DIALOG_TEAM_MO_KICK         DIALOG_TEAM + 2//MO = member option
#define DIALOG_TEAM_MO_LEVEL        DIALOG_TEAM + 3
#define DIALOG_TEAM_LIST            DIALOG_TEAM + 4
#define DIALOG_TEAM_WAITTOPASS      DIALOG_TEAM + 5

enum teamstruct {
    TID,
    Label[MAX_PLAYER_NAME],
    Level,
    Page,
    OptionName[MAX_PLAYER_NAME]
};

new teamInfo[MAX_PLAYERS][teamstruct];

stock team_changeNtoN(oldname[], newname[], playerid) {
	if(GetPlayerTeamID(playerid) != 0) {
		new teamname[MAX_PLAYER_NAME], msg[128], oldteamname[MAX_PLAYER_NAME];
		format(teamname, sizeof teamname, isNameHasTeamName(newname));
		format(oldteamname, sizeof oldteamname, isNameHasTeamName(oldname));
		if(strcmp(teamname, oldteamname) == 0) {
			format(msg, sizeof msg, "UPDATE team_members SET name='%s' WHERE name='%s'", newname, oldname);
		} else {
			format(msg, sizeof msg, "DELETE FROM team_members WHERE name='%s'", oldname);
		}
		mysql_query(mariadb, msg, false);
    }
}

stock GetPlayerTeamID(playerid) {
    return teamInfo[playerid][TID];
}

stock GetTeamNameFromTID(tid) {
    new msg[128], Cache:result, teamname[MAX_PLAYER_NAME];
    format(msg, sizeof msg, "SELECT name FROM team_list WHERE id='%d'", tid);
    result = mysql_query(mariadb, msg);
    cache_get_value_name(0, "name", teamname);
    cache_delete(result);
    return teamname;
}

stock teamWode(playerid, page) {
    new title[88], msg[2048], Cache:result, teamname[MAX_PLAYER_NAME], teamleader[MAX_PLAYER_NAME], membercount, waittopass;
    format(msg, sizeof msg, "SELECT * FROM team_list WHERE id='%d'", teamInfo[playerid][TID]);
    result = mysql_query(mariadb, msg);
    cache_get_value_name(0, "name", teamname);

    format(msg, sizeof msg, "SELECT name FROM team_members WHERE tid='%d' AND level='%d'", teamInfo[playerid][TID], 3);
    result = mysql_query(mariadb, msg);
    cache_get_value_name(0, "name", teamleader);

    format(msg, sizeof msg, "SELECT COUNT(name) AS waittopass FROM team_applications WHERE tname='%s'", teamname);
    result = mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "waittopass", waittopass);
    format(msg, sizeof msg, "SELECT * FROM team_members WHERE tid='%d'", teamInfo[playerid][TID]);
    result = mysql_query(mariadb, msg);
    cache_get_row_count(membercount);

    format(msg, sizeof msg, "\
    %s名称:%s\n\
    领导人:%s\n\
    成员数量:[%d]\n\
    等待审核:[%d]\n\
    --------------------成员列表--------------------\n",
    teamInfo[playerid][Label], teamname, teamleader, membercount, waittopass);
    if(page != 1) {
        format(msg, sizeof msg, "%s上一页\n", msg);
    }
    new pagemax = 20, maxpage = GetMaxPage(membercount);
    for(new nowid = 0; nowid < membercount; nowid ++) {
        if(nowid >= (page - 1) * pagemax && nowid < (pagemax * page)) {
            new name[MAX_PLAYER_NAME], level;
            cache_get_value_name(nowid, "name", name);
            cache_get_value_name_int(nowid, "level", level);
            format(msg, sizeof msg, "%s{%s}%s (Lv.%d)\n", msg, (isNameOnline(name) == -1 ? ("CFCFCF") : ("00FF00")), name, level);
        }
    }
    if(page != maxpage && maxpage != 1) {
        format(msg, sizeof msg, "%s下一页", msg);
    }
    format(title, sizeof title, "%s - %s", teamInfo[playerid][Label], teamname);
    ShowPlayerDialog(playerid, DIALOG_TEAM_WODE, DIALOG_STYLE_LIST, title, msg, "选择", "关闭");
    cache_delete(result);
}

stock teamList(playerid, page) {
    new msg[2048], Cache:result, teamcount;
    format(msg, sizeof msg, "SELECT *,(SELECT COUNT(tid) FROM team_members WHERE tid=team_list.id) as members FROM team_list;");
    result = mysql_query(mariadb, msg);
    cache_get_row_count(teamcount);
    if(teamcount == 0) {
        DIALOG_node(playerid, "当前没有任何已注册团队");
    } else {
        format(msg, sizeof msg, "名称\t标签\t人数\n");
        if(page != 1) {
            format(msg, sizeof msg, "%s上一页\t \t \n", msg);
        }
        new pagemax = 20, maxpage = GetMaxPage(teamcount);
        for(new nowid = 0; nowid < teamcount; nowid ++) {
            if(nowid >= (page - 1) * pagemax && nowid < (pagemax * page)) {
                new tid, name[MAX_PLAYER_NAME], label[MAX_PLAYER_NAME], members;
                cache_get_value_name_int(nowid, "id", tid);
                cache_get_value_name(nowid, "name", name);
                cache_get_value_name(nowid, "label", label);
                cache_get_value_name_int(nowid, "members", members);
                format(msg, sizeof msg, "%s%s\t%s\t%d\n", msg, name, label, members);
            }
        }
        if(page != maxpage && maxpage != 1) {
            format(msg, sizeof msg, "%s下一页\t \t ", msg);
        }
        ShowPlayerDialog(playerid, DIALOG_TEAM_LIST, DIALOG_STYLE_TABLIST_HEADERS, "点击可申请加入", msg, "选择", "关闭");
    }
    cache_delete(result);
}

stock teamWaitToPass(playerid, err[] = " ") {
    new title[88], msg[2048], waittopass, teamname[MAX_PLAYER_NAME], Cache:result;
    format(teamname, sizeof teamname, GetTeamNameFromTID(teamInfo[playerid][TID]));
    format(msg, sizeof msg, "SELECT * FROM team_applications WHERE tname='%s'", teamname);
    result = mysql_query(mariadb, msg);
    cache_get_row_count(waittopass);
    if(waittopass == 0) {
        DIALOG_node(playerid, "当前没有任何申请");
    } else {
        format(title, sizeof title, "{FF0000}", err);
        format(msg, sizeof msg, "{FF0000}删除所有申请\n");
        for(new i = 0; i < waittopass; i ++) {
            new name[MAX_PLAYER_NAME];
            cache_get_value_name(i, "name", name);
            format(msg, sizeof msg, "%s%s\n", msg, name);
        }
        ShowPlayerDialog(playerid, DIALOG_TEAM_WAITTOPASS, DIALOG_STYLE_LIST, title, msg, "选择", "关闭");
    }
    cache_delete(result);
}

forward teamODR(playerid, dialogid, response, listitem, inputtext[]);
public teamODR(playerid, dialogid, response, listitem, inputtext[]) {
    if(dialogid == DIALOG_TEAM_WAITTOPASS) {
        if(!response) {
            return 1;
        }
        if(listitem == 0) {
            new msg[128], teamname[MAX_PLAYER_NAME];
            format(teamname, sizeof teamname, GetTeamNameFromTID(teamInfo[playerid][TID]));
            format(msg, sizeof msg, "DELETE FROM team_applications WHERE tname='%s'", teamname);
            mysql_query(mariadb, msg, false);
            format(msg, sizeof msg, "[系统] 已清空%s申请列表", teamInfo[playerid][Label]);
            SCM(playerid, COLOR_BLUE, msg);
        } else {
            new msg[128], Cache:result, exist;
            format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM team_members WHERE name='%s'", inputtext);
            result = mysql_query(mariadb, msg);
            cache_get_value_name_int(0, "exist", exist);
            if(exist != 0) {
                format(msg, sizeof msg, "[系统] %s 已加入其他团队.", inputtext);
                SCM(playerid, COLOR_BLUE, msg);
            } else {
                if(accountIsNameExist(inputtext) == 0) {
                    format(msg, sizeof msg, "[系统] %s 好像已经改名了...", inputtext);
                    SCM(playerid, COLOR_BLUE, msg);
                } else {
                    new teamname[MAX_PLAYER_NAME];
                    format(teamname, sizeof teamname, GetTeamNameFromTID(teamInfo[playerid][TID]));
                    format(msg, sizeof msg, "INSERT INTO team_members (tid,name,level) VALUES ('%d','%s','%d')", teamInfo[playerid][TID], inputtext, 1);
                    mysql_query(mariadb, msg, false);
                    format(msg, sizeof msg, "[%s] 你已同意 %s 加入%s.", teamInfo[playerid][Label], inputtext, teamInfo[playerid][Label]);
                    SCM(playerid, COLOR_BLUE, msg);
                    format(msg, sizeof msg, "DELETE FROM team_applications WHERE tname='%s' AND name='%s'", teamname, inputtext);
                    mysql_query(mariadb, msg, false);
                    new pid = isNameOnline(inputtext);
                    if(pid != -1) {
                        teamInit(pid);
                    }
                }
            }
            cache_delete(result);
        }
        return 1;
    }
    if(dialogid == DIALOG_TEAM_LIST) {
        if(!response) {
            return 1;
        }
        if(strcmp(inputtext, "上一页") == 0) {
            teamInfo[playerid][Page] --;
            teamList(playerid, teamInfo[playerid][Page]);
        } else if(strcmp(inputtext, "下一页") == 0) {
            teamInfo[playerid][Page] ++;
            teamList(playerid, teamInfo[playerid][Page]);
        } else {
            if(teamInfo[playerid][TID] != 0) {
                SCM(playerid, COLOR_WHITE, "[系统] 你已经加入了一个团队不可以再加入其他团队.");
                return 1;
            }
            new tname[MAX_PLAYER_NAME];
            format(tname, sizeof tname, isNameHasTeamName(GetName(playerid)));
            if(!strlen(tname) || strcmp(tname, inputtext) != 0) {
                SCM(playerid, COLOR_WHITE, "[系统] 你的名字必须带有团队名称才可加入");
                return 1;
            }
            new Cache:result, exist, msg[128];
            format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM team_list WHERE name='%s'", inputtext);
            mysql_query(mariadb, msg);
            cache_get_value_name_int(0, "exist", exist);
            cache_delete(result);
            if(exist == 0) {
                SCM(playerid, COLOR_WHITE, "[系统] 未知错误, 请联系后台程序员.");
                return 1;
            }
            format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM team_applications WHERE tname='%s' AND name='%s'", inputtext, GetName(playerid));
            mysql_query(mariadb, msg);
            cache_get_value_name_int(0, "exist", exist);
            cache_delete(result);
            if(exist != 0) {
                SCM(playerid, COLOR_WHITE, "[系统] 你已经申请过了, 请等待该团队的管理回复.");
                return 1;
            }
            format(msg, sizeof msg, "INSERT INTO team_applications (tname,name) VALUES ('%s','%s')", inputtext, GetName(playerid));
            mysql_query(mariadb, msg, false);
            format(msg, sizeof msg, "[系统] 已向[%s]申请加入, 请等待该团队的管理回复.", inputtext);
            SCM(playerid, COLOR_BLUE, msg);
        }
        return 1;
    }
    if(dialogid == DIALOG_TEAM_WODE) {
        if(!response) {
            return 1;
        }
        if(strcmp(inputtext, "上一页") == 0) {
            teamInfo[playerid][Page] --;
            teamWode(playerid, teamInfo[playerid][Page]);
        } else if(strcmp(inputtext, "下一页") == 0) {
            teamInfo[playerid][Page] ++;
            teamWode(playerid, teamInfo[playerid][Page]);
        } else {
            if(strfind(inputtext, "等待审核:[") != -1) {
                if(teamInfo[playerid][Level] < 2) {
                    DIALOG_node(playerid, "你没有权限查看");
                    return 1;
                }
                teamWaitToPass(playerid);
                return 1;
            }
            new msg[128];
            if(teamInfo[playerid][Level] < 2) {
                format(msg, sizeof msg, "[%s] 你不可以使用该操作.", teamInfo[playerid][Label]);
                SCM(playerid, COLOR_BLUE, msg);
                return 1;
            }
            new Cache:result, name[MAX_PLAYER_NAME], level;
            sscanf(inputtext, "s[24] ", name);
            format(msg, sizeof msg, "SELECT level FROM team_members WHERE tid='%d' AND name='%s'", teamInfo[playerid][TID], name);
            result = mysql_query(mariadb, msg);
            cache_get_value_name_int(0, "level", level);
            cache_delete(result);
            if(level == 0) {
                return 1;
            }
            format(teamInfo[playerid][OptionName], MAX_PLAYER_NAME, name);
            format(msg, sizeof msg, "\
            名称: %s\n\
            等级: %d\n\
            {FF0000}踢出%s", teamInfo[playerid][Label], level);
            ShowPlayerDialog(playerid, DIALOG_TEAM_MO_KICK, DIALOG_STYLE_LIST, name, msg, "选择", "返回");
        }
        return 1;
    }
    if(dialogid == DIALOG_TEAM_MO_KICK) {
        if(!response) {
            teamWode(playerid, teamInfo[playerid][Page]);
            return 1;
        }
        if(listitem == 1) {
            new tmp[128];
            if(teamInfo[playerid][Level] < 3) {
                format(tmp, sizeof tmp, "[%s] 只有领导人才可以更改成员等级.", teamInfo[playerid][Label]);
                SCM(playerid, COLOR_BLUE, tmp);
                return 1;
            }
            new idx;
            tmp = strtok(inputtext, idx);
            tmp = strtok(inputtext, idx);
            new level = strval(tmp);
            if(level == 1) {
                level = 2;
            } else {
                level = 1;
            }
            format(tmp, sizeof tmp, "UPDATE team_members SET level='%d' WHERE tid='%d' AND name='%s'", level, teamInfo[playerid][TID], teamInfo[playerid][OptionName]);
            mysql_query(mariadb, tmp, false);
            teamODR(playerid, DIALOG_TEAM_WODE, 1, -1, teamInfo[playerid][OptionName]);
        } else if(listitem == 2) {
            new msg[128];
            if(teamInfo[playerid][Level] < 2) {
                format(msg, sizeof msg, "[%s] 你的等级不可以使用该命令.", teamInfo[playerid][Label]);
                SCM(playerid, COLOR_BLUE, msg);
                return 1;
            }
            new Cache:result, level;
            format(msg, sizeof msg, "SELECT * FROM team_members WHERE tid='%d' AND name='%s'", teamInfo[playerid][TID], teamInfo[playerid][OptionName]);
            result = mysql_query(mariadb, msg);
            cache_get_value_name_int(0, "level", level);
            cache_delete(result);
            if(level >= teamInfo[playerid][Level]) {
                format(msg, sizeof msg, "[%s] 你不可以踢出和你同等级或大于你等级的人.", teamInfo[playerid][Label]);
                SCM(playerid, COLOR_BLUE, msg);
                return 1;
            }
            format(msg, sizeof msg, "DELETE FROM team_members WHERE tid='%d' AND name='%s'", teamInfo[playerid][TID], teamInfo[playerid][OptionName]);
            mysql_query(mariadb, msg, false);
            new pid = isNameOnline(teamInfo[playerid][OptionName]);
            if(pid != -1) {
                format(msg, sizeof msg, "[%s] %s 把你踢出了%s", teamInfo[playerid][Label], GetName(playerid), teamInfo[playerid][Label]);
                SCM(pid, COLOR_BLUE, msg);
            }
            format(msg, sizeof msg, "[%s] 你把 %s 踢出了%s.", teamInfo[playerid][Label], teamInfo[playerid][OptionName], teamInfo[playerid][Label]);
            SCM(playerid, COLOR_BLUE, msg);
        }
        return 1;
    }
    return 1;
}

CMD:t(playerid, params[]) {
    if(strcmp(params, "list", true) == 0) {
        teamInfo[playerid][Page] = 1;
        teamList(playerid, teamInfo[playerid][Page]);
        return 1;
    }
    if(GetPlayerTeamID(playerid) == 0) {
        SCM(playerid, COLOR_WHITE, "[系统] 你并不属于任何团队, 所以你不能使用该命令");
        return 1;
    }
    new idx, tmp[128];
    tmp = strtok(params, idx);
    if(strcmp(tmp, "help", true) == 0) {
        format(tmp, sizeof tmp, "/t wode 我的%s, /t list 所有团队列表, /t goto 传送到成员, /t get 拉成员到身边(Lv.2), /t [内容] 帮派内聊天", teamInfo[playerid][Label]);
        DIALOG_node(playerid, tmp);
        return 1;
    }
    if(strcmp(tmp, "wode", true) == 0) {
        teamInfo[playerid][Page] = 1;
        teamWode(playerid, teamInfo[playerid][Page]);
        return 1;
    }
    if(strcmp(tmp, "goto", true) == 0) {
        tmp = strtok(params, idx);
        if(!strlen(tmp)) {
            format(tmp, sizeof tmp, "[%s] /t goto [成员ID] 可以传送到成员身边", teamInfo[playerid][Label]);
            SCM(playerid, COLOR_BLUE, tmp);
            return 1;
        }
        new pid = strval(tmp);
        if(pid < 0 || pid > MAX_PLAYERS || IsPlayerConnected(pid) == 0 || teamInfo[pid][TID] != teamInfo[playerid][TID]) {
            format(tmp, sizeof tmp, "[%s] 错误的成员ID.", teamInfo[playerid][Label]);
            SCM(playerid, COLOR_BLUE, tmp);
            return 1;
        }
        new Float:x, Float:y, Float:z;
        GetPlayerPos(pid, x, y, z);
        SetPlayerPosEx(playerid, x, y, z + 1);
        format(tmp, sizeof tmp, "[%s] 你传送到了 %s 的身边", teamInfo[pid][Label], GetName(pid));
        SCM(playerid, COLOR_BLUE, tmp);
        format(tmp, sizeof tmp, "[%s] %s 传送到了你的身边", teamInfo[playerid][Label], GetName(playerid));
        SCM(pid, COLOR_BLUE, tmp);
        return 1;
    }
    if(strcmp(tmp, "get", true) == 0) {
        if(teamInfo[playerid][Level] < 2) {
            format(tmp, sizeof tmp, "[%s] 你不可以使用该命令, 原因:等级不够.", teamInfo[playerid][Label]);
            SCM(playerid, COLOR_BLUE, tmp);
            return 1;
        }
        tmp = strtok(params, idx);
        if(!strlen(tmp)) {
            format(tmp, sizeof tmp, "[%s] /t get [成员ID] 可以拉某个成员到身边", teamInfo[playerid][Label]);
            SCM(playerid, COLOR_BLUE, tmp);
            return 1;
        }
        new pid = strval(tmp);
        if(pid < 0 || pid > MAX_PLAYERS || IsPlayerConnected(pid) == 0 || teamInfo[pid][TID] != teamInfo[playerid][TID]) {
            format(tmp, sizeof tmp, "[%s] 错误的成员ID.", teamInfo[playerid][Label]);
            SCM(playerid, COLOR_BLUE, tmp);
            return 1;
        }
        new Float:x, Float:y, Float:z;
        GetPlayerPos(playerid, x, y, z);
        SetPlayerPosEx(pid, x, y, z + 1);
        format(tmp, sizeof tmp, "[%s] 你把成员 %s 拉到了你的身边", teamInfo[pid][Label], GetName(pid));
        SCM(playerid, COLOR_BLUE, tmp);
        format(tmp, sizeof tmp, "[%s] %s 把你拉到了他的身边", teamInfo[playerid][Label], GetName(playerid));
        SCM(pid, COLOR_BLUE, tmp);
        return 1;
    }
    idx=0;
    tmp = strrest(params, idx);
    format(tmp, sizeof tmp, "[%s] %s(%d):%s", teamInfo[playerid][Label], GetName(playerid), playerid, tmp);
    SCMToTeam(teamInfo[playerid][TID], COLOR_BLUE, tmp);
    return 1;
}

stock SCMToTeam(tid, color, string[]) {
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1 && teamInfo[i][TID] != 0 && teamInfo[i][TID] == tid) {
            SCM(i, color, string);
        }
    }
}

teamInit(playerid) {
    teamInfo[playerid][TID] = 0;
    format(teamInfo[playerid][Label], 2, "");
    teamInfo[playerid][Level] = 0;
    new name[MAX_PLAYER_NAME];
    format(name, sizeof name, isNameHasTeamName(GetName(playerid)));
    if(strlen(name)) {
        new Cache:result, msg[128], exist;
        format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM team_list WHERE name='%s'", name);
        result = mysql_query(mariadb, msg);
        cache_get_value_name_int(0, "exist", exist);
        cache_delete(result);
        if(exist == 0) {
            format(msg, sizeof msg, "[系统] 该团队名称[%s]未注册, 你可以关注微信公众号:samp狂野之城.申请.", name);
            SCM(playerid, COLOR_BLUE, msg);
        } else {
            new playername[MAX_PLAYER_NAME];
            GetPlayerName(playerid, playername, sizeof playername);
            format(msg, sizeof msg, "SELECT label FROM team_list WHERE name = '%s'", name);
            result = mysql_query(mariadb, msg);
            cache_get_value_name(0, "label", teamInfo[playerid][Label], MAX_PLAYER_NAME);
            cache_delete(result);
            format(msg, sizeof msg, "SELECT * FROM team_members WHERE name = '%s'", playername);
            result = mysql_query(mariadb, msg);
            cache_get_value_name_int(0, "tid", teamInfo[playerid][TID]);
            cache_get_value_name_int(0, "level", teamInfo[playerid][Level]);
            cache_delete(result);
            if(teamInfo[playerid][TID] != 0) {
                format(msg, sizeof msg, "[%s] {00FFFF}%s{FFFF88}%s成员{00FFAA}%s{00AAFF}ID: %d{FFFF88} 已上线", teamInfo[playerid][Label], name, teamInfo[playerid][Label], playername, playerid);
                SCMToAll(COLOR_BLUE, msg);
    	        format(msg, sizeof msg, "欢迎您{00FFAA}%s{FFFF88}回到%s{00FFFF}%s{FFFF88}等级{FFAA00}%d{FFFF00}:-)", playername, teamInfo[playerid][Label], name, teamInfo[playerid][Level]);
                SCM(playerid, COLOR_BLUE, msg);
            }
        }
    }
}

stock teamResetLeader(playerid, newleader[], teamname[]) {
    if(!strlen(teamname)) {
        SCM(playerid, COLOR_WHITE, "[系统] 团队名称不能为空");
        return 1;
    }
    if(!strlen(newleader)) {
        SCM(playerid, COLOR_WHITE, "[系统] 新领导人不能为空");
        return 1;
    }
    new tname[MAX_PLAYER_NAME], msg[128];
    format(tname, sizeof tname, isNameHasTeamName(newleader));
    if(!strlen(tname) || strcmp(tname, teamname) != 0) {
        SCM(playerid, COLOR_WHITE, "[系统] 新领导人的名称不具有团队名称");
        return 1;
    }
    new Cache:result, exist;
    format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM team_list WHERE name='%s'", teamname);
    mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "exist", exist);
    cache_delete(result);
    if(exist == 0) {
        SCM(playerid, COLOR_WHITE, "[系统] 该团队名称不存在.");
        return 1;
    }
    new tid;
    format(msg, sizeof msg, "SELECT id FROM team_list WHERE name='%s'", teamname);
    result = mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "id", tid);
    cache_delete(result);
    format(msg, sizeof msg, "DELETE FROM team_members WHERE tid='%d' AND level='%d'", tid, 3);
    mysql_query(mariadb, msg, false);
    format(msg, sizeof msg, "INSERT INTO team_members (tid,name,level) VALUES ('%d','%s','%d')", tid, newleader, 3);
    mysql_query(mariadb, msg, false);
    format(msg, sizeof msg, "[系统] 你把[%s]的领导人设置成了 %s", teamname, newleader);
    SCM(playerid, COLOR_WHITE, msg);
    return 1;
}

stock teamDelete(playerid, teamname[]) {
    if(!strlen(teamname)) {
        SCM(playerid, COLOR_WHITE, "[系统] 团队名称不能为空");
        return 1;
    }
    new Cache:result, exist, msg[128];
    format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM team_list WHERE name='%s'", teamname);
    mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "exist", exist);
    if(exist == 0) {
        SCM(playerid, COLOR_WHITE, "[系统] 该团队名称不存在");
    } else {
        new tid = 0, label[MAX_PLAYER_NAME];
        format(msg, sizeof msg, "SELECT * FROM team_list WHERE name='%s'", teamname);
        result = mysql_query(mariadb, msg);
        cache_get_value_name_int(0, "id", tid);
        cache_get_value_name(0, "label", label);
        if(tid == 0) {
            SCM(playerid, COLOR_RED, "[系统] 未知错误, 请联系后台程序员.");
        } else {
            format(msg, sizeof msg, "DELETE FROM team_list WHERE name='%s'", teamname);
            mysql_query(mariadb, msg, false);
            format(msg, sizeof msg, "DELETE FROM team_members WHERE tid='%d'", tid);
            mysql_query(mariadb, msg, false);
            format(msg, sizeof msg, "DELETE FROM team_applications WHERE tid='%d'", tid);
            mysql_query(mariadb, msg, false);
            for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
                if(IsPlayerConnected(i) == 1 && teamInfo[i][TID] == tid) {
                    format(msg, sizeof msg, "[系统] 你的%s已被管理员删除.", teamInfo[i][Label]);
                    SCM(i, COLOR_RED, msg);
                }
            }
            format(msg, sizeof msg, "[系统] 你删了一个名为[%s]的%s", teamname, label);
            SCM(playerid, COLOR_RED, msg);
        }
    }
    cache_delete(result);
    return 1;
}

stock teamCreate(playerid, teamleader[], teamlabel[], teamname[]) {
    if(!strlen(teamname)) {
        SCM(playerid, COLOR_WHITE, "[系统] 团队名称不能为空");
        return 1;
    }
    if(strlen(teamname) > MAX_PLAYER_NAME) {
        new msg[88];
        format(msg, sizeof msg, "[系统] 团队名称不能大于%d字符", MAX_PLAYER_NAME);
        SCM(playerid, COLOR_WHITE, msg);
        return 1;
    }
    if(!strlen(teamlabel)) {
        SCM(playerid, COLOR_WHITE, "[系统] 团队标签不能为空");
        return 1;
    }
    if(strlen(teamlabel) > MAX_PLAYER_NAME) {
        new msg[88];
        format(msg, sizeof msg, "[系统] 团队标签不能大于%d字符", MAX_PLAYER_NAME);
        SCM(playerid, COLOR_WHITE, msg);
        return 1;
    }
    if(!strlen(teamleader)) {
        SCM(playerid, COLOR_WHITE, "[系统] 团队领导人不能为空");
        return 1;
    }
    if(strlen(teamleader) > MAX_PLAYER_NAME) {
        new msg[88];
        format(msg, sizeof msg, "[系统] 团队领导人不能大于%d字符", MAX_PLAYER_NAME);
        SCM(playerid, COLOR_WHITE, msg);
        return 1;
    }
    new Cache:result, exist, msg[256];
    format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM team_list WHERE name='%s'", teamname);
    mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "exist", exist);
    cache_delete(result);
    if(exist != 0) {
        format(msg, sizeof msg, "[系统] 创建失败, 原因:[%s]已存在", teamname);
        SCM(playerid, COLOR_WHITE, msg);
        return 1;
    }
    format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM team_members WHERE name='%s'", teamleader);
    mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "exist", exist);
    cache_delete(result);
    if(exist != 0) {
        format(msg, sizeof msg, "[系统] 创建失败, 原因:%s已在一个团队中.", teamleader);
        SCM(playerid, COLOR_WHITE, msg);
        return 1;
    }
    format(msg, sizeof msg, "INSERT INTO team_list (name,label) VALUES ('%s','%s')", teamname, teamlabel);
    mysql_query(mariadb, msg, false);
    new tid;
    format(msg, sizeof msg, "SELECT id FROM team_list WHERE name='%s'", teamname);
    result = mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "id", tid);
    cache_delete(result);
    if(tid == 0) {
        SCM(playerid, COLOR_WHITE, "[系统] 创建失败, 原因:未知(请自行查看mysql错误记录)");
        return 1;
    }
    format(msg, sizeof msg, "INSERT INTO team_members (tid,name,level) VALUES ('%d','%s','%d')", tid, teamleader, 3);
    mysql_query(mariadb, msg, false);
    format(msg, sizeof msg, "[系统] 你为 %s 创建了一个名为[%s]的%s", teamleader, teamname, teamlabel);
    SCM(playerid, COLOR_WHITE, msg);
    return 1;
}
