#include <a_mysql>
#include <ASAN>
#include <Pawn.CMD>
#include <streamer>
#include <physics>
#include <sscanf2>
#define MAX_CONNMSG                 (30)
#define MAX_CONNMSG_LEN             (144)
#define MAX_SERVNAME_LEN            (128)
#define SCM                         SendClientMessage
#define SCMToAll                    SendClientMessageToAll
#define COLOR_WHITE                 (0xEFEFF7FF)
#define COLOR_GREEN                 (0x4DF752FF)
#define COLOR_BLUE                  (0x3FB7ECFF)
#define COLOR_RED                   (0xFF0000FF)
#define COLOR_YELLOW                (0xF9FF2DFF)
#define COLOR_ORANGE                (0xFEA979FF)
#define COLOR_VIP                   (0xFFFF00C8)
#define defaultIT                   "sbkyc1ty"
#define MAX_REASON_LEN              (88)
#define MAX_PLAYER_IP_LEN           (16)
#define MAX_PSPS                    (20)
#define MAX_LENOF_SPNAME            (30)
#define MAX_ANNOUNCEMENTS           (30)
#define DEFAULT_ANNOUNCEMENT_SEC    (600)
#define SERV_MODE_PRODUCTION		(0)
#define SERV_MODE_DEVELOPMENT		(1)
#define Attire.                     Attire_

#define OBJ_SLOT_POOL (0)

///////氮气
#define NITROGEN_TIME (0) //定义氮气冷却为0
new NitrogenCooldown[MAX_PLAYERS]; //创建载具氮气变量

enum poolBall
{
	bObject,
	bExisting
}

new
	PlayingPool[MAX_PLAYERS],
	PoolCamera[MAX_PLAYERS],
	UsingChalk[MAX_PLAYERS],
	PoolScore[MAX_PLAYERS],
	Float:AimAngle[MAX_PLAYERS][2],
	AimObject,
	PoolStarted,
	PoolAimer = -1,
	PoolLastShooter = -1,
	PoolLastScore,
	PoolBall[16][poolBall],
	Text:PoolTD[4],
	Float:PoolPower,
	PoolDir;



CMD:pool(playerid, params[])
{
	if(!PlayingPool[playerid])
	{
		PlayingPool[playerid] = 1;
		PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
		GivePlayerWeapon(playerid, 7, 1);
		SCM(playerid, COLOR_WHITE, "[台球] 加载游戏成功请在白球旁边按N瞄准,按A/B进行调整出球视角,按Y擦杆，按空格射击手机为[SPR]");
		PoolScore[playerid] = 0;
		if(!PoolStarted)
		{
			PoolStarted = 1;
			RespawnPoolBalls(1);
		}
	}
	else
	{
		if(PoolAimer != playerid)
		{
			PlayingPool[playerid] = 0;
			new
				count = GetPoolPlayersCount();
			if(count <= 0)
			{
				PoolStarted = 0;
				RespawnPoolBalls();
			}
		}
	}
	return 1;
}

/* 创建战局 */
CMD:ceadd(playerid, params[])
{
    if (GetChaseStatus() == CHASE_STATUS_GATHERING) return SendClientMessage(playerid, -1, CHASE_INFO"追逐战局正在集结中, 无法创建。请输入: /cejoin 加入。");
    if (GetChaseStatus() == CHASE_STATUS_RUNNING) return SendClientMessage(playerid, -1, CHASE_INFO"追逐战局正在进行中, 无法创建。");
    if (GetChaseStatus() == CHASE_STATUS_ENDING) return SendClientMessage(playerid, -1, CHASE_INFO"追逐战局已结束, 正在打扫战局，请稍后再试。");
    if (GetChaseStatus() == CHASE_STATUS_FREEING)
    {
        if (List_GetSize(List_PlayPlayer) >= CHASES_MAX_PLAYERS) return SendClientMessage(playerid, -1, CHASE_INFO"追逐战局已满员, 无法创建。");

        return ShowPlayerDialog(playerid, 9988, DIALOG_STYLE_MSGBOX, "创建 警察&毒贩-追逐战局", "最大加入人数: 9名\n最大警察人数: 6名\n最大扮演毒贩人数: 3名", "创建", "取消");
    }
    return 1;
}

