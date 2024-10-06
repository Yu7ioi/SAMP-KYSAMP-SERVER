/**
    背包系统 by DeepDC
**/

#define MAX_ITEM_NAME_LEN              (64) // 物品名称最大长度
#define ITEM_TYPE_MATERIAL             (0) // 物品种类：消耗品
#define ITEM_TYPE_OBJ                  (1) // 物品种类：家具
#define ITEM_TYPE_ATTIRE               (2) // 物品种类：装扮OBJ
#define ITEM_TYPE_ACOBJ                (3) // 物品种类：爱车OBJ

enum storageStruct
{
    id,
    itemId,
    itemOwnerUid,
    itemCount,
    itemType
};

enum itemStruct
{
    itemName[MAX_ITEM_NAME_LEN],
    itemId, // 物品id
    itemType, // 物品种类
    itemUsability, // 是否可以直接使用
    itemObjId // 物品为OBJ时的id
};

new itemInfo[][itemStruct] =
{
    {"鱼饵", 0, 1, 0, -1},
    {"漂流瓶", 1, 1, 1, -1},
};

new storageInfo[MAX_PLAYERS][storageStruct]; // 存储当前在线玩家的背包信息

/**
*   初始化背包系统(当玩家进入游戏时)
*/
stock initPlayerStorage(playerid)
{
    new Cache:result;
    new query[128];
    new counts;
    new storageItemCount; // 数据表项目数量
    format(query, sizeof query, "SELECT * FROM storage where item_owner_uid = '%d'", GetPlayerUID(playerid));
    result = mysql_query(mariadb, query);
    cache_get_row_count(storageItemCount);
    for (new i = 0; i < storageItemCount; i++)
    {
        new id;
        new itemId;
        new itemOwnerUid;
        new itemCount;
        new itemType;

        cache_get_value_name_int(i, "id", id);
        cache_get_value_name_int(i, "item_id", itemId);
        cache_get_value_name_int(i, "item_owner_uid", itemOwnerUid);
        cache_get_value_name_int(i, "item_count", itemCount);
        cache_get_value_name_int(i, "item_type", itemType);

        counts ++;
    }
    cache_delete(result);
    printf("玩家 %s 背包系统加载成功", GetName(playerid));
}