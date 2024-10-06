#define DIALOG_TELE                     (300)
#define MAX_TELE_INTRODUCE              (55)

new TELE_SLASH, TELE_2SLASH, pTelePage[MAX_PLAYERS], pTeleSort[MAX_PLAYERS];

CMD:telemenu(playerid, params[]) {
    pTelePage[playerid] = 1;
    pTeleSort[playerid] = 0;
    teleDialogList(playerid, pTelePage[playerid]);
    return 1;
}

CMD:vmake(playerid, params[]) {
    if(!strlen(params)) {
        SCM(playerid, COLOR_WHITE, "[命令] /vmake [传送点] [介绍]");
        SCM(playerid, COLOR_WHITE, "例子: /vmake ldz 雷达站");
        return 1;
    }
    new idx;
    teleC2Slash(playerid, strtok(params, idx), strtok(params, idx));
    return 1;
}

stock tele_changeNtoN(oldname[], newname[]) {
    new msg[256];
    format(msg, sizeof msg, "UPDATE tele_s SET author='%s' WHERE author='%s'", newname, oldname);
    mysql_query(mariadb, msg, false);
    format(msg, sizeof msg, "UPDATE tele_ss SET author='%s' WHERE author='%s'", newname, oldname);
    mysql_query(mariadb, msg, false);
}

stock teleDialogList(playerid, page) {
    new msg[1024], Cache:result, count, sort[128];
    if(pTeleSort[playerid] == 0) {
        format(msg, sizeof msg, "SELECT name,introduce,flow FROM tele_s");
        format(sort, sizeof sort, "{00FF00}创建顺序\t{FFFFFF}点击切换\t{CFCFCF}流量顺序");
    } else if(pTeleSort[playerid] == 1) {
        format(msg, sizeof msg, "SELECT name,introduce,flow FROM tele_s ORDER BY flow DESC");
        format(sort, sizeof sort, "{00FF00}流量顺序\t{FFFFFF}点击切换\t{CFCFCF}创建顺序");
    }
	result = mysql_query(mariadb, msg);
	cache_get_row_count(count);
    if(count == 0) {
        DIALOG_node(playerid, "暂时还没有官方传送点");
    } else {
		format(msg, sizeof msg, "传送点\t名称\t流量\n%s\n", sort);
        if(page != 1) {
            format(msg, sizeof msg, "%s上一页\t \t \n", msg);
        }
        new pagemax = 15, maxpage = GetMaxPage(count);
		for(new nowid = 0; nowid < count; nowid ++) {
            if(nowid >= (page - 1) * pagemax && nowid < (pagemax * page)) {
                new name[MAX_PLAYER_NAME], introduce[MAX_TELE_INTRODUCE], flow;
                cache_get_value_name(nowid, "name", name);
                cache_get_value_name(nowid, "introduce", introduce);
                cache_get_value_name_int(nowid, "flow", flow);
                format(msg, sizeof msg, "%s/%s\t%s\t%d\n", msg, name, introduce, flow);
            }
		}
        if(page != maxpage && maxpage != 1) {
            format(msg, sizeof msg, "%s下一页\t \t ", msg);
        }
        ShowPlayerDialog(playerid, DIALOG_TELE, DIALOG_STYLE_TABLIST_HEADERS, "官方传送点列表", msg, "选择", "关闭");
	}
    cache_delete(result);
}

forward teleODR(playerid, dialogid, response, listitem, inputtext[]);
public teleODR(playerid, dialogid, response, listitem, inputtext[]) {
    if(dialogid == DIALOG_TELE) {
        if(!response) {
            return 1;
        }
        if(strcmp(inputtext, "创建顺序") == 0) {
            pTeleSort[playerid] = 1;
            teleDialogList(playerid, pTelePage[playerid]);
            return 1;
        }
        if(strcmp(inputtext, "流量顺序") == 0) {
            pTeleSort[playerid] = 0;
            teleDialogList(playerid, pTelePage[playerid]);
            return 1;
        }
        if(strcmp(inputtext, "上一页") == 0) {
            pTelePage[playerid] --;
            teleDialogList(playerid, pTelePage[playerid]);
            return 1;
        }
        if(strcmp(inputtext, "下一页") == 0) {
            pTelePage[playerid] ++;
            teleDialogList(playerid, pTelePage[playerid]);
            return 1;
        }
        strdel(inputtext, 0, 1);
        teleSlash(playerid, inputtext);
        return 1;
    }
    return 1;
}