/* 加入战局 */
CMD:cejoin(playerid, params[])
{
    if (GetChaseStatus() == CHASE_STATUS_FREEING) return SendClientMessage(playerid, -1, CHASE_INFO"当前还没有创建战局，请输入: /ceadd 创建。");
    if (GetChaseStatus() == CHASE_STATUS_RUNNING && List_GetSize(List_PlayPlayer) >= CHASES_MAX_PLAYERS) return SendClientMessage(playerid, -1, CHASE_INFO"当前战局已经开始，操作失败。");
    if (GetChaseStatus() == CHASE_STATUS_ENDING) return SendClientMessage(playerid, -1, CHASE_INFO"当前战局已结束, 正在打扫战局，请稍后输入/ceadd创建。");

    if (GetChaseStatus() == CHASE_STATUS_GATHERING)
    {
        if (List_GetSize(List_PlayPlayer) >= CHASES_MAX_PLAYERS) return SendClientMessage(playerid, -1, CHASE_INFO"追逐战局已满员, 无法加入。");

		if (IsPlayerPolice(playerid) || IsPlayerDealer(playerid))
		{
			format(gString, sizeof gString, "{2165DB}警察[%i/%i{2165DB}]\n{E68911}毒贩[%i/%i{E68911}]\n{1590C0}退出战局",
        List_GetSize(List_PlayPolice), CHASES_MAX_POLICES,
        List_GetSize(List_PlayDealer), CHASES_MAX_DEALERS);
		}else{
			format(gString, sizeof gString, "{2165DB}警察[%i/%i{2165DB}]\n{E68911}毒贩[%i/%i{E68911}]",
        List_GetSize(List_PlayPolice), CHASES_MAX_POLICES,
        List_GetSize(List_PlayDealer), CHASES_MAX_DEALERS);
		}

        ShowPlayerDialog(playerid, DIALOG_CHASE_SELECT, DIALOG_STYLE_LIST, "请选择扮演角色", gString, "选择", "关闭");
        return gString[0] = EOS; // 释放全局数组单元格
    }
    return 1;
}

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys) {
	//if(configOPKSC(playerid, newkeys) == 1) return 1;
	if(bankOPKSC(playerid, newkeys) == 1) return 1;
	if(ACOBJOPKSC(playerid, newkeys) == 1) return 1;
	PHouseOPKSC(playerid, newkeys);

	if (GetChaseStatus() == CHASE_STATUS_RUNNING)
    {
        if (IsPlayerInChasePlayer(playerid) && IsPlayerPolice(playerid))
        {
            if (!GetPlayerInChaseDeath(playerid) && PRESSED(KEY_WALK))
            {
                if (gettime() < ceVar[PoliceSendTackle][playerid]) return 1;
                ceVar[PoliceSendTackle][playerid] = gettime() + TACKLE_TIME;
    
                ApplyAnimation(playerid, "PARACHUTE", "FALL_skyDive_DIE", 4.0, 0, 1, 1, 1, -1);
                ApplyAnimation(playerid, "PED", "EV_dive", 4.1, 0, 1, 1, 0, 0, 1);
                SendClientMessage(playerid, -1, CHASE_INFO"你发动了飞扑抓捕(10秒冷却)!");
                ceVar[PoliceTackleTimer][playerid] = SetTimerEx("OnPoliceTackle", 1700, 0, "i", playerid);
            }
            if (GetPlayerInChaseDeath(playerid) && PRESSED(KEY_FIRE))
            {
                SwitchSpectatingNext(playerid, 1);
            }
        }

        if (IsPlayerInChasePlayer(playerid) && IsPlayerDealer(playerid))
        {
            if (GetPlayerInChaseDeath(playerid) && PRESSED(KEY_FIRE))
            {
                SwitchSpectatingNext(playerid, 0);
            }
        }
    }

	
    if (IsPlayerInAnyVehicle(playerid) && GetPlayerVehicleSeat(playerid) == 0)
    {
        // 判断玩家是否在车上在车上便调用氮气定义
        if (IsKeyJustUp(KEY_FIRE, newkeys, oldkeys))
        {
            if (gettime() < NitrogenCooldown[playerid]) return 0;
            NitrogenCooldown[playerid] = gettime() + NITROGEN_TIME;
            AddVehicleComponent(GetPlayerVehicleID(playerid), 1010);
        }
    }
	if(PoolStarted && PlayingPool[playerid])
	{
		if (IsKeyJustUp(KEY_YES, newkeys, oldkeys))
		{
			if(PlayingPool[playerid] && PoolAimer != playerid && !UsingChalk[playerid])
			{
				SetTimerEx("PlayPoolSound", 1400, 0, "d", 31807);
				SetPlayerArmedWeapon(playerid, 0);
				SetPlayerAttachedObject(playerid, OBJ_SLOT_POOL, 338, 6, 0, 0.07, -0.85, 0, 0, 0);
				ApplyAnimation(playerid, "POOL", "POOL_ChalkCue",3.0,0,0,0,0,0,1);
				UsingChalk[playerid] = 1;
				SetTimerEx("RestoreWeapon", 3500, 0, "d", playerid);
			}
		}
		if (IsKeyJustUp(KEY_CTRL_BACK, newkeys, oldkeys))
		{
			if(PoolAimer == playerid)
			{
				if(PoolCamera[playerid] < 2) PoolCamera[playerid]++;
				else PoolCamera[playerid] = 0;
				new
					Float:poolrot = AimAngle[playerid][0],
					Float:Xa,
					Float:Ya,
					Float:Za,
					Float:x,
					Float:y;
				GetObjectPos(PoolBall[0][bObject], Xa, Ya, Za);
				switch(PoolCamera[playerid])
				{
					case 0:
					{
						GetXYBehindObjectInAngle(PoolBall[0][bObject], poolrot, x, y, 0.675);
						SetPlayerCameraPos(playerid, x, y, 998.86785888672+0.28);
						SetPlayerCameraLookAt(playerid, Xa, Ya, Za+0.170);
					}
					case 1:
					{
						SetPlayerCameraPos(playerid, 511.84469604492, -84.831642150879, 1001.4904174805);
						SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
					}
					case 2:
					{
						SetPlayerCameraPos(playerid, 508.7971496582, -84.831642150879, 1001.4904174805);
						SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
					}
				}
			}
		}
		if (IsKeyJustUp(KEY_NO, newkeys, oldkeys))
		{
			if(AreAllBallsStopped())
			{
				if(PoolAimer != playerid)
				{
					if(!UsingChalk[playerid] && PoolAimer == -1 && PoolBall[0][bExisting])
					{
						new
							Float:poolrot,
							Float:X,
							Float:Y,
							Float:Z,
							Float:Xa,
							Float:Ya,
							Float:Za,
							Float:x,
							Float:y;
						GetPlayerPos(playerid, X, Y, Z);
						GetObjectPos(PoolBall[0][bObject], Xa, Ya, Za);
						if(Is2DPointInRangeOfPoint(X, Y, Xa, Ya, 1.5) && Z < 999.5)
						{
							TogglePlayerControllable(playerid, 0);
							GetAngleToXY(Xa, Ya, X, Y, poolrot);
							SetPlayerFacingAngle(playerid, poolrot);
							AimAngle[playerid][0] = poolrot;
							AimAngle[playerid][1] = poolrot;
							SetPlayerArmedWeapon(playerid, 0);
							GetXYInFrontOfPos(Xa, Ya, poolrot+180, x, y, 0.085);
							AimObject = CreateObject(3004, x, y, Za, 7.0, 0, poolrot+180);
							switch(PoolCamera[playerid])
							{
								case 0:
								{
									GetXYBehindObjectInAngle(PoolBall[0][bObject], poolrot, x, y, 0.675);
									SetPlayerCameraPos(playerid, x, y, 998.86785888672+0.28);
									SetPlayerCameraLookAt(playerid, Xa, Ya, Za+0.170);
								}
								case 1:
								{
									SetPlayerCameraPos(playerid, 511.84469604492, -84.831642150879, 1001.4904174805);
									SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
								}
								case 2:
								{
									SetPlayerCameraPos(playerid, 508.7971496582, -84.831642150879, 1001.4904174805);
									SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
								}
							}
							ApplyAnimation(playerid, "POOL", "POOL_Med_Start",50.0,0,0,0,1,1,1);
							PoolAimer = playerid;
							TextDrawShowForPlayer(playerid, PoolTD[0]);
							TextDrawShowForPlayer(playerid, PoolTD[1]);
							TextDrawTextSize(PoolTD[2], 501.0, 0.0);
							TextDrawShowForPlayer(playerid, PoolTD[2]);
							TextDrawShowForPlayer(playerid, PoolTD[3]);
							PoolPower = 1.0;
							PoolDir = 0;
						}
					}
				}
				else
				{
					TogglePlayerControllable(playerid, 1);
					GivePlayerWeapon(playerid, 7, 1);
					ApplyAnimation(playerid, "CARRY", "crry_prtial", 1.0, 0, 0, 0, 0, 0, 1);
					SetCameraBehindPlayer(playerid);
					PoolAimer = -1;
					DestroyObject(AimObject);
					TextDrawHideForPlayer(playerid, PoolTD[0]);
					TextDrawHideForPlayer(playerid, PoolTD[1]);
					TextDrawHideForPlayer(playerid, PoolTD[2]);
					TextDrawHideForPlayer(playerid, PoolTD[3]);
				}
			}
		}
		if (IsKeyJustUp(KEY_SPRINT, newkeys, oldkeys))
		{
			if(PoolAimer == playerid)
			{
				new
					Float:speed;
				ApplyAnimation(playerid, "POOL", "POOL_Med_Shot",3.0,0,0,0,0,0,1);
				speed = 0.4 + (PoolPower * 2.0) / 100.0;
				PHY_SetObjectVelocity(PoolBall[0][bObject], speed * floatsin(-AimAngle[playerid][0], degrees), speed * floatcos(-AimAngle[playerid][0], degrees));
				if(PoolCamera[playerid] == 0)
				{
					switch(random(2))
					{
						case 0: SetPlayerCameraPos(playerid, 511.84469604492, -84.831642150879, 1001.4904174805);
						case 1: SetPlayerCameraPos(playerid, 508.7971496582, -84.831642150879, 1001.4904174805);
					}
					SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
				}
				PlayPoolSound(31810);
				PoolAimer = -1;
				DestroyObject(AimObject);
				GivePlayerWeapon(playerid, 7, 1);
				PoolLastShooter = playerid;
				PoolLastScore = 0;
				TextDrawHideForPlayer(playerid, PoolTD[0]);
				TextDrawHideForPlayer(playerid, PoolTD[1]);
				TextDrawHideForPlayer(playerid, PoolTD[2]);
				TextDrawHideForPlayer(playerid, PoolTD[3]);
			}
		}
	}
	return 1;
}

