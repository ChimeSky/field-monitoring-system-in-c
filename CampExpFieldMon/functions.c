#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "functions.h"
#include "model.h"
#include "UI.h"

MonitorSystem* g_sys = NULL;    // 全局变量


///////////////////////
static void get_current_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    strftime(buffer, size, "%Y-%m-%d %H:%M", &timeinfo);    //YYYY-MM-DD HH:MM
}

static int check_is_abnormal(float temp, float hum) {   //判断异常
    if (temp < 10.0f || temp > 35.0f) return 1;
    if (hum < 40.0f || hum > 85.0f) return 1;
    return 0;
}


//////////////////////////////   系统管理
MonitorSystem* create_monitor_system(void) {
    MonitorSystem* sys = (MonitorSystem*)malloc(sizeof(MonitorSystem));
    if (sys == NULL) return NULL;

    sys->fields = (Field**)malloc(sizeof(Field*) * INITIAL_CAPACITY);   //指针数组
    if (sys->fields == NULL) {
        free(sys);
        return NULL;
    }

    for (int i = 0; i < INITIAL_CAPACITY; i++) {    //初始化
        sys->fields[i] = NULL;
    }

    sys->field_count = 0;
    sys->field_capacity = INITIAL_CAPACITY;
    sys->next_field_id = 1;
    sys->record_head = NULL;
    sys->alert_head = NULL;

    return sys;
}

void destroy_monitor_system(MonitorSystem* sys) {
    if (sys == NULL) return;

	for (int i = 0; i < sys->field_count; i++) {    //释放field指针
        if (sys->fields[i] != NULL) {
            free(sys->fields[i]);
        }
    }
    free(sys->fields);

    SensorRecord* curr_rec = sys->record_head;  //释放链表
    while (curr_rec != NULL) {
        SensorRecord* temp = curr_rec;
        curr_rec = curr_rec->next;
        free(temp);
    }

    Alert* curr_alert = sys->alert_head;
    while (curr_alert != NULL) {
        Alert* temp = curr_alert;
        curr_alert = curr_alert->next;
        free(temp);
    }

	free(sys);      //释放
}

void expand_field_array(MonitorSystem* sys) {   //扩容
    if (sys == NULL) return;

    int new_capacity = sys->field_capacity * 2;

    Field** new_fields = (Field**)malloc(sizeof(Field*) * new_capacity);
    if (new_fields == NULL) return;

    for (int i = 0; i < sys->field_count; i++) {    // 复制旧数组指针
        new_fields[i] = sys->fields[i];
    }

    for (int i = sys->field_count; i < new_capacity; i++) {     //初始化
        new_fields[i] = NULL;
    }

	free(sys->fields);  // 释放旧数组

    sys->fields = new_fields;
    sys->field_capacity = new_capacity;
}


//////////////////////////////////////////  实验田管理
int add_field(MonitorSystem* sys, const char* name, const char* manager) {
    if (sys == NULL || name == NULL || manager == NULL) return -1;

    if (sys->field_count >= sys->field_capacity) {      //检查扩容
        expand_field_array(sys);
    }

    Field* new_field = (Field*)malloc(sizeof(Field));
    if (new_field == NULL) return -1;

    new_field->id = sys->next_field_id++;

    strncpy_s(new_field->name, NAME_MAX, name, _TRUNCATE);
    strncpy_s(new_field->manager, NAME_MAX, manager, _TRUNCATE);
    new_field->sensor_count = 0;

    sys->fields[sys->field_count] = new_field;      //存入指针数组
    sys->field_count++;

    return new_field->id;
}

Field* find_field_by_id(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return NULL;
    
    for (int i = 0; i < sys->field_count; i++) {
        if (sys->fields[i]->id == field_id) {
            return sys->fields[i];
        }
    }
    return NULL;
}

static void get_latest_data(MonitorSystem* sys, int field_id, float* temp, float* hum, int* has_data, int* is_abn) {    //由ID获取最新的温湿度记录
    SensorRecord* curr = sys->record_head;
    *has_data = 0;
    *is_abn = 0;

    while (curr != NULL) {  //插入链表头
        if (curr->field_id == field_id) {
            *temp = curr->temperature;
            *hum = curr->humidity;
            *is_abn = curr->is_abnormal;
            *has_data = 1;
            return;
        }
        curr = curr->next;
    }
}