teleInit() {
    new Cache:result;
    result = mysql_query(mariadb, "SELECT COUNT(id) AS nowmax FROM tele_s");
    cache_get_value_name_int(0, "nowmax", TELE_SLASH);
    cache_delete(result);
    result = mysql_query(mariadb, "SELECT COUNT(id) AS nowmax FROM tele_ss");
    cache_get_value_name_int(0, "nowmax", TELE_2SLASH);
    cache_delete(result);
    printf("tele -> 系统传送点当前拥有%d个", TELE_SLASH);
    printf("tele -> 玩家自制传送点当前拥有%d个", TELE_2SLASH);
}

stock teleCSlash(playerid, introduce[], name[]) {
    if(!strlen(name)) {
        SCM(playerid, COLOR_WHITE, "[系统] 传送点名称不能为空");
        return 1;
    }
    new msg[256];
    if(strlen(name) > MAX_PLAYER_NAME) {
        format(msg, sizeof msg, "[系统] 传送点名称长度不能大于%d字符", MAX_PLAYER_NAME);
        SCM(playerid, COLOR_WHITE, msg);
    }
    if(!strlen(introduce)) {
        SCM(playerid, COLOR_WHITE, "[系统] 传送点介绍不能为空");
        return 1;
    }
    if(strlen(introduce) > MAX_TELE_INTRODUCE) {
        format(msg, sizeof msg, "[系统] 传送点介绍长度不能大于%d字符", MAX_TELE_INTRODUCE);
        SCM(playerid, COLOR_WHITE, msg);
    }
    new Cache:result, exist;
    format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM tele_s WHERE name='%s'", name);
    mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "exist", exist);
    cache_delete(result);
    if(exist != 0) {
        format(msg, sizeof msg, "[系统] 传送点'/%s'创建失败, 原因:该传送点名称已存在", name);
        SCM(playerid, COLOR_WHITE, msg);
        return 1;
    }
    new Float:x, Float:y, Float:z, Float:a = GetPlayerAngle(playerid);
    GetPlayerPos(playerid, x, y, z);
    format(msg, sizeof msg, "INSERT INTO tele_s (name,introduce,author,x,y,z,a,flow,unixtime) VALUES ('%s','%s','%s','%.3f','%.3f','%.3f','%.3f','%d','%d')",
    name, introduce, GetName(playerid), x, y, z, a, 0, gettime());
    mysql_query(mariadb, msg, false);
    format(msg, sizeof msg, "[系统] 你创建了一个名为: /%s %s 的传送点", name, introduce);
    SCM(playerid, COLOR_WHITE, msg);
    return 1;
}

stock teleC2Slash(playerid, introduce[], name[]) {
    if(!strlen(name)) {
        SCM(playerid, COLOR_WHITE, "[系统] 传送点名称不能为空");
        return 1;
    }
    new msg[256];
    if(strlen(name) > MAX_PLAYER_NAME) {
        format(msg, sizeof msg, "[系统] 传送点名称长度不能大于%d字符", MAX_PLAYER_NAME);
        SCM(playerid, COLOR_RED, msg);
    }
    if(!strlen(introduce)) {
        SCM(playerid, COLOR_RED, "[系统] 传送点介绍不能为空");
        return 1;
    }
    if(strlen(introduce) > MAX_TELE_INTRODUCE) {
        format(msg, sizeof msg, "[系统] 传送点介绍长度不能大于%d字符", MAX_TELE_INTRODUCE);
        SCM(playerid, COLOR_RED, msg);
    }
    new Cache:result, exist;
    format(msg, sizeof msg, "SELECT COUNT(name) AS exist FROM tele_ss WHERE name='%s'", name);
    mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "exist", exist);
    cache_delete(result);
    if(exist != 0) {
        format(msg, sizeof msg, "[系统] 传送点'//%s'创建失败, 原因:该传送点名称已存在", name);
        SCM(playerid, COLOR_RED, msg);
        return 1;
    }
    new Float:x, Float:y, Float:z, Float:a = GetPlayerAngle(playerid);
    GetPlayerPos(playerid, x, y, z);
    format(msg, sizeof msg, "INSERT INTO tele_ss (name,introduce,author,x,y,z,a,flow,unixtime) VALUES ('%s','%s','%s','%.3f','%.3f','%.3f','%.3f','%d','%d')",
    name, introduce, GetName(playerid), x, y, z, a, 0, gettime());
    mysql_query(mariadb, msg, false);
    format(msg, sizeof msg, "[系统] 你创建了一个名为: //%s %s 的传送点", name, introduce);
    SCM(playerid, COLOR_BLUE, msg);
    return 1;
}