public PHY_OnObjectUpdate(objectid)
{
	if(PoolStarted)
	{
		for(new i; i < sizeof PoolBall; i++)
		{
			if(objectid == PoolBall[i][bObject] && PHY_IsObjectMoving(PoolBall[i][bObject]))
			{
				new
					hole = IsBallInHole(i);
				if(hole)
				{
					new
						Float:speed,
						Float:vx, Float:vy, Float:vz;
					PHY_GetObjectVelocity(PoolBall[i][bObject], vx, vy, vz);
					speed = floatsqroot(vx * vx + vy * vy) + 0.2;

					PoolBall[i][bExisting] = 0;
					PHY_DeleteObject(PoolBall[i][bObject]);

					PlayPoolSound(31803 + random(3));

					switch(hole)
					{
						case 1: MoveObject(PoolBall[i][bObject], 509.61123657,-85.79737091,998.86785889-0.25, speed);
						case 2: MoveObject(PoolBall[i][bObject], 510.67373657,-84.84423065,998.86785889-0.25, speed);
						case 3: MoveObject(PoolBall[i][bObject], 510.61914062,-83.88769531,998.86785889-0.25, speed);
						case 4: MoveObject(PoolBall[i][bObject], 509.61077881,-83.89227295,998.86785889-0.25, speed);
						case 5: MoveObject(PoolBall[i][bObject], 510.61825562,-85.80107880,998.86785889-0.25, speed);
						case 6: MoveObject(PoolBall[i][bObject], 509.55642700,-84.84602356,998.86785889-0.25, speed);
					}
					if(i)
					{
						PoolScore[PoolLastShooter] ++;
						PoolLastScore ++;
						new string[128];
						if(PoolLastScore > 0) format(string, 128, "~g~~h~+%d", PoolLastScore);
						else format(string, 128, "~r~~h~%d", PoolLastScore);
						GameTextForPlayer(PoolLastShooter, string, 100000, 4);
						PlayerPlaySound(PoolLastShooter, 1250, 0.0, 0.0, 0.0);
					}
					else
					{
						PoolScore[PoolLastShooter] --;
						PoolLastScore --;
						new string[128];
						if(PoolLastScore > 0) format(string, 128, "~g~~h~+~r~~h~%d", PoolLastScore);
						else format(string, 128, "~r~~h~%d", PoolLastScore);
						GameTextForPlayer(PoolLastShooter, string, 100000, 4);
						PlayerPlaySound(PoolLastShooter, 1250, 0.0, 0.0, 0.0);
					}
					if(GetPoolBallsCount() <= 1)
					{
						PoolStarted = 0;
						PoolAimer = -1;
						new
							winscore = GetMaxPoolScore(),
							name[MAX_PLAYER_NAME];
						RespawnPoolBalls();
						RestoreCamera(PoolLastShooter);
						PoolLastShooter = -1;
						foreach (new p : Player)
						{
							if(PlayingPool[p] && PoolScore[p] == winscore)
							{
								new
									string[128];
								GetPlayerName(p, name, sizeof name);
								format(string, 128, "{FFFF45}台球: {EBEBEB}%s {FFFF45}将最后一颗球，球号: {EBEBEB}%d {FFFF45}打进获得了最终胜利.", name, winscore);
								SendPoolPlayersMessage(string);
							}
						}
						foreach (new p : Player)
						{
							if(PlayingPool[p])
							{
								PlayingPool[p] = 0;
							}
						}
					}
					else if(AreAllBallsStopped())
					{
						SetTimerEx("RestoreCamera", 800, 0, "d", PoolLastShooter);
						PoolLastShooter = -1;
					}
				}
				return 1;
			}
		}
	}
	return 1;
}

public PHY_OnObjectCollideWithObject(object1, object2)
{
	if(PoolStarted)
	{
		for(new i; i < sizeof PoolBall; i++)
		{
			if(object1 == PoolBall[i][bObject])
			{
				PlayPoolSound(31800 + random(3));
				return 1;
			}
		}
	}
	return 1;
}

public PHY_OnObjectCollideWithWall(objectid, wallid)
{
	if(PoolStarted)
	{
		for(new i; i < sizeof PoolBall; i++)
		{
			if(objectid == PoolBall[i][bObject])
			{
				PlayPoolSound(31808);
				return 1;
			}
		}
	}
	return 1;
}

Float:GetPointDistanceToPoint(Float:x1,Float:y1,Float:x2,Float:y2)
{
	new Float:x, Float:y;
	x = x1-x2;
	y = y1-y2;
	return floatsqroot(x*x+y*y);
}


stock GetAngleToXY(Float:X, Float:Y, Float:CurrX, Float:CurrY, &Float:angle)
{
	angle = atan2(Y-CurrY, X-CurrX);
	angle = floatsub(angle, 90.0);
	if(angle < 0.0) angle = floatadd(angle, 360.0);
}

stock GetXYInFrontOfPos(Float:xx,Float:yy,Float:a, &Float:x2, &Float:y2, Float:distance)
{
	if(a>360)
	{
		a=a-360;
	}
	xx += (distance * floatsin(-a, degrees));
	yy += (distance * floatcos(-a, degrees));
	x2=xx;
	y2=yy;
}

stock IsPointFacingPoint(Float:dOffset, Float:X, Float:Y, Float:pA, Float:pX, Float:pY)
{
	new
		Float:ang;

	if( Y > pY ) ang = (-acos((X - pX) / floatsqroot((X - pX)*(X - pX) + (Y - pY)*(Y - pY))) - 90.0);
	else if( Y < pY && X < pX ) ang = (acos((X - pX) / floatsqroot((X - pX)*(X - pX) + (Y - pY)*(Y - pY))) - 450.0);
	else if( Y < pY ) ang = (acos((X - pX) / floatsqroot((X - pX)*(X - pX) + (Y - pY)*(Y - pY))) - 90.0);

	if(AngleInRangeOfAngle(-ang, pA, dOffset)) return true;

	return false;

}

stock Is2DPointInRangeOfPoint(Float:x, Float:y, Float:x2, Float:y2, Float:range)
{
	x2 -= x;
	y2 -= y;
	return ((x2 * x2) + (y2 * y2)) < (range * range);
}

stock IsObjectInSphere(objectid,Float:x,Float:y,Float:z,Float:radius2)
{
	new Float:x1,Float:y1,Float:z1,Float:tmpdis;
	GetObjectPos(objectid,x1,y1,z1);
	tmpdis = floatsqroot(floatpower(floatabs(floatsub(x,x1)),2)+ floatpower(floatabs(floatsub(y,y1)),2)+ floatpower(floatabs(floatsub(z,z1)),2));
	if(tmpdis < radius2) return 1;
	return 0;
}

GetBallModel(i)
{
	switch(i)
	{
		case 0: return 3003;
		case 1: return 3002;
		case 2: return 3001;
		case 3: return 2995;
		case 4: return 2996;
		case 5: return 3106;
		case 6: return 3105;
		case 7: return 3103;
		case 8: return 3001;
		case 9: return 3100;
		case 10: return 2997;
		case 11: return 3000;
		case 12: return 3102;
		case 13: return 2999;
		case 14: return 2998;
		case 15: return 3104;
	}
	return 0;
}

stock GetPoolBallsCount()
{
	new
		count;
	for(new i; i < 16; i++)
	{
		if(PoolBall[i][bExisting] || i == 0) count++;
	}
	return count;
}


stock GetMaxPoolScore()
{
	new
		scoremax = -1;
	foreach (new i : Player)
	{
		if(PlayingPool[i])
		{
			if(PoolScore[i] > scoremax)
			{
				scoremax = PoolScore[i];
			}
		}
	}
	return scoremax;
}

stock SendPoolPlayersMessage(string[])
{
	foreach (new i : Player)
	{
		if(PlayingPool[i])
		{
			SendClientMessage(i, -1, string);
		}
	}
}

stock AreAllBallsStopped()
{
	new
		Float:x, Float:y, Float:z;
	for(new i; i < 16; i++)
	{
		if(PoolBall[i][bExisting])
		{
			PHY_GetObjectVelocity(PoolBall[i][bObject], x, y, z);
			if(x != 0.0 || y != 0.0)
				return 0;
		}
	}
	return 1;
}

stock RespawnPoolBalls(init = 0)
{
	for(new i; i < 16; i++)
	{
		/*PoolBall[i][bMoving] = 0;
		PoolBall[i][bSpeed] = 0;*/
		//StopObject(PoolBall[i][bObject]);
		DestroyObject(PoolBall[i][bObject]);
		if(PoolBall[i][bExisting])
		{
			PHY_DeleteObject(PoolBall[i][bObject]);
			PoolBall[i][bExisting] = 0;
		}
	}

	if(PoolAimer != -1)
	{
		TogglePlayerControllable(PoolAimer, 1);
		ClearAnimations(PoolAimer);
		ApplyAnimation(PoolAimer, "CARRY", "crry_prtial", 1.0, 0, 0, 0, 0, 0);
		SetCameraBehindPlayer(PoolAimer);
		PoolAimer = -1;
		DestroyObject(AimObject);
		TextDrawHideForPlayer(PoolAimer, PoolTD[0]);
		TextDrawHideForPlayer(PoolAimer, PoolTD[1]);
		TextDrawHideForPlayer(PoolAimer, PoolTD[2]);
		TextDrawHideForPlayer(PoolAimer, PoolTD[3]);
	}

	CreateBalls();
	if(init)
	{
		for(new i; i < sizeof PoolBall; i++)
			InitBall(i);
	}
}