void display_all_fields(MonitorSystem* sys) {
    if (sys == NULL) return;

    printf("%-3s %-20s %-10s %-11s %-11s %-11s %-11s\n", "ID", "名称", "负责人", "传感器数", "最新温度", "最新湿度", "状态");
    printf("========================================================================================\n");

    if (sys->field_count == 0) {
        printf("                       / / / / / 暂无田块数据 / / / / /              \n");
    }
    else {
        for (int i = 0; i < sys->field_count; i++) {
            Field* f = sys->fields[i];
            float temp = 0, hum = 0;
            int has_data = 0, is_abn = 0;

            get_latest_data(sys, f->id, &temp, &hum, &has_data, &is_abn);

            char temp_str[20] = "-";
            char hum_str[20] = "-";
            char status_str[20] = "正常";

            if (has_data) {
                snprintf(temp_str, 20, "%.1f℃", temp);
                snprintf(hum_str, 20, "%.0f%%", hum);
                if (is_abn) strcpy_s(status_str, 20, "异常");
            }
            else {
                strcpy_s(status_str, 20, "无数据");
            }

            printf("%-3d %-20s %-10s     %-3d      %-11s %-11s %-8s\n",
                f->id, f->name, f->manager, f->sensor_count, temp_str, hum_str, status_str);
        }
    }
    printf("========================================================================================\n");
    printf("共 %d 个田块\n", sys->field_count);
}

int delete_field(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return 0;

    int index = -1;
    for (int i = 0; i < sys->field_count; i++) {
        if (sys->fields[i]->id == field_id) {
            index = i;
            break;
        }
    }
    if (index == -1) return 0;  //未找到

    SensorRecord* prev_rec = NULL;  //删除传感器记录
    SensorRecord* curr_rec = sys->record_head;
    while (curr_rec != NULL) {
        if (curr_rec->field_id == field_id) {
            SensorRecord* to_free = curr_rec;
            if (prev_rec == NULL) {
                sys->record_head = curr_rec->next;
                curr_rec = sys->record_head;
            }
            else {
                prev_rec->next = curr_rec->next;
                curr_rec = prev_rec->next;
            }
            free(to_free);
        }
        else {
            prev_rec = curr_rec;
            curr_rec = curr_rec->next;
        }
    }

    Alert* prev_alert = NULL;   //删除告警记录
    Alert* curr_alert = sys->alert_head;
    while (curr_alert != NULL) {
        if (curr_alert->field_id == field_id) {
            Alert* to_free = curr_alert;
            if (prev_alert == NULL) {
                sys->alert_head = curr_alert->next;
                curr_alert = sys->alert_head;
            }
            else {
                prev_alert->next = curr_alert->next;
                curr_alert = prev_alert->next;
            }
            free(to_free);
        }
        else {
            prev_alert = curr_alert;
            curr_alert = curr_alert->next;
        }
    }

    free(sys->fields[index]);   //删除田块并移动数组
    for (int i = index; i < sys->field_count - 1; i++) {
        sys->fields[i] = sys->fields[i + 1];
    }
    sys->field_count--;
    return 1;
}

void search_fields_by_name(MonitorSystem* sys, const char* keyword) {
    if (sys == NULL) return;
    printf("\n搜索关键词 \"%s\" 结果:\n", keyword);
    printf("%-3s %-20s %-10s %-8s\n", "ID", "名称", "负责人", "传感器数");
    printf("------------------------------------------------------\n");
    int found = 0;
    for (int i = 0; i < sys->field_count; i++) {
        if (strstr(sys->fields[i]->name, keyword) != NULL) {
            printf("%-3d %-20s %-10s     %-3d   \n",
                sys->fields[i]->id, sys->fields[i]->name, sys->fields[i]->manager, sys->fields[i]->sensor_count);
            found++;
        }
    }
    printf("------------------------------------------------------\n");
    if (found == 0) printf("未找到匹配田块……\n");
}