stock teleSlash(playerid, name[]) {
    new Cache:result, msg[144], id, introduce[MAX_TELE_INTRODUCE], telename[MAX_PLAYER_NAME], author[MAX_PLAYER_NAME], Float:x, Float:y, Float:z, Float:a, flow;
    format(msg, sizeof msg, "SELECT * FROM tele_s WHERE name='%s'", name);
    result = mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "id", id);
    cache_get_value_name(0, "name", telename);
    cache_get_value_name(0, "introduce", introduce);
    cache_get_value_name(0, "author", author);
    cache_get_value_name_float(0, "x", x);
    cache_get_value_name_float(0, "y", y);
    cache_get_value_name_float(0, "z", z);
    cache_get_value_name_float(0, "a", a);
    cache_get_value_name_int(0, "flow", flow);
    cache_delete(result);
    flow ++;
    if(id == 0) return 0;
    SetPlayerPosEx(playerid, x, y, z, a);
    format(msg, sizeof msg, "[传送]{00FFFF} %s(%d){FFFF00} 传送到 {FF00FF}/%s {00FF00}%s {FFFF00}%s {CCFF00}流量:{AACC00}%d", GetName(playerid), playerid, telename, introduce, author, flow);
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1 && GetPlayerTeleReceiveStatus(i) == 1) {
            SCM(i, COLOR_YELLOW, msg);
        }
    }
    format(msg, sizeof msg, "UPDATE tele_s SET flow='%d' WHERE id='%d'", flow, id);
    mysql_query(mariadb, msg, false);
    return 1;
}

stock tele2Slash(playerid, name[]) {
    strdel(name, 0, 1);
    new Cache:result, msg[128], id, introduce[MAX_TELE_INTRODUCE], telename[MAX_PLAYER_NAME], author[MAX_PLAYER_NAME], Float:x, Float:y, Float:z, Float:a, flow;
    format(msg, sizeof msg, "SELECT * FROM tele_ss WHERE name='%s'", name);
    result = mysql_query(mariadb, msg);
    cache_get_value_name_int(0, "id", id);
    cache_get_value_name(0, "name", telename);
    cache_get_value_name(0, "introduce", introduce);
    cache_get_value_name(0, "author", author);
    cache_get_value_name_float(0, "x", x);
    cache_get_value_name_float(0, "y", y);
    cache_get_value_name_float(0, "z", z);
    cache_get_value_name_float(0, "a", a);
    cache_get_value_name_int(0, "flow", flow);
    cache_delete(result);
    flow ++;
    if(id == 0) {
        format(msg, sizeof msg, "[传送] 你输入的传送点'//%s'不存在", name);
        SCM(playerid, COLOR_BLUE, msg);
    } else {
        SetPlayerPosEx(playerid, x, y, z, a);
        format(msg, sizeof msg, "[传送] 你传送到了 //%s", telename);
        SCM(playerid, COLOR_BLUE, msg);
        format(msg, sizeof msg, "UPDATE tele_ss SET flow='%d' WHERE id='%d'", flow, id);
        mysql_query(mariadb, msg, false);
    }
}