stock CreateBalls()
{
	PoolBall[0][bObject] = CreateObject(3003, 510.11218261719, -84.40771484375, 998.86785888672, 0, 0, 0);
	PoolBall[1][bObject] = CreateObject(3002, 510.10882568359, -85.166389465332, 998.86749267578, 0, 0, 0);
	PoolBall[2][bObject] = CreateObject(3101, 510.14270019531, -85.232612609863, 998.86749267578, 0, 0, 0);
	PoolBall[3][bObject] = CreateObject(2995, 510.0676574707, -85.232200622559, 998.86749267578, 0, 0, 0);
	PoolBall[4][bObject] = CreateObject(2996, 510.18600463867, -85.295257568359, 998.86749267578, 0, 0, 0);
	PoolBall[5][bObject] = CreateObject(3106, 510.11242675781, -85.297294616699, 998.86749267578, 0, 0, 0);
	PoolBall[6][bObject] = CreateObject(3105, 510.03665161133, -85.299163818359, 998.86749267578, 0, 0, 0);
	PoolBall[7][bObject] = CreateObject(3103, 510.22308349609, -85.362342834473, 998.86749267578, 0, 0, 0);
	PoolBall[8][bObject] = CreateObject(3001, 510.14828491211, -85.365989685059, 998.86749267578, 0, 0, 0);
	PoolBall[9][bObject] = CreateObject(3100, 510.07455444336, -85.365234375, 998.86749267578, 0, 0, 0);
	PoolBall[10][bObject] = CreateObject(2997, 510.00054931641, -85.363563537598, 998.86749267578, 0, 0, 0);
	PoolBall[11][bObject] = CreateObject(3000, 510.25915527344, -85.431137084961, 998.86749267578, 0, 0, 0);
	PoolBall[12][bObject] = CreateObject(3102, 510.18399047852, -85.430549621582, 998.86749267578, 0, 0, 0);
	PoolBall[13][bObject] = CreateObject(2999, 510.10900878906, -85.43196105957, 998.86749267578, 0, 0, 0);
	PoolBall[14][bObject] = CreateObject(2998, 510.03570556641, -85.432624816895, 998.86749267578, 0, 0, 0);
	PoolBall[15][bObject] = CreateObject(3104, 509.96197509766, -85.427406311035, 998.86749267578, 0, 0, 0);
}

stock InitBall(i)
{
	PHY_InitObject(PoolBall[i][bObject], 3003, _, _, PHY_MODE_2D);
	PHY_SetObjectFriction(PoolBall[i][bObject], 0.40);
	PHY_RollObject(PoolBall[i][bObject], _, PHY_ROLLING_MODE_ADVANCED);
	PHY_SetObjectWorld(PoolBall[i][bObject], 3);
	PoolBall[i][bExisting] = 1;
}

stock LoadPool()
{
	CreateBalls();

	SetTimer("PoolTimer", 21, 1);

	PHY_SetWallWorld(PHY_CreateWall(509.627 - 0.038, -85.780 - 0.038, 510.598 + 0.038, -85.780 - 0.038), 3);
	PHY_SetWallWorld(PHY_CreateWall(510.598 + 0.038, -85.780 - 0.038, 510.598 + 0.038, -83.907 + 0.038), 3);
	PHY_SetWallWorld(PHY_CreateWall(510.598 + 0.038, -83.907 + 0.038, 509.627 - 0.038, -83.907 + 0.038), 3);
	PHY_SetWallWorld(PHY_CreateWall(509.627 - 0.038, -83.907 + 0.038, 509.627 - 0.038, -85.780 - 0.038), 3);


	PoolTD[0] = TextDrawCreate(505.000000, 260.000000, "~n~~n~");
	TextDrawBackgroundColor(PoolTD[0], 255);
	TextDrawFont(PoolTD[0], 1);
	TextDrawLetterSize(PoolTD[0], 0.500000, 0.439999);
	TextDrawColor(PoolTD[0], -1);
	TextDrawSetOutline(PoolTD[0], 0);
	TextDrawSetProportional(PoolTD[0], 1);
	TextDrawSetShadow(PoolTD[0], 1);
	TextDrawUseBox(PoolTD[0], 1);
	TextDrawBoxColor(PoolTD[0], 255);
	TextDrawTextSize(PoolTD[0], 569.000000, -10.000000);

	PoolTD[1] = TextDrawCreate(506.000000, 261.000000, "~n~~n~");
	TextDrawBackgroundColor(PoolTD[1], 255);
	TextDrawFont(PoolTD[1], 1);
	TextDrawLetterSize(PoolTD[1], 0.500000, 0.300000);
	TextDrawColor(PoolTD[1], -1);
	TextDrawSetOutline(PoolTD[1], 0);
	TextDrawSetProportional(PoolTD[1], 1);
	TextDrawSetShadow(PoolTD[1], 1);
	TextDrawUseBox(PoolTD[1], 1);
	TextDrawBoxColor(PoolTD[1], 911303167);
	TextDrawTextSize(PoolTD[1], 568.000000, 0.000000);

	PoolTD[2] = TextDrawCreate(506.000000, 261.000000, "~n~~n~");
	TextDrawBackgroundColor(PoolTD[2], 255);
	TextDrawFont(PoolTD[2], 1);
	TextDrawLetterSize(PoolTD[2], 0.500000, 0.300000);
	TextDrawColor(PoolTD[2], -1);
	TextDrawSetOutline(PoolTD[2], 0);
	TextDrawSetProportional(PoolTD[2], 1);
	TextDrawSetShadow(PoolTD[2], 1);
	TextDrawUseBox(PoolTD[2], 1);
	TextDrawBoxColor(PoolTD[2], -1949699841);
	TextDrawTextSize(PoolTD[2], 501.000000, 0.000000);

	PoolTD[3] = TextDrawCreate(503.000000, 240.000000, "Power");
	TextDrawBackgroundColor(PoolTD[3], 255);
	TextDrawFont(PoolTD[3], 2);
	TextDrawLetterSize(PoolTD[3], 0.280000, 1.699999);
	TextDrawColor(PoolTD[3], -1);
	TextDrawSetOutline(PoolTD[3], 1);
	TextDrawSetProportional(PoolTD[3], 1);
}

stock GetPoolPlayersCount()
{
	new
		count;
	if(PoolStarted)
	{
		foreach (new i : Player)
		{
			if(PlayingPool[i])
			{
				count++;
			}
		}
	}
	return count;
}

