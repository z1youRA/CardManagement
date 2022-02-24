//
// Created by z1youra on 2022/2/22.
//

#ifndef CARDMANAGEMENT_HEADER_H
#define CARDMANAGEMENT_HEADER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <queue>
#include <math.h>

#define DELETED 0 //销户
#define NORMAL 1
#define NONEXIST 2             //账户不存在
#define BANNED 0               //禁用
#define LOSS 2                 //挂失
#define EXPDATE 20240715       //过期时间
#define FIRSTCARDNUMBER 312346 //第一张卡卡号
#define MAJORNUM 100
#define STUDENTPERMAJOR 500
#define STUDENTSNUM 50000 //学生上限
#define OK 1
#define FAILED 0
#define OPENACC 0
#define DELETEACC 1
#define OPENCARD 2
#define REPOLOSS 3
#define CANCELLOSS 4
#define TOPUP 5
#define PAY 6
#define EMPTY 7
#define TIMENOW 2022090309220510

using namespace std;

typedef struct card
{
    long studentNum;
    int cardNum;
    int status;
    int balance;
    int expDate;
    int pwd;
    struct card *next; //指向该学生的下一张卡
    struct card *down; //指向卡号递增的下一张卡
    //#TODO类似十字链表，在窗口和卡片处均存储log
} Card;

typedef struct opelog
{
    int type; //操作类型
    long studentNum;
    char name[20];
    int cardNum;
    int result; //操作结果
    int value;  //金额
    long time;
    struct opelog *next;
} OpeLog;

typedef struct student
{
    char name[20];
    long studentNum;
    Card *front;
    Card *rear;
    int status;
    OpeLog *head;
    OpeLog *tail;
} Student;

typedef struct window
{
    int index;
    int logQuantity;
    OpeLog *rear;
} Window;

struct comp
{
    bool operator()(const struct opelog *a, const struct opelog *b)
    {
        return a->time > b->time;
    }
};

#endif //CARDMANAGEMENT_HEADER_H
