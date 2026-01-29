#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include"functions.h"
#include"model.h"
#include"UI.h"

void clearScreen() {
    system("cls");
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printLogo() {
    printf("\n");
    printf("███╗   ██╗     ██╗ █████╗ ██╗   ██╗\n");
    printf("████╗  ██║     ██║██╔══██╗██║   ██║\n");
    printf("██╔██╗ ██║     ██║███████║██║   ██║\n");
    printf("██║╚██╗██║██   ██║██╔══██║██║   ██║\n");
    printf("██║ ╚████║╚█████╔╝██║  ██║╚██████╔╝\n");
    printf("╚═╝  ╚═══╝ ╚════╝ ╚═╝  ╚═╝ ╚═════╝ \n");
}

void displayMainMenu() {
    printf("\n");
    printf("=======校园试验田环境检测系统=======\n");
    printf("====================================\n");
    printf("1. 试验田管理\n");
    printf("2. 传感器数据管理\n");
    printf("3. 数据查询与统计\n");
    printf("4. 告警管理\n");
    printf("5. 系统信息\n");
    printf("0. 退出系统\n");
    printf("====================================\n\n");
}

void fieldManagementMenu() {
    printf("\n");
    printf("=======校园试验田环境检测系统=======\n");
	printf("-------------实验田管理-------------\n");
	printf("1. 添加新田块\n");
	printf("2. 显示所有田块\n");
	printf("3. 查找田块（按ID）\n");
	printf("4. 搜索田块（按名称）\n");
    printf("5. 搜索田块（按负责人）\n");
    printf("6. 删除田块\n");
    printf("0. 返回主菜单\n");
	printf("------------------------------------\n\n");
}

void sensorDataMenu() {
    printf("\n");
    printf("=======校园试验田环境检测系统=======\n");
    printf("-----------传感器数据管理-----------\n");
    printf("1. 添加传感器记录\n");
    printf("2. 显示最新记录\n");
    printf("3. 显示所有记录\n");
    printf("4. 删除过期记录\n");
	printf("5. 获取记录数量\n");
	printf("6. 计算平均温度\n");
    printf("0. 返回主菜单\n");
    printf("------------------------------------\n\n");
}

void dataQueryMenu() {
    printf("\n");
    printf("=======校园试验田环境检测系统=======\n");
    printf("-----------数据查询与统计-----------\n");
    printf("1. 查找异常记录\n");
    printf("2. 查找极值\n");
    printf("3. 分析趋势\n");
    printf("0. 返回主菜单\n");
    printf("------------------------------------\n\n");
}

void alertManagementMenu() {
    printf("\n");
    printf("=======校园试验田环境检测系统=======\n");
    printf("--------------告警管理--------------\n");
    printf("1. 检查并生成告警\n");
    printf("2. 显示所有告警\n");
    printf("3. 处理告警\n");
    printf("4. 统计告警数量\n");
    printf("0. 返回主菜单\n");
    printf("------------------------------------\n\n");
}

void systemInformationMenu() {
    printf("\n");
    printf("=======校园试验田环境检测系统=======\n");
    printf("--------------系统信息--------------\n");
    printf("------------------------------------\n\n");
}