forward PoolTimer();
public PoolTimer()
{
	if(!PoolStarted) return 0;
	if(PoolAimer != -1)
	{
		new
			playerid = PoolAimer,
			keys,
			ud,
			lr;
		GetPlayerKeys(playerid, keys, ud, lr);
		if(!(keys & KEY_SPRINT))
		{
			if(lr)
			{
				new
					Float:X,
					Float:Y,
					Float:Z,
					Float:Xa,
					Float:Ya,
					Float:Za,
					Float:x,
					Float:y,
					Float:newrot,
					Float:dist;
                GetPlayerPos(playerid, X, Y, Z);
				GetObjectPos(PoolBall[0][bObject], Xa, Ya, Za);
				newrot = AimAngle[playerid][0] + (lr > 0 ? 0.9 : -0.9);
				dist = GetPointDistanceToPoint(X, Y, Xa, Ya);
				if(AngleInRangeOfAngle(AimAngle[playerid][1], newrot, 30.0))
				{
					AimAngle[playerid][0] = newrot;
					switch(PoolCamera[playerid])
					{
						case 0:
						{
							GetXYBehindObjectInAngle(PoolBall[0][bObject], newrot, x, y, 0.675);
							SetPlayerCameraPos(playerid, x, y, 998.86785888672+0.28);
							SetPlayerCameraLookAt(playerid, Xa, Ya, Za+0.170);
						}
						case 1:
						{
							SetPlayerCameraPos(playerid, 511.84469604492, -84.831642150879, 1001.4904174805);
							SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
						}
						case 2:
						{
							SetPlayerCameraPos(playerid, 508.7971496582, -84.831642150879, 1001.4904174805);
							SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
						}
					}
					GetXYInFrontOfPos(Xa, Ya, newrot+180, x, y, 0.085);
					SetObjectPos(AimObject, x, y, Za);
					SetObjectRot(AimObject, 7.0, 0, AimAngle[playerid][0]+180);
					GetXYInFrontOfPos(Xa, Ya, newrot+180, X, Y, dist);
					SetPlayerPos(playerid, X, Y, Z);
					SetPlayerFacingAngle(playerid, newrot);
				}
			}
		}
		else
		{
			if(PoolDir)
				PoolPower -= 2.0;
			else
				PoolPower += 2.0;
			if(PoolPower <= 0)
			{
				PoolDir = 0;
				PoolPower = 2.0;
			}
			else if(PoolPower > 100.0)
			{
				PoolDir = 1;
				PoolPower = 98.0;
			}
			TextDrawTextSize(PoolTD[2], 501.0 + ((67.0 * PoolPower)/100.0), 0.0);
			TextDrawShowForPlayer(playerid, PoolTD[2]);
		}
	}

	if(PoolLastShooter != -1 && AreAllBallsStopped())
	{
		SetTimerEx("RestoreCamera", 800, 0, "d", PoolLastShooter);
		PoolLastShooter = -1;
	}
	return 1;
}

forward RestoreCamera(playerid);
public RestoreCamera(playerid)
{
	if(!PoolBall[0][bExisting])
	{
		DestroyObject(PoolBall[0][bObject]);
		PoolBall[0][bObject] = CreateObject(GetBallModel(0) ,510.11218261719, -84.40771484375, 998.86785888672, 0, 0, 0);
		InitBall(0);
	}
	GameTextForPlayer(playerid, " ", 100000, 4);
	if(PoolAimer == playerid) return 0;
	TogglePlayerControllable(playerid, 1);
	return SetCameraBehindPlayer(playerid);
}

forward RestoreWeapon(playerid);
public RestoreWeapon(playerid)
{
	RemovePlayerAttachedObject(playerid, OBJ_SLOT_POOL);
	UsingChalk[playerid] = 0;
	GivePlayerWeapon(playerid, 7, 1);
	ApplyAnimation(playerid, "CARRY", "crry_prtial", 1.0, 0, 0, 0, 0, 0, 1);
	return 1;
}

stock IsBallInHole(i)
{
	if(IsObjectInSphere(PoolBall[i][bObject],509.61123657,-85.79737091,998.86785889,0.0825)) return 1;
	else if(IsObjectInSphere(PoolBall[i][bObject],510.67373657,-84.84423065,998.86785889,0.0825)) return 2;
	else if(IsObjectInSphere(PoolBall[i][bObject],510.61914062,-83.88769531,998.86785889,0.0825)) return 3;
	else if(IsObjectInSphere(PoolBall[i][bObject],509.61077881,-83.89227295,998.86785889,0.0825)) return 4;
	else if(IsObjectInSphere(PoolBall[i][bObject],510.61825562,-85.80107880,998.86785889,0.0825)) return 5;
	else if(IsObjectInSphere(PoolBall[i][bObject],509.55642700,-84.84602356,998.86785889,0.0825)) return 6;
	else return 0;
}

stock GetXYBehindObjectInAngle(objectid, Float:a, &Float:x2, &Float:y2, Float:distance)
{
	new Float:z;
	GetObjectPos(objectid, x2, y2, z);

	x2 += (distance * floatsin(-a+180, degrees));
	y2 += (distance * floatcos(-a+180, degrees));
}

forward PlayPoolSound(soundid);
public PlayPoolSound(soundid)
{
	foreach(new i : Player)
	{
		if(PlayingPool[i])
		{
			PlayerPlaySound(i, soundid, 0, 0, 0);
		}
	}
	return 1;
}

stock IsKeyJustDown(key, newkeys, oldkeys)
{
	if((newkeys & key) && !(oldkeys & key)) return 1;
	return 0;
}

stock IsKeyJustUp(key, newkeys, oldkeys)
{
	if(!(newkeys & key) && (oldkeys & key)) return 1;
	return 0;
}

stock PreloadAnimLib(playerid, animlib[])
{
	ApplyAnimation(playerid,animlib,"null",0.0,0,0,0,0,0);
}

stock AngleInRangeOfAngle(Float:a1, Float:a2, Float:range)
{
	a1 -= a2;
	if((a1 < range) && (a1 > -range)) return true;

	return false;
}


new announcement[MAX_ANNOUNCEMENTS][MAX_CONNMSG_LEN];
new announcement_sec[2];//0 = 多少秒发送一次消息, 1 = 还差多少秒发送
new announcement_count[2];// 0 = 有多少条消息, 1 = 已发送了多少条消息

new serverRules[2048] = "\
1.请勿在服务器内公屏进行辱骂、挑衅行为\n\
2.请勿R人，禁止使用影响其他玩家游戏的CLEO和工具（如吸车等）\n\
3.集体活动时，请勿捣乱，听从管理员指挥\n\
4.禁止在服务器内进行其他服务器的宣传（包括但不限于私聊发送广告）\n\
5.当其他玩家明确表示厌烦你，请勿继续打扰其他玩家游戏\n\
6.sf机场为人多区域，请勿进行轰炸\n\
7.服务器内禁止使用外挂（包括但不限于喷车、吸车、花屏、BOT多开等外挂软件）\n\
8.海盗船为挂机区域，请勿在海盗船内刷车、杀人、轰炸等\n\
";

new MySQL:mariadb;
new connMsg[MAX_CONNMSG][MAX_CONNMSG_LEN];
new pReason[MAX_PLAYERS][MAX_REASON_LEN];
new pNitro[MAX_PLAYERS];
new dyServMode;

enum configservtype {
    Name[MAX_SERVNAME_LEN],
    GMT[MAX_SERVNAME_LEN]
};

new configServ[configservtype];

stock GetServMode() {
	return dyServMode;
}

new GetWeatherNames[][MAX_PLAYER_NAME] = {
    "LA极晴",
    "LA晴",
    "LA极晴雾",
    "LA晴雾",
    "LA阴",
    "SF晴",
    "SF极晴",
    "SF阴",
    "SF雨",
    "SF雾",
    "LV晴",
    "LV热晴",
    "LV阴",
    "乡村极晴",
    "乡村晴",
    "乡村阴",
    "乡村雨",
    "沙漠极晴",
    "沙漠晴",
    "沙漠沙尘暴",
    "水下(绿,雾)"
};

enum spawnposstruct {
    Name[MAX_LENOF_SPNAME],
    Float:X,
    Float:Y,
    Float:Z,
    Float:A
};

new pSPos[MAX_PSPS][spawnposstruct], count_spawnpos;

new PlayerColors[] = {
    0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF,
	0xFF8C13FF,
    0xC715FFFF,
    0x20B2AAFF,
    0x00FF00FF,
    0x6495EDFF,
    0xf0e68cFF,
    0x778899FF,
    0xFF1493FF,
    0xF4A460FF,
    0xEE82EEFF
};

enum configQAtype {
    Timer,
    Now,
    Solution[128]
};

new QAs[configQAtype];

stock configRuntime_QAs() {
    QAs[Now] ++;
    if(QAs[Now] >= QAs[Timer]) {
        QAs[Now] = 0;
        new Cache:result, msg[144], id, q[128];
        result = mysql_query(mariadb, "SELECT value FROM config WHERE keyword='qa'");
		cache_get_row_count(id);
        if(id != 0) {
            id = random(id);
            cache_get_value_name(id, "value", msg, sizeof msg);
            cache_delete(result);
            sscanf(msg, "s[128] s[64]", q, QAs[Solution]);
            format(msg, sizeof msg, "[问答] %s", q);
            SCMToAll(COLOR_GREEN, msg); 
        }
    }
}

