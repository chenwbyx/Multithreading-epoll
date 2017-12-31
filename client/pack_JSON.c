#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

void create_udp_package(char * pack, char *ip, char *mac)
{
    pack[0]='\0';
    strcat(pack,"{\"IP\":\"");
    strcat(pack,ip);
    strcat(pack,"\",\"MAC\":\"");
    strcat(pack,mac);
    strcat(pack,"\"}");
}
void dis_udp_package(char * pack, char *ip, char *mac)
{
    cJSON *json , *json_value , *json_timestamp;
    json = cJSON_Parse(pack);
    if (!json)
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
    else
    {
        json_value = cJSON_GetObjectItem( json , "MAC");
        if( json_value->type == cJSON_String )
            strcpy(mac,json_value->valuestring);
        json_timestamp = cJSON_GetObjectItem( json , "IP");
        if( json_timestamp->type == cJSON_String )
            strcpy(ip,json_timestamp->valuestring);
        cJSON_Delete(json);
    }
}