void search_fields_by_manager(MonitorSystem* sys, const char* manager_name) {
    if (sys == NULL) return;
    printf("\n搜索负责人 \"%s\" 结果:\n", manager_name);
    printf("%-3s %-20s %-10s %-8s\n", "ID", "名称", "负责人", "传感器数");
    printf("------------------------------------------------------\n");
    int found = 0;
    for (int i = 0; i < sys->field_count; i++) {
        if (strcmp(sys->fields[i]->manager, manager_name) == 0) {
            printf("%-3d %-20s %-10s     %-3d   \n",
                sys->fields[i]->id, sys->fields[i]->name, sys->fields[i]->manager, sys->fields[i]->sensor_count);
            found++;
        }
    }
    printf("------------------------------------------------------\n");
    if (found == 0) printf("未找到匹配田块……\n");
}


////////////////////////////////////    传感器数据管理
int add_sensor_record(MonitorSystem* sys, int field_id, float temperature, float humidity) {
    if (sys == NULL) return 0;

    Field* f = find_field_by_id(sys, field_id);
    if (f == NULL) return 0;

    SensorRecord* new_rec = (SensorRecord*)malloc(sizeof(SensorRecord));
    if (new_rec == NULL) return 0;

    new_rec->field_id = field_id;
    new_rec->temperature = temperature;
    new_rec->humidity = humidity;
    get_current_timestamp(new_rec->timestamp, TIME_MAX);
    new_rec->is_abnormal = check_is_abnormal(temperature, humidity);

    new_rec->next = sys->record_head;
    sys->record_head = new_rec;

	f->sensor_count++;  //计数++
    return 1;
}

void display_latest_records(MonitorSystem* sys, int field_id, int count) {
    if (sys == NULL) return;

    printf("\n田块 ID:%d 最新 %d 条记录:\n", field_id, count);
    printf(" %-19s   %-9s  %-9s  %-12s \n", "时间", "温度(℃ )", "湿度(%)", "状态");
    printf("----------------------------------------------------------\n");

    SensorRecord* curr = sys->record_head;
    int shown = 0;
    while (curr != NULL && shown < count) {
        if (curr->field_id == field_id) {
            char status[20];
            if (curr->is_abnormal) strcpy_s(status, 20, "异常");
            else strcpy_s(status, 20, "正常");

            printf(" %-19s   %5.1f      %3.0f     %-12s \n",
                curr->timestamp, curr->temperature, curr->humidity, status);
            shown++;
        }
        curr = curr->next;
    }
    printf("----------------------------------------------------------\n");
}

void display_field_all_records(MonitorSystem* sys, int field_id) {

    display_latest_records(sys, field_id, 99999);
} 