stock configQAIsSolution(text[]) {
    if(mk_strcmp(text, QAs[Solution]) == 0 && strlen(QAs[Solution]) != 0 ) {
        format(QAs[Solution], 2, "");
        return 1;
    }
    return 0;
}

stock configKGOBJ(playerid) {
	Streamer_UpdateEx(playerid, 0.0, 0.0, -50000.0);
    Streamer_ToggleItemUpdate(playerid, 0, GetPlayerOBJShowStatus(playerid));
}


stock configConnect(playerid) {
    pNitro[playerid] = 0;
    if(isNameColor(GetName(playerid)) == 1) {
        SCM(playerid, COLOR_RED, "[系统] 你被踢出了服务器, 原因:名字不能带有特殊字符.");
        KickEx(playerid, "错误的名字格式");
    }
}

CMD:rule(playerid, params[]) {
    DIALOG_node(playerid, serverRules, "服务器规则");
    return 1;
}

stock configOPKSC(playerid, newkeys) {
    if(newkeys == 4 || newkeys == 12) {
        if(GetPlayerVehicleSeat(playerid) == 0 && gettime() - pNitro[playerid] > 0) {
            pNitro[playerid] = gettime() + 10;
            AddVehicleComponent(GetPlayerVehicleID(playerid), 1010);
            return 1;
        }
    }
    return 0;
}


stock DIALOG_node(playerid, string[], title[] = "提示") {
    ShowPlayerDialog(playerid, 1, DIALOG_STYLE_MSGBOX, title, string, "关闭", "");
    return 1;
}

public OnPlayerCommandReceived(playerid, cmd[], params[], flags) {
    log_command(GetName(playerid), GetIp(playerid), cmd, params, GetPlayerUID(playerid));
    if(GetPlayerStatus(playerid) == 0) {
        SCM(playerid, COLOR_WHITE, "[系统] 你当前不可以输入任何命令, 原因:你未登录.");
        return 0;
    }
    /*if(isPlayerInPPC(playerid) == 1 && strcmp(cmd, "ppc") != 0) {
        SCM(playerid, COLOR_RED, "[系统] 在碰碰车中不可以使用其他命令.请先输入/ppc exit 退出碰碰车");
        return 0;
    }*/
    if(GetPlayerDMStatus(playerid) == 1 && strcmp(cmd, "dm") != 0) {
        SCM(playerid, COLOR_WHITE, "[系统] 在DM中不可以使用其他命令.");
        return 0;
    }
    if(IsPlayerInJail(playerid) == 1) {
        SCM(playerid, COLOR_WHITE, "[系统] 监狱中不能使用任何命令.");
        return 0;
    }
	new msg[128];
    format(msg, sizeof(msg), "[指令]%s(%d):%s", GetName(playerid), GetIp(playerid), cmd, params, GetPlayerUID(playerid));
    PlayerUserCommand(msg);
    return 1;
}

public OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags) {
    if(result == -1) {
        new telecmd[128];
        format(telecmd, sizeof telecmd, cmd);
        if(telecmd[0] == '/' && telecmd[1]) {
            tele2Slash(playerid, telecmd);
            return 0;
        }
        if(teleSlash(playerid, telecmd) == 1) return 0;
        new msg[128];
        format(msg, sizeof msg, "[系统] 你输入的命令'/%s'不存在, 你可以输入 /help 查看帮助或关注服务器群 寻求帮助", cmd);
        SCM(playerid, COLOR_WHITE, msg);
        return 0;
    }
    return 1;
}

configInit() {
    SetRecyclingStatus(0);
    QAs[Timer] = 1200;
    QAs[Now] = 0;
    ShowPlayerMarkers(PLAYER_MARKERS_MODE_GLOBAL);
    mariadb = mysql_connect("localhost", "root", "kysamp2323", "xky");
    mysql_set_charset("gbk", mariadb);
    new Cache:result, connmsgs, value[128], idx = 0;
    result = mysql_query(mariadb, "SELECT value FROM config WHERE keyword='servname'");
    cache_get_value_name(0, "value", configServ[Name], MAX_SERVNAME_LEN);
    if(!strlen(configServ[Name])) {
        format(configServ[Name], MAX_SERVNAME_LEN, "夜曲傻逼");
    }
    SetServName(configServ[Name]);
	result = mysql_query(mariadb, "SELECT value FROM config WHERE keyword='servmode'");
	cache_get_value_name(0, "value", value, sizeof value);
	dyServMode = (strcmp(value, "development") == 0 ? (1) : (0));
	printf("servmode -> %s", value);
    result = mysql_query(mariadb, "SELECT value FROM config WHERE keyword='game_mode_text'");
    cache_get_value_name(0, "value", configServ[GMT], MAX_SERVNAME_LEN);
    if(!strlen(configServ[GMT])) {
        format(configServ[GMT], MAX_SERVNAME_LEN, "none");
    }
    SetGameModeText(configServ[GMT]);
    result = mysql_query(mariadb, "SELECT value FROM config WHERE keyword='announcement_sec'");
    cache_get_value_name(0, "value", value);
    if(strval(value) == 0) {
        announcement_sec[0] = DEFAULT_ANNOUNCEMENT_SEC;
    } else {
        announcement_sec[0] = strval(value);
    }
    announcement_count[1] = 0;
    printf("config -> 定时发送消息时间为%d秒", announcement_sec[0]);
    result = mysql_query(mariadb, "SELECT value FROM config WHERE keyword='announcement'");
    cache_get_row_count(announcement_count[0]);
    for(new i = 0; i < announcement_count[0]; i ++) {
        cache_get_value_name(i, "value", announcement[i], MAX_CONNMSG_LEN);
    }
    printf("config -> 已加载%d条定时发送消息", announcement_count[0]);
    result = mysql_query(mariadb, "SELECT value FROM config WHERE keyword = 'connmsg'");
    cache_get_row_count(connmsgs);
    if(connmsgs >= MAX_CONNMSG) {
        printErr("ERR:欢迎消息已达到上限(config->MAX_CONNMSG)");
    }
    for(new i = 0; i < connmsgs; i ++) {
        cache_get_value_name(i, "value", connMsg[i]);
    }
    printf("config -> 已加载%d条欢迎消息", connmsgs);
    cache_delete(result);
    result = mysql_query(mariadb, "SELECT value FROM config WHERE keyword = 'spawnpos'");
    cache_get_row_count(count_spawnpos);
    if(count_spawnpos >= MAX_PSPS) {
        printErr("ERR:系统出生点已达到上限(config->MAX_PSPS)");
    }
    for(new i = 0; i < count_spawnpos; i ++) {
        cache_get_value_name(i, "value", value);
        pSPos[i][X] = floatstr(strtok(value, idx));
        pSPos[i][Y] = floatstr(strtok(value, idx));
        pSPos[i][Z] = floatstr(strtok(value, idx));
        pSPos[i][A] = floatstr(strtok(value, idx));
        format(pSPos[i][Name], MAX_LENOF_SPNAME, strrest(value, idx));
        idx = 0;
    }
    printf("config -> 已加载%d个系统出生点", count_spawnpos);
    cache_delete(result);
    UsePlayerPedAnims();
}

stock SetServName(name[]) {
    new msg[144];
    format(msg, sizeof(msg), "hostname %s", name);
    SendRconCommand(msg);
}

stock SetServPassword(password[]) {
    new msg[144];
    format(msg, sizeof(msg), "password %s", (strcmp(password, "none") == 0 ? (" ") : (password)));
    SendRconCommand(msg);
}

stock configPSPRandom(playerid) {
    new r = random(count_spawnpos), msg[128];
    SetPlayerPos(playerid, pSPos[r][X], pSPos[r][Y], pSPos[r][Z]);
    SetPlayerFacingAngle(playerid, pSPos[r][A]);
    format(msg, sizeof msg, "[系统] 出生在 >> %s", pSPos[r][Name]);
    SCM(playerid, COLOR_WHITE, msg);
}

configExit() {
    mysql_close(mariadb);
}

