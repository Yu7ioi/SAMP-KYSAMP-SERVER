#define BET_MONEY	3000
enum betinfo
{
	Money,
	Total,
	Type
};
new Bet[betinfo];
new pBet[MAX_PLAYERS];//0不赌 1小 2大
stock Bet_OnPlayerDisconnect(playerid)
{
	pBet[playerid] = 0;
}
CMD:rebet(playerid, cmdtext[])
{
	if(GetPlayerAccess(playerid, "admin") == 0) return 1;
	if(GetPlayerAccess(playerid, "gm_5") == 0) return 1;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		Bet[Money]=0;
		Bet[Type]=0;
		Bet[Total]=0;
	    pBet[i] = 0;
	}
	new msg[256];
	format(msg, sizeof msg, "管理员%s重置了赌局", GetName(playerid));
	SCMToAll(COLOR_RED, msg);
	return 1;
}
CMD:bet(playerid, cmdtext[])
{
	new tmp[128],msg[128], idx;
	tmp = strtok(cmdtext, idx);
	if(!strlen(tmp))
	{
		SCM(playerid,-1, "赌博 * /bet 1为小 2为大.(需要3000游戏币哟)");
		return 1;
	}
	if(mk_strcmp(tmp, "1", true) == 0)
	{
	    if(userInfo[playerid][OnlineTime]<500)
	    {
  			SCM(playerid, -1, "赌博 *需要500的时间分才能参与赌博.");
			return 1;
		}
	    if(GetMoney(playerid)<BET_MONEY)
	    {
   			SCM(playerid, -1, "赌博 *需要3000的游戏币才能参与赌博.");
			return 1;
		}
		if(pBet[playerid]!=0)
		{
			SCM(playerid, -1, "赌博 *你已经参与过了,耐心等待开局吧.");
			return 1;
		}
		if(Bet[Total]==0)
		{
			format(msg, sizeof msg,"赌博  * 玩家{0088FF}%s{CC66FF}开启了一场赌局(1分钟后开局),他选择的为小,各位玩家可以输入/bet参与赌博", GetName(playerid));
			SCMToAll(COLOR_RED, msg);
			GiveMoney(playerid,-BET_MONEY);
	    	pBet[playerid]=1;
			SCM(playerid, -1, "赌博 * 你选择了小.");
			Bet[Money]=Bet[Money]+BET_MONEY;
			Bet[Total]++;
			SetTimer("makebet",60000, false);
			return 1;
		}
		GiveMoney(playerid,-BET_MONEY);
	    pBet[playerid]=1;
		SCM(playerid, -1, "赌博 * 你选择了小.");
		Bet[Money]=Bet[Money]+BET_MONEY;
		Bet[Total]++;
		return 1;
	}
	if(mk_strcmp(tmp, "2", true) == 0)
	{
 		if(userInfo[playerid][OnlineTime]<500)
	    {
  			SCM(playerid, -1, "赌博 *需要500的时间分才能参与赌博.");
			return 1;
		}
	    if(GetMoney(playerid)<BET_MONEY)
	    {
   			SCM(playerid, -1, "赌博 *需要3000的游戏币才能参与赌博.");
			return 1;
		}
		if(pBet[playerid]!=0)
		{
			SCM(playerid, -1, "赌博 *你已经参与过了,耐心等待开局吧.");
			return 1;
		}
		if(Bet[Total]==0)
		{
			format(msg, sizeof msg,"赌博  * 玩家{0088FF}%s{CC66FF}开启了一场赌局(1分钟后开局),他选择的为大,各位玩家可以输入/bet参与赌博", GetName(playerid));
			SCMToAll(COLOR_RED, msg);
			GiveMoney(playerid,-BET_MONEY);
	    	pBet[playerid]=2;
			SCM(playerid, -1, "赌博 * 你选择了大.");
			Bet[Money]=Bet[Money]+BET_MONEY;
			Bet[Total]++;
			SetTimer("makebet",60000, false);
			return 1;
		}
		GiveMoney(playerid,-BET_MONEY);
	    pBet[playerid]=2;
		SCM(playerid, -1, "赌博 * 你选择了大.");
		Bet[Money]=Bet[Money]+BET_MONEY;
		Bet[Total]++;
		return 1;
	}
	return 1;
}
stock GetWinPlayer()
{
	new sz=random(7);
	if(sz<=3){Bet[Type]=1;}
	if(sz>3){Bet[Type]=2;}
	printf("结果 %d",Bet[Type]);
	new winplayer=0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(IsPlayerConnected(i) != 0)
		{
		    if(pBet[i]!=0)
		    {
		        if(pBet[i]==Bet[Type])
		        {
        			winplayer++;
		        }
			}
		}
	}
	return winplayer;
}
stock SendBetRerult(msg[])
{
	new str[48];
	if(Bet[Type] == 1) { str = "本次赌博结果:[{F81414}小{FFFFFF}]"; }
	if(Bet[Type] == 2) { str = "本次赌博结果:[{37DB45}大{FFFFFF}]"; }
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(IsPlayerConnected(i) != 0)
		{
		    if(pBet[i]!=0)
		    {
                ShowPlayerDialog(i,1,DIALOG_STYLE_MSGBOX,str,msg,"知道了","");
                pBet[i]=0;
			}
		}
	}
	return 1;
}
forward makebet();
public makebet()
{
	new nm[1024],str[1024];
	new winmoney,wp;
	wp=GetWinPlayer();
	if(wp!=0)
	{
	winmoney=Bet[Money]/wp;
	}
	new msg[256];
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(IsPlayerConnected(i) != 0)
		{
		    if(pBet[i]!=0)
		    {
		        if(pBet[i]==Bet[Type])
		        {
        			format(str,128,"{FFFFFF}获利玩家:[{FF0000}%s{FFFFFF}]\t获利:[{00FF00}%d{FFFFFF}]\n",GetName(i),winmoney);
					strcat(nm,str);
					GiveMoney(i,winmoney);
					format(msg,256,"赌博 * 你从本次赌博中获利了$%d",winmoney);
					SCM(i, -1,msg);
		        }
			}
		}
	}
	format(str,128,"{FFFFFF}参与玩家总数:[{FF0000}%d{FFFFFF}]\t奖金池:[{00FF00}%d{FFFFFF}]",Bet[Total],Bet[Money]);
	strcat(nm,str);
	SendBetRerult(nm);
	Bet[Money]=0;
	Bet[Type]=0;
	Bet[Total]=0;
	SCMToAll(COLOR_RED,"本场赌局结束!可以继续使用/bet进入下一轮");
	return 1;
}
