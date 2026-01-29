#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "model.h"

//系统管理函数
MonitorSystem* create_monitor_system(void);
void destroy_monitor_system(MonitorSystem* sys);
void expand_field_array(MonitorSystem* sys);

//实验田管理函数
int add_field(MonitorSystem* sys, const char* name, const char* manager);
void display_all_fields(MonitorSystem* sys);
Field* find_field_by_id(MonitorSystem* sys, int field_id);
int delete_field(MonitorSystem* sys, int field_id);
void search_fields_by_name(MonitorSystem* sys, const char* keyword);
void search_fields_by_manager(MonitorSystem* sys, const char* manager_name);

//传感器数据管理函数
int add_sensor_record(MonitorSystem* sys, int field_id, float temperature, float humidity);
void display_latest_records(MonitorSystem* sys, int field_id, int count);
void display_field_all_records(MonitorSystem* sys, int field_id);
void delete_old_records(MonitorSystem* sys, int days);
int get_record_count(MonitorSystem* sys, int field_id);
float calculate_avg_temperature(MonitorSystem* sys, int field_id);

//数据分析函数
void find_abnormal_records(MonitorSystem* sys, int field_id);
void find_extreme_values(MonitorSystem* sys, int field_id);
void analyze_trend(MonitorSystem* sys, int field_id);

//告警管理函数
void check_and_generate_alerts(MonitorSystem* sys, int field_id);
void display_all_alerts(MonitorSystem* sys);
int resolve_alert(MonitorSystem* sys, int field_id, const char* alert_type);
int get_alert_count(MonitorSystem* sys, int field_id);

//交互菜单函数
void fieldManagement();
void sensorDataManagement();
void dataQueryStatistics();
void alertManagement();
void systemInformation();

#endif