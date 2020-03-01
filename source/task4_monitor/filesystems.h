/* 
 */
#ifndef FILESYSTEMS_H
#define FILESYSTEMS_H

#include <QString>
#include <QTimer>
#include <QProcess>
#include <QDebug>
#include <QObject>
#include "sys/statfs.h"
#include <stdio.h>
#include <mntent.h>
#include <string.h>
#include <sys/vfs.h>


static const unsigned long long G = 1024*1024*1024ull;
static const unsigned long long M = 1024*1024;
static const unsigned long long K = 1024;
static char str[20];


extern int device_num;
extern char mount_on_device[20];
extern QString device_info[20][6];

char* kscale(unsigned long b, unsigned long bs);
int mydf();



#endif //FILESYSTEMS_H