showConnMsg(playerid) {
    for(new i = 0; i < MAX_CONNMSG; i ++) {
        if(strlen(connMsg[i])) {
            SCM(playerid, COLOR_WHITE, connMsg[i]);
        }
    }
}

printErr(str[]) {
    printf(str);
    SendRconCommand("exit");
}

stock isNameHasTeamName(name[]) {
    new tname[MAX_PLAYER_NAME], brackets[2];
    brackets[0] = -1, brackets[1] = -1;
    format(tname, sizeof tname, name);
    for(new i = 0; i < strlen(tname); i ++) {
        if(tname[i] == '[') {
            if(i > brackets[0] && brackets[0] != -1) {
                brackets[0] = -1;
                break;
            }
            brackets[0] = i;
        }
        if(tname[i] == ']') {
            if(i > brackets[1] && brackets[1] != -1) {
                brackets[1] = -1;
                break;
            }
            brackets[1] = i;
        }
    }
    if(brackets[0] != -1 && brackets[1] != -1) {
        strdel(tname, brackets[1], strlen(tname));
        strdel(tname, 0, brackets[0]+1);
    } else {
        format(tname, 2, "");
    }
    return tname;
}

stock ChangeSpecialChar(str[]) {
	/* \ ' */
	new msg[144];
	for(new i = 0; i < strlen(str); i ++) {
		if(str[i] == 92 || str[i] == 39) {
			str[i] = 32;
		}
	}
	format(msg, sizeof msg, str);
	return msg;
}

stock log_ban(from_name[], from_ip[], from_uid, to_name[], to_ip[], to_uid, time, reason[]) {
    new msg[512];
    format(msg, sizeof msg, "INSERT INTO log_ban (fname,fip,fuid,tname,tip,tuid,time,reason,date) VALUES ('%s','%s','%d','%s','%s','%d','%d','%s','%d')", 
    from_name, from_ip, from_uid, to_name, to_ip, to_uid, time, reason, gettime());
    mysql_query(mariadb, msg, false);
}

stock log_mute(from_name[], from_ip[], from_uid, to_name[], to_ip[], to_uid, time, reason[]) {
    new msg[512];
    format(msg, sizeof msg, "INSERT INTO log_mute (fname,fip,fuid,tname,tip,tuid,time,reason,date) VALUES ('%s','%s','%d','%s','%s','%d','%d','%s','%d')", 
    from_name, from_ip, from_uid, to_name, to_ip, to_uid, time, reason, gettime());
    mysql_query(mariadb, msg, false);
}

stock log_jail(from_name[], from_ip[], from_uid, to_name[], to_ip[], to_uid, time, reason[]) {
    new msg[512];
    format(msg, sizeof msg, "INSERT INTO log_jail (fname,fip,fuid,tname,tip,tuid,time,reason,date) VALUES ('%s','%s','%d','%s','%s','%d','%d','%s','%d')", 
    from_name, from_ip, from_uid, to_name, to_ip, to_uid, time, reason, gettime());
    mysql_query(mariadb, msg, false);
}

stock log_kick(from_name[], from_ip[], from_uid, to_name[], to_ip[], to_uid, reason[]) {
    new msg[512];
    format(msg, sizeof msg, "INSERT INTO log_kick (fname,fip,fuid,tname,tip,tuid,reason,date) VALUES ('%s','%s','%d','%s','%s','%d','%s','%d')", 
    from_name, from_ip, from_uid, to_name, to_ip, to_uid, reason, gettime());
    mysql_query(mariadb, msg, false);
}

stock log_changepasswrd(uid, name[], ip[]) {
    new msg[256];
    format(msg, sizeof msg, "INSERT INTO log_changepwd (uid,name,ip,date) VALUES ('%d','%s','%s','%d')", 
    uid, name, ip, gettime());
    mysql_query(mariadb, msg, false);
}

stock log_changename(name[], toname[], ip[], uid) {
    new msg[256];
    format(msg, sizeof msg, "INSERT INTO log_changename (name,toname,ip,uid,date) VALUES ('%s','%s','%s','%d','%d')", 
    name, toname, ip, uid, gettime());
    mysql_query(mariadb, msg, false);
}

stock log_command(name[], ip[], command[], params[], uid) {
    new msg[384];
    format(msg, sizeof msg, "INSERT INTO log_command (name,ip,command,params,uid,date) VALUES ('%s','%s','%s','%s','%d','%d')", 
    name, ip, ChangeSpecialChar(command), ChangeSpecialChar(params), uid, gettime());
    mysql_query(mariadb, msg, false);
	//PlayerUserCommand(msg);
}

stock log_chat(name[], ip[], chat[], uid) {
    new msg[384];
    format(msg, sizeof msg, "INSERT INTO log_chat (name,ip,chat,uid,date) VALUES ('%s','%s','%s','%d','%d')", 
    name, ip, ChangeSpecialChar(chat), uid, gettime());
    mysql_query(mariadb, msg, false);
}

stock log_money(from_name[], from_ip[], from_uid, to_name[], to_ip[], to_uid, money, reason[]) {
    new msg[512];
    format(msg, sizeof msg, "INSERT INTO log_money (fname,fip,fuid,tname,tip,tuid,money,reason,date) VALUES ('%s','%s','%d','%s','%s','%d','%d','%s','%d')", 
    from_name, from_ip, from_uid, to_name, to_ip, to_uid, money, reason, gettime());
    mysql_query(mariadb, msg, false);
}

stock isNameOnline(name[]) {
    new pid = -1;
    for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
        if(IsPlayerConnected(i) == 1) {
            if(strcmp(GetName(i), name) == 0) {
                pid = i;
                break;
            }
        }
    }
    return pid;
}

stock GetMaxPage(counts, pagemax = 20) {
    new maxpage = counts % pagemax;
    if(maxpage == 0) {
        maxpage = counts / pagemax;
    } else {
        maxpage = (counts / pagemax) + 1;
    }
    return maxpage;
}

stock GetIp(playerid) {
    new ip[16];
    GetPlayerIp(playerid, ip, sizeof ip);
    return ip;
}

stock GetName(playerid) {
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof name);
    return name;
}

BanExx(playerid, reason[], delay = 250) {
    format(pReason[playerid], MAX_REASON_LEN, "被踢出了服务器, 原因:%s.", reason);
    SetTimerEx("BanDelay", delay, false, "i", playerid);
}

forward BanDelay(playerid);
public BanDelay(playerid) {
    Ban(playerid);
    return 1;
}

KickEx(playerid, reason[], delay = 250) {
    format(pReason[playerid], MAX_REASON_LEN, "被踢出了服务器, 原因:%s.", reason);
    SetTimerEx("KickDelay", delay, false, "i", playerid);
}

forward KickDelay(playerid);
public KickDelay(playerid) {
    Kick(playerid);
    return 1;
}

Float:GetPlayerAngle(playerid) {
    new Float:angle;
    if(GetPlayerVehicleID(playerid) == 0) {
        GetPlayerFacingAngle(playerid, angle);
    } else {
        GetVehicleZAngle(GetPlayerVehicleID(playerid), angle);
    }
    return angle;
}

stock SetPlayerPosEx(playerid, Float:x, Float:y, Float:z, Float:a = 0.0) {
    new vid = GetPlayerVehicleID(playerid);
    if(vid == 0) {
        SetPlayerPos(playerid, x, y, z);
        if(a != 0) {
            SetPlayerFacingAngle(playerid, a);
        }
    } else {
        SetVehiclePos(vid, x, y, z);
        if(a != 0) {
            SetVehicleZAngle(vid, a);
        }
    }
}

stock SetPlayerVirtualWorldEx(playerid, worldid) {
    if(GetPlayerVehicleSeat(playerid) == 0) {
        SetVehicleVirtualWorld(GetPlayerVehicleID(playerid), worldid);
        SetPlayerVirtualWorld(playerid, worldid);
    } else {
        SetPlayerVirtualWorld(playerid, worldid);
    }
}

