#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include"functions.h"
#include"model.h"
#include"UI.h"

extern MonitorSystem* g_sys;

int main() {
    int choice;
    int running = 1;

    g_sys = create_monitor_system();

    if (g_sys == NULL) {
        printf("系统初始化失败！\n");
        return 1;
    }

    while (running) {
        clearScreen();
        printLogo();
        displayMainMenu();

        printf("请输入选项：");

        if (scanf_s("%d", &choice) != 1) {
            printf("输入错误，请输入数字z！\n");

            clearInputBuffer();

            printf("\n按Enter键继续……");

            (void)getchar();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
        case 1:
            clearScreen();
            printLogo();

            fieldManagement();
            break;
        case 2:
            clearScreen();
            printLogo();

            sensorDataManagement();
            break;
        case 3:
            clearScreen();
            printLogo();

            dataQueryStatistics();
            break;
        case 4:
            clearScreen();
            printLogo();

            alertManagement();
            break;
        case 5:
            clearScreen();
            printLogo();

            systemInformation();
            break;
        case 0:
            printf("程序已退出！\n");

            running = 0;
            break;
        default:
            printf("无效选项，请重新选择！\n");
            printf("\n按Enter键继续……");

            (void)getchar();
            break;
        }

    }

    destroy_monitor_system(g_sys);

    return 0;
}