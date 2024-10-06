#include <scripts/vspawner>

new pTemporaryVID[MAX_PLAYERS], Text3D:pTemporaryVIDLabel[MAX_PLAYERS];

stock GetPlayerTemporaryVID(playerid) {
    return pTemporaryVID[playerid];
}

CMD:f(playerid, params[]) {
    new vid = GetPlayerVehicleID(playerid);
    if(vid == 0) return SCM(playerid, COLOR_GREEN, "[临时载具] 你不在任何载具内.");
    new Float:x, Float:y, Float:z;
    GetVehiclePos(vid, x, y, z);
    SetVehiclePos(vid, x, y, z + 0.5);
    SetVehicleZAngle(vid, GetPlayerAngle(playerid));
    return 1;
}

CMD:c(playerid, params[]) {
    if(!strlen(params)) {
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c [载具ID] [颜色1] [颜色2] 可以创建临时载具.");
        return 1;
    }
    new tmp[128], idx;
    tmp = strtok(params, idx);
    if(strcmp(tmp, "help", true) == 0) {
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c wode 可以传送你的专用交通工具.");
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c ti 可以踢掉专用交通工具上的乘客.");
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c suo 可以锁住你的专用交通工具.");
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c color [颜色1ID] [颜色1ID] 可以更改你的专用交通工具的颜色.");
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c [载具ID] [颜色1] [颜色2] 可以创建专用交通工具.");
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c fix 可以完全修复你的载具");
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c dl 隐藏/显示文字");
        SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /c list 载具列表");
		SCM(playerid, COLOR_GREEN, "[临时载具] 输入 /f 可以翻正你的载具");
        return 1;
    } else if(strcmp(tmp, "list", true) == 0) {
        vspawnerOpen(playerid);
        return 1;
	} else if(strcmp(tmp, "dl", true) == 0) {
		if(pTemporaryVID[playerid] != 0) {
			if(IsValidDynamic3DTextLabel(pTemporaryVIDLabel[playerid]) == 1) {
				DestroyDynamic3DTextLabel(pTemporaryVIDLabel[playerid]);
			} else {
				format(tmp, sizeof tmp, "%s的临时载具", GetName(playerid));
				pTemporaryVIDLabel[playerid] = CreateDynamic3DTextLabel(tmp, COLOR_WHITE, 0.0, 0.0, 0.0, 10.0, INVALID_PLAYER_ID, pTemporaryVID[playerid]);
			}
		}
		return 1;
    } else if(strcmp(tmp, "fix", true) == 0) {
        new vid = GetPlayerVehicleID(playerid);
        if(vid == 0) return SCM(playerid, COLOR_GREEN, "[临时载具]你不在任何载具内.");
        RepairVehicle(vid);
        SCM(playerid, COLOR_GREEN, "[临时载具] 该载具已完全修复.");
        return 1;
    } else if(strcmp(tmp, "color", true) == 0) {
        if(pTemporaryVID[playerid] == 0) return SCM(playerid, COLOR_BLUE, "[临时载具] 你还没有载具呢.");
        tmp = strtok(params, idx);
        if(!strlen(tmp)) return SCM(playerid, COLOR_GREEN, "[临时载具] 颜色1ID不能为空");
        new color1 = strval(tmp);
        tmp = strtok(params, idx);
        if(!strlen(tmp)) return SCM(playerid, COLOR_GREEN, "[临时载具] 颜色2ID不能为空");
        new color2 = strval(tmp);
        ChangeVehicleColor(pTemporaryVID[playerid], color1, color2);
        SCM(playerid, COLOR_GREEN, "[临时载具] 你的载具颜色已更改.");
        return 1;
    } else if(strcmp(tmp, "suo", true) == 0) {
        if(pTemporaryVID[playerid] == 0) return SCM(playerid, COLOR_BLUE, "[载具] 你还没有专用交通工具呢.");
        new engine, lights, alarm, doors, bonnet, boot, objective;
        GetVehicleParamsEx(pTemporaryVID[playerid], engine, lights, alarm, doors, bonnet, boot, objective);
        if(doors == 0) {
            doors = 1;
            SCM(playerid, COLOR_BLUE, "[载具] 你的专用交通工具已上锁.");
        } else {
            doors = 0;
            SCM(playerid, COLOR_BLUE, "[载具] 你的专用交通工具没锁了.");
        }
        SetVehicleParamsEx(pTemporaryVID[playerid], engine, lights, alarm, doors, bonnet, boot, objective);
        return 1;
    } else if(strcmp(tmp, "ti", true) == 0) {
        if(pTemporaryVID[playerid] == 0) return SCM(playerid, COLOR_BLUE, "[载具] 你还没有专用交通工具呢.");
        new ojbk = 0;
        for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
            if(IsPlayerConnected(i) == 1 && i != playerid && GetPlayerVehicleID(i) == pTemporaryVID[playerid]) {
                SCM(i, COLOR_BLUE, "[载具] 你被载具主人踢出了载具.");
                ojbk = 1;
            }
        }
        if(ojbk == 0) {
            SCM(playerid, COLOR_BLUE, "[载具] 你的载具上并没有任何乘客.");
        } else {
            SCM(playerid, COLOR_BLUE, "[载具] 在你专用交通工具上的乘客已被踢出.");
        }
        return 1;
    } else if(strcmp(tmp, "wode", true) == 0) {
        if(pTemporaryVID[playerid] == 0) return SCM(playerid, COLOR_BLUE, "[临时载具] 你还没有临时载具.");
        new Float:x, Float:y, Float:z, Float:a = GetPlayerAngle(playerid);
        GetPlayerPos(playerid, x, y, z);
        SetVehiclePos(pTemporaryVID[playerid], x, y, z);
        SetVehicleZAngle(pTemporaryVID[playerid], a);
        SetVehicleVirtualWorld(pTemporaryVID[playerid], GetPlayerVirtualWorld(playerid));
        SCM(playerid, COLOR_BLUE, "[临时载具] 你召唤了你的临时载具.");
        PutPlayerInVehicle(playerid, pTemporaryVID[playerid], 0);
        return 1;
    }
    new vid, color1, color2;
    vid = strval(tmp);
    if(vid < 400 || vid > 611) {
        SCM(playerid, COLOR_GREEN, "[临时载具] 错误的载具ID.");
        return 1;
    }
    if(GetPlayerVirtualWorld(playerid) == 0) {
        if(vid == 577 || vid == 592 || vid == 432 || vid == 520 || vid == 406 || vid == 444) {
            SCM(playerid, COLOR_GREEN, "[临时载具] 大世界里不可以刷大型载具.");
            return 1;
        }
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        color1 = random(255);
    } else {
        color1 = strval(tmp);
    }
    tmp = strtok(params, idx);
    if(!strlen(tmp)) {
        color2 = random(255);
    } else {
        color2 = strval(tmp);
    }
    new Float:x, Float:y, Float:z, Float:a = GetPlayerAngle(playerid);
    GetPlayerPos(playerid, x, y, z);
    temporary_vehicleCreate(playerid, vid, x, y, z, a, color1, color2);
    return 1;
}