new SqlInjectChars[] = {
    32,//%
    34,//"
    35,//#
    37,//%
    39,//'
    42,//*
    43,//+
    44,//,
    45,//-
    47,///
    58,//:
    59,//;
    60,//<
    61,//=
    62,//>
    63,//?
    64,//@
    96,//`
    123,//{
    125,//}
    126,//~
    127,//DEL (delete)
	128// 杨科敏
};

stock sqlInject(str[]) {
    for(new i = 0; i < strlen(str); i ++) {
        for(new k = 0; k < sizeof SqlInjectChars; k ++) {
            if(str[i] == SqlInjectChars[k]) {
                return 0;
            }
        }
    }
    return 1;
}

stock isNameColor(name[]) {
    for(new i = 0; i < strlen(name); i ++) {
        if(name[i] == 123 || name[1] == 125) {
            return 1;
        }
    }
    return 0;
}

stock charCheck(str[]) {
	new tz = 0;
	for(new i = 0; i < strlen(str); i++) {
  		if(str[i]!=0) {
	        if(str[i] >= 48 && str[i] <= 57 || str[i] >= 65 && str[i] <= 90 || str[i] >= 97 && str[i] <= 122) {
			    tz = 1;
			} else {
		        tz = 0;
		        i = strlen(str)+1;
		    }
		}
    }
	return tz;
}

stock UnixtoTIme(unixtime) {
    new years = 0, months = 0, days = 0, hours = 0, minutes = 0, seconds = 0, msg[12];
    if(unixtime == 0) {
        format(msg, sizeof msg, "未记录");
        return msg;
    }
    unixtime = gettime() - unixtime;
    while(unixtime != 0) {
        if(unixtime > 32136000) {
            years ++;
            unixtime = unixtime - 32136000;
        } else if(months > 2592000) {
            months ++;
            unixtime = unixtime - 2592000;
        } else if(unixtime > 86400) {
            days ++;
            unixtime = unixtime - 86400;
        } else if(unixtime > 3600) {
            hours ++;
            unixtime = unixtime - 3600;
        } else if(unixtime > 60) {
            minutes ++;
            unixtime = unixtime - 60;
        } else {
            seconds ++;
            unixtime = unixtime - 1;
        }
    }
    if(years != 0) {
        format(msg, sizeof msg, "%d年前", years);
        return msg;
    }
    if(months != 0) {
        format(msg, sizeof msg, "%d月前", months);
        return msg;
    }
    if(days != 0) {
        format(msg, sizeof msg, "%d天前", days);
        return msg;
    }
    if(hours != 0) {
        format(msg, sizeof msg, "%d小时前", hours);
        return msg;
    }
    if(minutes != 0) {
        format(msg, sizeof msg, "%d分钟前", minutes);
        return msg;
    }
    format(msg, sizeof msg, "%d秒前", seconds);
    return msg;
}

new GetVehicleModelName[212][] = {
   "Landstalker",  "Bravura",  "Buffalo", "Linerunner", "Perennial", "Sentinel",
   "Dumper",  "Firetruck" ,  "Trashmaster" ,  "Stretch",  "Manana",  "Infernus",
   "Voodoo", "Pony",  "Mule", "Cheetah", "Ambulance",  "Leviathan",  "Moonbeam",
   "Esperanto", "Taxi",  "Washington",  "Bobcat",  "Mr Whoopee", "BF Injection",
   "Hunter", "Premier",  "Enforcer",  "Securicar", "Banshee", "Predator", "Bus",
   "Rhino",  "Barracks",  "Hotknife",  "Trailer",  "Previon", "Coach", "Cabbie",
   "Stallion", "Rumpo", "RC Bandit",  "Romero", "Packer", "Monster",  "Admiral",
   "Squalo", "Seasparrow", "Pizzaboy", "Tram", "Trailer",  "Turismo", "Speeder",
   "Reefer", "Tropic", "Flatbed","Yankee", "Caddy", "Solair","Berkley's RC Van",
   "Skimmer", "PCJ-600", "Faggio", "Freeway", "RC Baron","RC Raider","Glendale",
   "Oceanic", "Sanchez", "Sparrow",  "Patriot", "Quad",  "Coastguard", "Dinghy",
   "Hermes", "Sabre", "Rustler", "ZR-350", "Walton",  "Regina",  "Comet", "BMX",
   "Burrito", "Camper", "Marquis", "Baggage", "Dozer","Maverick","News Chopper",
   "Rancher", "FBI Rancher", "Virgo", "Greenwood","Jetmax","Hotring","Sandking",
   "Blista Compact", "Police Maverick", "Boxville", "Benson","Mesa","RC Goblin",
   "Hotring Racer", "Hotring Racer", "Bloodring Banger", "Rancher",  "Super GT",
   "Elegant", "Journey", "Bike", "Mountain Bike", "Beagle", "Cropdust", "Stunt",
   "Tanker", "RoadTrain", "Nebula", "Majestic", "Buccaneer", "Shamal",  "Hydra",
   "FCR-900","NRG-500","HPV1000","Cement Truck","Tow Truck","Fortune","Cadrona",
   "FBI Truck", "Willard", "Forklift","Tractor","Combine","Feltzer","Remington",
   "Slamvan", "Blade", "Freight", "Streak","Vortex","Vincent","Bullet","Clover",
   "Sadler",  "Firetruck", "Hustler", "Intruder", "Primo", "Cargobob",  "Tampa",
   "Sunrise", "Merit",  "Utility Truck",  "Nevada", "Yosemite", "Windsor",  "Monster",
   "Monster","Uranus","Jester","Sultan","Stratum","Elegy","Raindance","RCTiger",
   "Flash","Tahoma","Savanna", "Bandito", "Freight", "Trailer", "Kart", "Mower",
   "Dune", "Sweeper", "Broadway", "Tornado", "AT-400",  "DFT-30", "Huntley",
   "Stafford", "BF-400", "Newsvan","Tug","Trailer","Emperor","Wayfarer","Euros",
   "Hotdog", "Club", "Trailer", "Trailer","Andromada","Dodo","RC Cam", "Launch",
   "Police Car (LSPD)", "Police Car (SFPD)","Police Car (LVPD)","Police Ranger",
   "Picador",   "S.W.A.T. Van",  "Alpha",   "Phoenix",   "Glendale",   "Sadler",
   "Luggage Trailer","Luggage Trailer","Stair Trailer", "Boxville", "Farm Plow",
   "Utility Trailer"
};

new GetWeaponNames[][24] = {
	"空","拳扣","高尔夫球杆","警棍","小刀","棒球棍","铁锹",
	"台球杆","日本刀","电锯","紫色假阴茎","假阴茎","振动器","振动器","花","甘蔗","手雷",
	"催泪瓦斯","燃烧瓶","","","","九妹妹手枪",
	"消音九妹妹手枪","沙鹰","散弹","双管散弹","连散枪","乌兹冲锋枪","MP5","AK47","M4",
	"Tec冲锋枪","步枪","狙击枪","火箭筒","热导追踪",
	"火焰喷射器","加特林","炸药包","遥控器","喷漆","灭火器","相机","夜视镜","夜视镜","降落伞"
};

stock Float:GetVehicleSpeed(vehicleid) {
	new Float:x, Float:y, Float:z;
	GetVehicleVelocity(vehicleid, x, y, z);
	return floatpower((x*x)+(y*y)+(z*z), 0.5) * 180;
}

stock strtok(const string[], &index)
{
	new length = strlen(string);
	while ((index < length) && (string[index] <= ' '))
	{
		index++;
	}

	new offset = index;
	new result[20];
	while ((index < length) && (string[index] > ' ') && ((index - offset) < (sizeof(result) - 1)))
	{
		result[index - offset] = string[index];
		index++;
	}
	result[index - offset] = EOS;
	return result;
}

//------------------------------------------------

stock strrest(const string[], &index)
{
	new length = strlen(string);
	while ((index < length) && (string[index] <= ' '))
	{
		index++;
	}
	new offset = index;
	new result[128];
	while ((index < length) && ((index - offset) < (sizeof(result) - 1)))
	{
		result[index - offset] = string[index];
		index++;
	}
	result[index - offset] = EOS;
	return result;
}
