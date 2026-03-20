#ifndef WINDIXER_DISK_H
#define WINDIXER_DISK_H

#include <windows.h>

#define MAX_VOLUMES 26

typedef struct {
    char letter;
    char label[64];
    char fs_type[16];
    char drive_type[32];
    ULONGLONG total;
    ULONGLONG free;
    int accessible;
} VolumeInfo;

typedef struct {
    VolumeInfo volumes[MAX_VOLUMES];
    int count;
} VolumeList;

void menu_disk(void);

#endif