void delete_old_records(MonitorSystem* sys, int days) {
    if (sys == NULL) return;

    time_t now = time(NULL);    //截止时间戳
    time_t limit = now - (long long)days * 24 * 3600;

    SensorRecord* prev = NULL;
    SensorRecord* curr = sys->record_head;
    int count = 0;

    while (curr != NULL) {
        struct tm tm_t = { 0 };
        int year, month, day, hour, minute;
        sscanf_s(curr->timestamp, "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute);
        tm_t.tm_year = year - 1900;
        tm_t.tm_mon = month - 1;
        tm_t.tm_mday = day;
        tm_t.tm_hour = hour;
        tm_t.tm_min = minute;
        time_t rec_time = mktime(&tm_t);

        if (rec_time < limit) {     //删除
            SensorRecord* to_free = curr;

            Field* f = find_field_by_id(sys, curr->field_id);   //减少田块的计数
            if (f && f->sensor_count > 0) f->sensor_count--;

            if (prev == NULL) {
                sys->record_head = curr->next;
                curr = sys->record_head;
            }
            else {
                prev->next = curr->next;
                curr = prev->next;
            }
            free(to_free);
            count++;
        }
        else {
            prev = curr;
            curr = curr->next;
        }
    }
    printf("已删除 %d 条过期记录\n", count);
}

int get_record_count(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return 0;
    Field* f = find_field_by_id(sys, field_id);
    return f ? f->sensor_count : 0;
}

float calculate_avg_temperature(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return 0.0f;
    float sum = 0;
    int count = 0;
    SensorRecord* curr = sys->record_head;
    while (curr != NULL) {
        if (curr->field_id == field_id) {
            sum += curr->temperature;
            count++;
        }
        curr = curr->next;
    }
    return count > 0 ? (sum / count) : 0.0f;
}


/////////////////////////////////////////     数据分析
void find_abnormal_records(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return;
    printf("\n田块 ID:%d 异常记录详情:\n\n", field_id);
    printf("%-20s %-35s\n", "时间", "原因");
    printf("--------------------------------------------------------------------\n");

    SensorRecord* curr = sys->record_head;
    int found = 0;
    while (curr != NULL) {
        if (curr->field_id == field_id && curr->is_abnormal) {
            char reason[50] = "";
            if (curr->temperature < 10) strcat_s(reason, 50, "温度过低 ");
            if (curr->temperature > 35) strcat_s(reason, 50, "温度过高 ");
            if (curr->humidity < 40) strcat_s(reason, 50, "湿度过低 ");
            if (curr->humidity > 85) strcat_s(reason, 50, "湿度过高 ");

            printf("%-20s %-35s\n", curr->timestamp, reason);
            found++;
        }
        curr = curr->next;
    }
    printf("--------------------------------------------------------------------\n");
    if (found == 0) printf("无异常记录\n");
}

void find_extreme_values(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return;
    float max_t = -999.0f, min_t = 999.0f;
    float max_h = -1.0f, min_h = 101.0f;
    int count = 0;

    SensorRecord* curr = sys->record_head;
    while (curr != NULL) {
        if (curr->field_id == field_id) {
            if (curr->temperature > max_t) max_t = curr->temperature;
            if (curr->temperature < min_t) min_t = curr->temperature;
            if (curr->humidity > max_h) max_h = curr->humidity;
            if (curr->humidity < min_h) min_h = curr->humidity;
            count++;
        }
        curr = curr->next;
    }

    if (count == 0) {
        printf("该田块无数据！\n");
    }
    else {
        printf("\n田块 ID:%d 极值统计:\n", field_id);
        printf("最高温度: %.1f℃, 最低温度: %.1f℃\n", max_t, min_t);
        printf("最高湿度: %.0f%%,  最低湿度: %.0f%%\n", max_h, min_h);
    }
}

void analyze_trend(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return;

    SensorRecord* latest = NULL;
    SensorRecord* oldest = NULL;
    int count = 0;

    SensorRecord* curr = sys->record_head;
    while (curr != NULL) {
        if (curr->field_id == field_id) {
            if (latest == NULL) latest = curr;
            oldest = curr;
            count++;
        }
        curr = curr->next;
    }

    if (count < 2) {
        printf("数据不足(少于2条)，无法分析趋势！\n");
        return;
    }

    printf("\n田块  ID:%d  的趋势分析:\n", field_id);

    printf("温度: %.1f -> %.1f ", oldest->temperature, latest->temperature);      //温度趋势
    if (latest->temperature > oldest->temperature + 0.5) printf("[上升趋势]\n");
    else if (latest->temperature < oldest->temperature - 0.5) printf("[下降趋势]\n");
    else printf("[基本稳定]\n");

    printf("湿度: %.0f -> %.0f ", oldest->humidity, latest->humidity);     //湿度趋势
    if (latest->humidity > oldest->humidity + 2.0) printf("[上升趋势]\n");
    else if (latest->humidity < oldest->humidity - 2.0) printf("[下降趋势]\n");
    else printf("[基本稳定]\n");
}


////////////////////////////////////////////////	    告警管理
void check_and_generate_alerts(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return;

    SensorRecord* latest = NULL;    //查找最新记录
    SensorRecord* curr = sys->record_head;
    while (curr != NULL) {
        if (curr->field_id == field_id) {
            latest = curr;
            break;
        }
        curr = curr->next;
    }

    if (latest == NULL) return;

    struct TempAlert {
        int sev;
        char type[NAME_MAX];
        char desc[DESC_MAX];
    } new_alerts[3];    //温、湿、连续异常
    int alert_cnt = 0;

    if (latest->temperature > 35.0f) {      //检查温度
        new_alerts[alert_cnt].sev = 2;
        strcpy_s(new_alerts[alert_cnt].type, NAME_MAX, "高温告警");
        snprintf(new_alerts[alert_cnt].desc, DESC_MAX, "当前温度 %.1f℃ 超过阈值", latest->temperature);
        alert_cnt++;
    }
    else if (latest->temperature < 10.0f) {
        new_alerts[alert_cnt].sev = 2;
        strcpy_s(new_alerts[alert_cnt].type, NAME_MAX, "低温告警");
        snprintf(new_alerts[alert_cnt].desc, DESC_MAX, "当前温度 %.1f℃ 低于阈值", latest->temperature);
        alert_cnt++;
    }

    if (latest->humidity > 85.0f) {     //检查湿度
        new_alerts[alert_cnt].sev = 1;
        strcpy_s(new_alerts[alert_cnt].type, NAME_MAX, "高湿告警");
        snprintf(new_alerts[alert_cnt].desc, DESC_MAX, "当前湿度 %.0f%% 超过阈值", latest->humidity);
        alert_cnt++;
    }
    else if (latest->humidity < 40.0f) {
        new_alerts[alert_cnt].sev = 1;
        strcpy_s(new_alerts[alert_cnt].type, NAME_MAX, "低湿告警");
        snprintf(new_alerts[alert_cnt].desc, DESC_MAX, "当前湿度 %.0f%% 低于阈值", latest->humidity);
        alert_cnt++;
    }

    int abnormal_streak = 0;    //检查连续异常
    curr = sys->record_head;
    int checked = 0;
    while (curr != NULL && checked < 3) {
        if (curr->field_id == field_id) {
            if (curr->is_abnormal) abnormal_streak++;
            else break;     // 中断
            checked++;
        }
        curr = curr->next;
    }
    if (abnormal_streak >= 3) {
        new_alerts[alert_cnt].sev = 3;
        strcpy_s(new_alerts[alert_cnt].type, NAME_MAX, "持续异常告警");
        strcpy_s(new_alerts[alert_cnt].desc, DESC_MAX, "连续3次检测到异常数据");
        alert_cnt++;
    }

    for (int i = 0; i < alert_cnt; i++) {   //插入链表
        Alert* new_node = (Alert*)malloc(sizeof(Alert));
        new_node->field_id = field_id;
        new_node->severity = new_alerts[i].sev;
        strcpy_s(new_node->alert_type, NAME_MAX, new_alerts[i].type);
        strcpy_s(new_node->description, DESC_MAX, new_alerts[i].desc);
        get_current_timestamp(new_node->alert_time, TIME_MAX);

        if (sys->alert_head == NULL || sys->alert_head->severity < new_node->severity) {    //插入排序
            new_node->next = sys->alert_head;
            sys->alert_head = new_node;
        }
        else {
            Alert* p = sys->alert_head;
            while (p->next != NULL && p->next->severity >= new_node->severity) {
                p = p->next;
            }
            new_node->next = p->next;
            p->next = new_node;
        }
        printf("已生成: [%s] %s\n", new_node->alert_type, new_node->description);
    }
    if (alert_cnt == 0) printf("检测完毕，各项指标正常，未生成告警。\n");
}

void display_all_alerts(MonitorSystem* sys) {
    if (sys == NULL || sys->alert_head == NULL) {
        printf("当前无告警信息\n");
        return;
    }
    printf("%-4s %-20s %-14s %-8s %-30s\n", "ID", "时间", "类型", "程度", "描述");
    printf("================================================================================\n");

    Alert* curr = sys->alert_head;
    while (curr != NULL) {
        char sev_str[10];
        if (curr->severity == 3) strcpy_s(sev_str, 10, "严重");
        else if (curr->severity == 2) strcpy_s(sev_str, 10, "中等");
        else strcpy_s(sev_str, 10, "轻微");

        printf("%-4d %-20s %-14s %-8s %-30s\n",
            curr->field_id, curr->alert_time, curr->alert_type, sev_str, curr->description);
        curr = curr->next;
    }
    printf("================================================================================\n");
}

int resolve_alert(MonitorSystem* sys, int field_id, const char* alert_type) {
    if (sys == NULL) return 0;

    Alert* prev = NULL;
    Alert* curr = sys->alert_head;

    while (curr != NULL) {      //寻找匹配的告警
        if (curr->field_id == field_id && strcmp(curr->alert_type, alert_type) == 0) {
            Alert* to_free = curr;      //删除节点
            if (prev == NULL) {
                sys->alert_head = curr->next;
            }
            else {
                prev->next = curr->next;
            }
            free(to_free);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;   // 未找到
}

int get_alert_count(MonitorSystem* sys, int field_id) {
    if (sys == NULL) return 0;
    
    int count = 0;
    Alert* curr = sys->alert_head;
    while (curr != NULL) {
        if (curr->field_id == field_id) count++;
        curr = curr->next;
    }
    return count;
}


////////////////////////////////////////////
void fieldManagement() {
    int choice;
    int running = 1;

    while (running) {
        clearScreen();
        printLogo();
        fieldManagementMenu();

        printf("请选择操作: ");
        if (scanf_s("%d", &choice) != 1) {
            printf("输入错误！请重新输入!\n");
            clearInputBuffer();
            (void)getchar();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
        case 1: {   // 添加新田块
            char name[NAME_MAX];
            char manager[NAME_MAX];
            int valid = 0;

            do {
                clearScreen();
                printLogo();
                fieldManagementMenu();
                
                printf("------- 添加新田块 -------\n");
                printf("请输入田块名称: ");
                fgets(name, NAME_MAX, stdin);
                name[strcspn(name, "\n")] = 0;

                if (strlen(name) > 0) valid = 1;
                else {
                    printf("名称不能为空！按Enter重试...");
                    (void)getchar();
                }
            } while (!valid);

            valid = 0;
            
            do {
                clearScreen();
                printLogo();
                fieldManagementMenu();

                printf("------- 添加新田块 -------\n");
                printf("请输入田块名称: %s\n", name);
                printf("请输入负责人姓名: ");
                fgets(manager, NAME_MAX, stdin);
                manager[strcspn(manager, "\n")] = 0;

                if (strlen(manager) > 0) valid = 1;
                else {
                    printf("负责人不能为空！按Enter重试...");
                    (void)getchar();
                }
            } while (!valid);

            int id = add_field(g_sys, name, manager);
            printf("\n添加成功！新田块ID: %d\n", id);
            break;
        }
        case 2: {   // 显示所有
            clearScreen();
            printLogo();
            fieldManagementMenu();
            display_all_fields(g_sys);
            break;
        }
        case 3: {   // 查找(ID)
            int id;
            int valid = 0;
            
            do {
                clearScreen();
                printLogo();
                fieldManagementMenu();
                
                printf("--- 查找田块 ---\n请输入ID: ");
                if (scanf_s("%d", &id) == 1 && id > 0) valid = 1;
                else {
                    printf("ID必须为正整数！按Enter重试...");
                    clearInputBuffer();
                    (void)getchar();
                }
            } while (!valid);
            clearInputBuffer();

            Field* f = find_field_by_id(g_sys, id);
            if (f) {
                printf("\n找到田块:\nID: %d\n名称: %s\n负责人: %s\n传感器数: %d\n",
                    f->id, f->name, f->manager, f->sensor_count);
            }
            else {
                printf("\n未找到 ID:%d 的田块\n", id);
            }
            break;
        }
        case 4: {   // 搜索(名称)
            char key[NAME_MAX];
            printf("请输入搜索关键词: ");
            fgets(key, NAME_MAX, stdin);
            key[strcspn(key, "\n")] = 0;
            search_fields_by_name(g_sys, key);
            break;
        }
        case 5: {   // 搜索(负责人)
            char key[NAME_MAX];
            printf("请输入负责人姓名: ");
            fgets(key, NAME_MAX, stdin);
            key[strcspn(key, "\n")] = 0;
            search_fields_by_manager(g_sys, key);
            break;
        }
        case 6: {   // 删除
            int id;
            char confirm;
            printf("请输入要删除的田块ID: ");
            if (scanf_s("%d", &id) == 1) {
                clearInputBuffer();
                Field* f = find_field_by_id(g_sys, id);
                if (f) {
                    printf("确定要删除 [%s] (ID:%d) 吗？(y/n): ", f->name, f->id);
                    scanf_s("%c", &confirm, 1);
                    clearInputBuffer();
                    if (toupper(confirm) == 'Y') {
                        delete_field(g_sys, id);
                        printf("删除成功!\n");
                    }
                    else {
                        printf("取消删除!\n");
                    }
                }
                else {
                    printf("田块不存在\n");
                }
            }
            else {
                clearInputBuffer();
                printf("输入无效!\n");
            }
            break;
        }
        case 0:
            running = 0;
            break;
        default:
            printf("无效选项！\n");
            break;
        }

        if (choice != 0) {
            printf("\n按Enter键继续...");
            (void)getchar();
        }
    }
}

void sensorDataManagement() {
    int choice;
    int running = 1;
    
    while (running) {
        clearScreen();
        printLogo();
        sensorDataMenu();
        
        printf("请选择操作: ");
        if (scanf_s("%d", &choice) != 1) {
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
        case 1: {   // 添加记录
            int id, valid = 0;
            float temp, hum;

            do {
                clearScreen();
                printLogo();
                sensorDataMenu();
                
                printf("--- 添加传感器数据 (步骤 1/3) ---\n请输入田块ID: ");
                if (scanf_s("%d", &id) == 1) {
                    if (find_field_by_id(g_sys, id)) valid = 1;
                    else printf("田块不存在！\n");
                }
                else printf("输入错误!\n");

                if (!valid) {
                    clearInputBuffer();
                    (void)getchar();
                }
            } while (!valid);

            valid = 0;
            do {
                clearScreen();
                printLogo();
                sensorDataMenu();
                
                printf("--- 添加传感器数据 (步骤 2/3) ---\n田块ID: %d\n请输入温度 (-50~60): ", id);
                if (scanf_s("%f", &temp) == 1) {
                    if (temp >= -50 && temp <= 60) valid = 1;
                    else printf("温度超出合理范围！\n");
                }
                else printf("输入错误!\n");
                if (!valid) {
                    clearInputBuffer();
                    (void)getchar();
                }
            } while (!valid);

            valid = 0;
            do {
                clearScreen();
                printLogo();
                sensorDataMenu();
                
                printf("--- 添加传感器数据 (步骤 3/3) ---\n田块ID: %d\n温度: %.1f\n请输入湿度 (0~100): ", id, temp);
                if (scanf_s("%f", &hum) == 1) {
                    if (hum >= 0 && hum <= 100) valid = 1;
                    else printf("湿度超出合理范围！\n");
                }
                else printf("输入错误!\n");
                if (!valid) {
                    clearInputBuffer();
                    (void)getchar();
                }
            } while (!valid);

            add_sensor_record(g_sys, id, temp, hum);
            printf("\n记录添加成功！\n");
            (void)getchar();
            break;
        }
        case 2: {   // 显示最新
            int id, count;
            printf("请输入田块ID: ");
            scanf_s("%d", &id);
            printf("显示最近几条记录: ");
            scanf_s("%d", &count);
            clearInputBuffer();
            display_latest_records(g_sys, id, count);
            break;
        }
        case 3: {   // 显示所有
            int id;
            printf("请输入田块ID: ");
            scanf_s("%d", &id);
            clearInputBuffer();
            display_field_all_records(g_sys, id);
            break;
        }
        case 4: {   // 删除过期
            int days;
            printf("保留最近几天的记录: ");
            if (scanf_s("%d", &days) == 1) {
                clearInputBuffer();
                delete_old_records(g_sys, days);
            }
            else clearInputBuffer();
            break;
        }
        case 5: { // 数量
            int id;
            printf("请输入田块ID: ");
            scanf_s("%d", &id); clearInputBuffer();
            printf("田块 %d 共有 %d 条记录\n", id, get_record_count(g_sys, id));
            break;
        }
        case 6: {   // 平均温度
            int id;
            printf("请输入田块ID: ");
            scanf_s("%d", &id); clearInputBuffer();
            printf("田块 %d 平均温度: %.2f℃\n", id, calculate_avg_temperature(g_sys, id));
            break;
        }
        case 0: running = 0; break;
        default: printf("无效选项！\n"); break;
        }
        if (choice != 0) {
            printf("\n按Enter键继续...");
            (void)getchar();
        }
    }
}

void dataQueryStatistics() {
    int choice;
    int running = 1;
    
    while (running) {
        clearScreen();
        printLogo();
        dataQueryMenu();
        
        printf("请选择操作: ");
        if (scanf_s("%d", &choice) != 1) {
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (choice >= 1 && choice <= 3) {
            int id;
            printf("请输入田块ID: ");
            if (scanf_s("%d", &id) == 1) {
                clearInputBuffer();
                if (find_field_by_id(g_sys, id)) {
                    clearScreen();
                    printLogo();
                    
                    if (choice == 1) find_abnormal_records(g_sys, id);
                    else if (choice == 2) find_extreme_values(g_sys, id);
                    else if (choice == 3) analyze_trend(g_sys, id);
                }
                else {
                    printf("田块不存在！\n");
                }
            }
            else clearInputBuffer();
        }
        else if (choice == 0) {
            running = 0;
        }
        else {
            printf("无效选项！\n");
        }
        if (choice != 0) {
            printf("\n按Enter键继续...");
            (void)getchar();
        }
    }
}

void alertManagement() {
    int choice;
    int running = 1;
    
    while (running) {
        clearScreen();
        printLogo();
        alertManagementMenu();
        
        printf("请选择操作: ");
        if (scanf_s("%d", &choice) != 1) {
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
        case 1: {   // 生成
            int id;
            printf("请输入需检测的田块ID: ");
            scanf_s("%d", &id);
            clearInputBuffer();
            
            if (find_field_by_id(g_sys, id)) check_and_generate_alerts(g_sys, id);
            else printf("田块不存在！\n");
            break;
        }
        case 2: {   // 显示
            display_all_alerts(g_sys);
            break;
        }
        case 3: {   // 处理
            int id;
            char type[NAME_MAX];
            printf("请输入田块ID: ");
            scanf_s("%d", &id);
            clearInputBuffer();
            
            printf("请输入告警类型 (如 高温告警): ");
            fgets(type, NAME_MAX, stdin);
            type[strcspn(type, "\n")] = 0;

            if (resolve_alert(g_sys, id, type)) printf("告警已消除！\n");
            else printf("未找到对应告警！\n");
            break;
        }
        case 4: {   // 统计
            int id;
            printf("请输入田块ID (0查看全部): ");
            scanf_s("%d", &id);
            clearInputBuffer();
            
            if (id == 0) {
                int total = 0;  //遍历
                Alert* p = g_sys->alert_head;
                while (p) { total++; p = p->next; }
                printf("系统共有 %d 条告警\n", total);
            }
            else {
                printf("田块 %d 共有 %d 条告警\n", id, get_alert_count(g_sys, id));
            }
            break;
        }
        case 0: {
            running = 0;
            break;
        }
        default: printf("无效选项！\n");
            break;
        }
        if (choice != 0) {
            printf("\n按Enter键继续...");
            (void)getchar();
        }
    }
}

void systemInformation() {
    clearScreen();
    printLogo();
    systemInformationMenu();

    printf("系统名称：校园试验田环境检测系统\n");
	printf("Version: Beta\n");
    printf("------------------------------\n");
    printf("开发者：ChimeSky\n(NJAU/CS Department/Liu Junfu)\n\n");
    printf("2026.1\n");
    printf("-------------------------------\n");
    printf("开发语言 : C\n");
    printf("编译环境 : MSVC\n");

    printf("\n按Enter键返回主菜单...");
    (void)getchar();
}