stock temporary_vehicleCreate(playerid, modelid, Float:x, Float:y, Float:z, Float:a, color1, color2) {
    temporary_vehicleInit(playerid);
    new tmp[128];
    GetPlayerPos(playerid, x, y, z);
    pTemporaryVID[playerid] = CreateVehicle(modelid, x, y, z, a, color1, color2, -1);
    if(pTemporaryVID[playerid] == INVALID_VEHICLE_ID) {
        DIALOG_node(playerid, "创建失败, 原因:服务器载具已达到上限");
        return 1;
    }
    format(tmp, sizeof tmp, "%s的临时载具", GetName(playerid));
    pTemporaryVIDLabel[playerid] = CreateDynamic3DTextLabel(tmp, COLOR_WHITE, 0.0, 0.0, 0.0, 10.0, INVALID_PLAYER_ID, pTemporaryVID[playerid]);
    SetVehicleVirtualWorld(pTemporaryVID[playerid], GetPlayerVirtualWorld(playerid));
    PutPlayerInVehicle(playerid, pTemporaryVID[playerid], 0);
    AddVehicleComponent(pTemporaryVID[playerid], 1010);
    format(tmp, sizeof tmp, "[临时载具] ---刷车成功,ID:%d 狂野第三交通委提醒您:道路千万条,安全第一条,行车不规范,亲人两行泪!---", modelid);
    SCM(playerid, COLOR_GREEN, tmp);
    return 1;
}

temporary_vehicleOPSC(playerid, newstate) {
    if(newstate == 2) {
        new vid = GetPlayerVehicleID(playerid);
        if(vid == pTemporaryVID[playerid]) {
            //SCM(playerid, COLOR_GREEN, "[临时载具] 欢迎回到你的临时载具.");
        } else {
            for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
                if(IsPlayerConnected(i) == 1 && i != playerid && vid == pTemporaryVID[i]) {
                    new msg[128];
                    format(msg, sizeof msg, "[临时载具] 该临时载具属于 %s", GetName(i));
                    SCM(playerid, COLOR_GREEN, msg);
                    SCM(playerid, COLOR_GREEN, "[临时载具] 你可以输入 /c color [颜色1ID] [颜色2ID] 创建属于你的临时载具.");
                    break;
                }
            }
        }
    }
}

temporary_vehicleInit(playerid) {
    if(pTemporaryVID[playerid] != 0) {
        if(IsValidDynamic3DTextLabel(pTemporaryVIDLabel[playerid]) == 1) {
            DestroyDynamic3DTextLabel(pTemporaryVIDLabel[playerid]);
        }
        DestroyVehicle(pTemporaryVID[playerid]);
        pTemporaryVID[playerid] = 0;
    }
}
