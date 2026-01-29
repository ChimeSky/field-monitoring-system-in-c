#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10
#define NAME_MAX 30
#define TIME_MAX 20
#define DESC_MAX 100

typedef struct Field {
    int id;
    char name[NAME_MAX];
    char manager[NAME_MAX];
    int sensor_count;
} Field;

typedef struct SensorRecord {
    int field_id;
    char timestamp[TIME_MAX];
    float temperature;
    float humidity;
    int is_abnormal;
    struct SensorRecord* next;
} SensorRecord;

typedef struct Alert {
    int field_id;
    char alert_time[TIME_MAX];
    char alert_type[NAME_MAX];
    char description[DESC_MAX];
    int severity;               // 1-低，2-中，3-高
    struct Alert* next;
} Alert;

typedef struct MonitorSystem {
    Field** fields;              // 核心要求：指向指针数组的二重指针
    int field_count;             // 当前已有的田块数量
    int field_capacity;          // 当前动态数组的最大容量
    int next_field_id;           // 下一个可用的田块ID（用于自动分配）

    SensorRecord* record_head;
    Alert* alert_head;
} MonitorSystem;

#endif