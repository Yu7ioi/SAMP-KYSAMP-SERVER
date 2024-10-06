new dyrecycling=0;

stock SetRecyclingStatus(typeid) {
    dyrecycling = typeid;
}

stock GetRecyclingStatus() {
    return dyrecycling;
}

stock RecyclingResources(unixnow) {
    SetRecyclingStatus(1);
    new query[128], Cache:result, unixbefore = unixnow - (90 * 86400), counts, uid, name[MAX_PLAYER_NAME];
    format(query, sizeof query, "SELECT uid,name FROM account WHERE lt_login < %d", unixbefore);
    result = mysql_query(mariadb, query);
    cache_get_row_count(counts);
    for(new i = 0; i < counts; i ++) {
        cache_get_value_name_int(i, "uid", uid);
        cache_get_value_name(i, "name", name, MAX_PLAYER_NAME);
        PHouse_Recycling(uid, name);
    }
    cache_delete(result);
    unixbefore = unixnow - (150 * 86400);
    format(query, sizeof query, "SELECT uid,name FROM account WHERE lt_login < %d", unixbefore);
    result = mysql_query(mariadb, query);
    cache_get_row_count(counts);
    for(new i = 0; i < counts; i ++) {
        cache_get_value_name_int(i, "uid", uid);
        cache_get_value_name(i, "name", name, MAX_PLAYER_NAME);
        ACVeh_Recycling(uid, name);
    }
    cache_delete(result);
    SetRecyclingStatus(0);
}