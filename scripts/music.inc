//new bool:ServerPlayingMusic = true;

forward NextOne();
public NextOne()
{
    ServerPlayingMusic = false;
}

stock Warning(playerid,string[] = "")
	{
	new string2[120];
	format(string2,sizeof(string2),"{ff0000}%s",string);
	return ShowPlayerDialog(playerid,999,DIALOG_STYLE_MSGBOX,"{ff0000}警告",string2,"Okay","");
	}

CMD:music(playerid, params[])
{
    if(ServerPlayingMusic) return SendClientMessage(playerid,COLOR_YELLOW,"[音乐系统]目前服务器正在播放音乐，请等待上首歌放完再点下一首歌");
    if (sscanf(params, "s[256]", params[1])) return SendClientMessage(playerid, COLOR_YELLOW, "全服点歌: /music [音乐链接] - 33300$点一次 顶一次歌涨一次你们看着办");
    if (GetMoney(playerid) < 30000) return SendClientMessage(playerid, COLOR_BLUE, "全服点歌: 你没有足够的金钱！");

    new string[144];

    for (new i; i < 101; i++)
    {
        new Float:Pos[3];
        GetPlayerPos(i, Pos[0], Pos[1], Pos[2]);
        PlayAudioStreamForPlayer(i, params[1], Pos[0], Pos[1], Pos[2], 1, 0);
    }
    format(string, sizeof(string), "本服尊敬的玩家'%s[%i]' 花费33300巨资为大家点了一首歌！", GetName(playerid), playerid);
    SCMToAll(COLOR_YELLOW, string);
    GiveMoney(playerid, -33300);
	ServerPlayingMusic = true;
	SetTimer("NextOne", 140000, false);
	//PlayerUserCommand(string);
    return 1;
}

CMD:stopsong(playerid, params[]) {
    StopAudioStreamForPlayer(playerid);
    SCM(playerid, COLOR_RED, "[系统] 你停止了音乐播放!");
	return 1;
}

stock PlayerMusicForAll(url[]) {
	for(new i = 0, all = GetPlayerPoolSize()+1; i < all; i ++) {
		if(IsPlayerConnected(i) == 1) {
			PlayAudioStreamForPlayer(i, url);
		}
	}
}



