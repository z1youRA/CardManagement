#include <regex>
#include "header.h"

unordered_map<long, Student *> students;
unordered_map<int, Card *> cards;
vector<OpeLog *> windowRec[100];
OpeLog *logs = NULL;
int cardSum = 0;
Window windows[100];
FILE* logFile = fopen("/home/z1youra/CLionProjects/cardManagement/log/ope_log.txt", "w");

//所有学生状态初始化为不存在
void initStatus()
{
    for (int i = 0; i < 100; i++)
    {
        windows[i].index = i + 1;
        windows[i].rear = NULL;
        windows[i].logQuantity = 0;
    }
}

//用卡号计算出验证位并返回
int getValidDigit(long num)
{
    int sum = 0;
    while (num != 0)
    {
        sum += num % 10;
        num /= 10;
    }
    return 9 - (sum % 10);
}

//依次序输出卡号
int cardNumberFactory()
{
    int currentCardNum = FIRSTCARDNUMBER + cardSum;
    cardSum++;
    int validDigit = getValidDigit(currentCardNum);
    return currentCardNum * 10 + validDigit;
}

int balanceToInt(float balance)
{
    return (int)round(balance * 100);   //使用round避免精度丢失
}

//初始化学生结构，返回指向该学生的指针
Student *initStu(char *name, long studentNum)
{
    Student *stu;
    int i = 0;
    stu = (Student *)malloc(sizeof(Student));
    if (stu == NULL)
    {
        printf("动态空间申请失败！\n");
        exit(1);
    }
    for (i = 0; name[i] != '\0'; i++)
    {
        stu->name[i] = name[i];
    }
    stu->name[i] = '\0';
    stu->studentNum = studentNum;
    stu->front = NULL;
    stu->rear = NULL;
    stu->status = NORMAL;
    stu->head = NULL;
    stu->tail = NULL;
    return stu;
}

//通过已知信息初始化卡片并返回指针
Card *initCard(long studentNum, int status, float balance, int expDate, int pwd)
{
    Card *card;
    card = (Card *)malloc(sizeof(Card));
    if (card == NULL)
    {
        printf("动态空间申请失败!\n");
        exit(1);
    }
    int abc = (int)(balance * 100);
    card->cardNum = cardNumberFactory();
    card->balance = abc;
    card->expDate = expDate;
    card->status = status;
    card->studentNum = studentNum;
    card->pwd = pwd;
    card->next = NULL;
    cards[card->cardNum] = card; //将卡指针存入unordered_map
    return card;
}

//通过卡号查找对应卡并返回其指针，若查找失败返回NULL#TODOrewrite
Card *getCard(int cardNum)
{
    unordered_map<int, Card *>::iterator iter = cards.find(cardNum);
    if (cards.find(cardNum) == cards.end())
    {
        printf("未找到该卡号，查找失败！\n");
        return NULL;
    }
    pair<int, Card *> pr = *iter;
    return pr.second;
}

//通过学号索引对应学生并返回其指针， 若学号超出范围exit
Student *getStudent(long studentNum)
{
    unordered_map<long, Student *>::iterator iter = students.find(studentNum);
    if (students.find(studentNum) == students.end())
    {
        return NULL;
    }
    pair<long, Student *> pr = *iter;
    return pr.second;
}

//初始化操作日志
OpeLog *initOpeLog(int type, long studentNum, int cardNum, int result, int value, long time)
{
    OpeLog *opelog;
    Student *stu;
    opelog = (OpeLog *)malloc(sizeof(OpeLog));
    if (opelog == NULL)
    {
        printf("动态空间申请失败!\n");
        exit(1);
    }
    opelog->type = type;
    opelog->studentNum = studentNum;
    opelog->cardNum = cardNum;
    opelog->result = result;
    opelog->value = value;
    opelog->time = time;
    if (studentNum == -1)
    { //若学号为-1，说明为占位所用
        strcpy(opelog->name, "EMPTY");
        return opelog;
    }
    stu = getStudent(studentNum);
    if(stu == NULL) {
        printf("ERROR\n\n\n");
        exit(1);
    }
    strcpy(opelog->name, getStudent(studentNum)->name);
    return opelog;
}

char* generateLog(Student *stu, int type, int result, int value, long time, const char* message) {
    long stuNum = stu->studentNum;
    int cardNum = stu->rear->cardNum;
    float balance = ((float )stu->rear->balance) / 100;
    char* log;
    char typeStr[10];
    char statusStr[10];
    log = (char *)malloc(150);
    int max_len = 150;
    int j;
    switch (type) {
        case OPENACC:
            strncpy(typeStr, "开户", 10);
            break;
        case DELETEACC:
            strncpy(typeStr, "销户", 10);break;
        case OPENCARD:
            strncpy(typeStr, "开卡", 10);break;
        case REPOLOSS:
            strncpy(typeStr, "挂失", 10);break;
        case CANCELLOSS:
            strncpy(typeStr, "解挂", 10);break;
        case TOPUP:
            strncpy(typeStr, "充值", 10);break;
        case PAY:
            strncpy(typeStr, "消费", 10);break;
        case EMPTY:
            strncpy(typeStr, "空", 10);break;
        default:
            strncpy(typeStr, "ERROR", 10);break;
    }
    if(result == OK) {
        strncpy(statusStr, "SUCCESS", 10);
    }
    else if(result == FAILED) {
        strncpy(statusStr, "FAILED", 10);
    }
    j = snprintf (log, max_len, "%ld StuID: %ld cardID: %d %s | %s value: %d balance: %f #%s", time, stuNum, cardNum, typeStr, statusStr, value, balance, message);
    if (j >= max_len)
        fputs ("Buffer length exceeded; string truncated", stderr);
    return log;
}

//传入学生信息及日志信息，将日志保存到学生信息中, 同时在logs链表中保存该条日志
int saveOpeLogToStu(Student *stu, int type, int result, int value, long time, const char* message)
{
    char* log = generateLog(stu, type, result, value, time, message);
//    printf("%s\n", log);
    fprintf(logFile, "%s\n", log);
    free(log);
//    if (stu->tail)
//    {
//        Student *temp = getStudent(stu->studentNum);
//        if (temp->rear == NULL)
//        {
//            stu->tail->next = initOpeLog(type, stu->studentNum, 0, result, value, time);
//        }
//        else
//        {
//            stu->tail->next = initOpeLog(type, stu->studentNum, temp->rear->cardNum, result, value, time);
//        }
//        stu->tail = stu->tail->next;
//    }
//    else
//    {
//        printf("ERROR:保存失败!");
//        exit(1);
//    }
}

//手动开户操作
int openAccount(char *name, long studentNum)
{
    Student *temp;
    Student *target = getStudent(studentNum); //学生在学生数组中的位置指针
    //#TODO 检查输入学号年份和专业是否存在
    if (target != NULL)
    {
        printf("ERROR: 该学号已开户！\n");
        exit(1);
    }
    temp = initStu(name, studentNum);
    // *target = *temp;
    students[studentNum] = temp; //将生成的学生指针存入unordered_map
    if (temp->head == NULL)
    {
        temp->head = initOpeLog(OPENACC, studentNum, 0, OK, 0, 0); //生成循环链表存储日志 #TODO日志从中间位置存储
        temp->tail = temp->head;
    }
    else
    {
        printf("该学生日志不为空，日志记录失败！");
        return FAILED;
    }
    return OK;
}

//为指定学生开卡
int openCard(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        Card *card = initCard(studentNum, NORMAL, 0, EXPDATE, 8888); //默认密码8888
        if (stu->rear == NULL)
        { //该学生卡片数量为0
            stu->front = card;
            stu->rear = card;
            card->next = NULL;
            saveOpeLogToStu(stu, OPENCARD, OK, 0, 0, "开卡成功");
        }
        else if (stu->rear->status == LOSS)
        {
            stu->rear->status = BANNED;
            stu->rear->next = card;
            card->balance = stu->rear->balance; //将上张卡余额转入本张卡， 上张卡balance清零
            stu->rear->balance = 0;
            stu->rear = card;
            card->next = NULL;
            saveOpeLogToStu(stu, OPENCARD, OK, 0, 0, "开卡成功");
        }
        else if (stu->rear->status == BANNED)
        {
//            printf("ERROR: 上张卡被禁用，开卡失败！\n");
            saveOpeLogToStu(stu, OPENCARD, FAILED, 0, 0, "上张卡被禁用");
            exit(FAILED);
        }
        else
        { // 上张卡正常使用，未挂失
//            printf("卡未挂失，请先挂失后开卡！\n");
            saveOpeLogToStu(stu, OPENCARD, FAILED, 0, 0, "卡未挂失");
            return FAILED;
        }
    }
    else
    {
        printf("ERROR: 账户不可用，开卡失败！\n");
//        saveOpeLogToStu(stu, OPENCARD, FAILED, 0, 0, "账户不可用");
        return FAILED;
    }
}

//向学号指定的账户充值，充值成功返回OK，失败返回FAILED
int topupBalance(long studentNum, float topupAmout)
{
    Student *stu = getStudent(studentNum);
    int temp = balanceToInt(topupAmout);
    if (stu->status == NORMAL)
    {
        if (stu->rear == NULL)
        {
            printf("该学生无卡，请先开卡！\n");
            saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0, "该学生无卡");
            return FAILED;
        }
        else if (stu->rear->status == NORMAL)
        {
            if (temp <= 0)
            {
                saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0, "充值金额需大于0");
//                printf("充值金额需大于0, 充值失败！\n");
                return FAILED;
            }
            if ((temp + stu->rear->balance) >= 100000)
            { //充值后金额大于1000元
                saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0, "卡内余额需小于1000元");
//                printf("卡内余额需小于1000元， 充值失败！\n");
                return FAILED;
            }
            stu->rear->balance += temp;
            saveOpeLogToStu(stu, TOPUP, OK, topupAmout, 0, "充值成功");
//            printf("充值成功！\n");
            return OK;
        }
        else
        {
            saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0, "该学生卡已被挂失或禁用");
//            printf("该学生卡已被挂失或禁用，充值失败！\n");
            return FAILED;
        }
    }
    else
    {
        saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0, "账户被注销或不存在");
//        printf("账户被注销或不存在，充值失败！\n");
        return FAILED;
    }
}

//挂失
int reportLoss(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        if (stu->rear == NULL)
        {
            saveOpeLogToStu(stu, REPOLOSS, FAILED, 0, 0, "该学生无卡");
//            printf("该学生无卡，请先开卡！\n");
            return FAILED;
        }
        else if (stu->rear->status == NORMAL)
        {
            stu->rear->status = LOSS;
            saveOpeLogToStu(stu, REPOLOSS, OK, 0, 0, "挂失成功");
//            printf("挂失成功！\n");
            return OK;
        }
        else
        {
            saveOpeLogToStu(stu, REPOLOSS, FAILED, 0, 0, "该学生卡已被挂失或禁用");
//            printf("该学生卡已被挂失或禁用，挂失失败！\n");
            return FAILED;
        }
    }
    else
    {
        saveOpeLogToStu(stu, REPOLOSS, FAILED, 0, 0, "账户不可用");
//        printf("ERROR: 账户不可用，挂失失败！\n");
        return FAILED;
    }
}

int cancelLoss(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        if (stu->rear == NULL)
        {
            saveOpeLogToStu(stu, CANCELLOSS, FAILED, 0, 0, "该学生无卡");
//            printf("该学生无卡，请先开卡！\n");
            return FAILED;
        }
        else if (stu->rear->status == LOSS)
        {
            stu->rear->status = NORMAL;
            saveOpeLogToStu(stu, CANCELLOSS, OK, 0, 0, "解挂成功");
//            printf("解挂成功！\n");
            return OK;
        }
        else
        {
//            printf("该学生卡未挂失或已禁用，解挂失败！\n");
            saveOpeLogToStu(stu, CANCELLOSS, FAILED, 0, 0, "该学生卡未挂失或已禁用");
            return FAILED;
        }
    }
    else
    {
        printf("ERROR: 账户不可用，解挂失败！\n");
        saveOpeLogToStu(stu, CANCELLOSS, FAILED, 0, 0, "账户不可用");
        return FAILED;
    }
}

int deleteAccount(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu == NULL)
    {
        printf("学号: %ld\n该学生不存在， 注销失败！\n", studentNum);
        return FAILED;
    }
    if (stu->status == NORMAL)
    {
        stu->status = DELETED;
        saveOpeLogToStu(stu, DELETEACC, OK, 0, 0, "该学生不存在");
        printf("账户注销成功！\n");
        return OK;
    }
    else
    {
        printf("ERROR: 账户不可用，注销失败！\n");
        saveOpeLogToStu(stu, DELETEACC, FAILED, 0, 0, "账户不可用");
        return FAILED;
    }
}

//食堂支付
int pay(int cardNum, float payAmount)
{
    Card *card = getCard(cardNum);
    Student *stu = getStudent(card->studentNum);
    int payInt = balanceToInt(payAmount);
//    if(stu->studentNum == 2020130027) {
//        saveOpeLogToStu(stu, PAY, OK, payAmount, 0, "imp");
//    }
    if (stu->status == NORMAL)
    { //学生账户状态正常
        if (card->status == NORMAL)
        { //卡状态正常
            if (card->balance - payInt >= 0)
            { //卡中余额充足
                card->balance -= payInt;
                // printf("支付成功！\n");
//                saveOpeLogToStu(stu, PAY, OK, payAmount, 0);
                return OK;
            }
            else
            {
                printf("卡中余额不足，支付失败!\n");
//                saveOpeLogToStu(stu, PAY, FAILED, payAmount, 0);
                return FAILED;
            }
        }
        else
        {
            printf("卡已挂失或禁用，支付失败！\n");
//            saveOpeLogToStu(stu, PAY, FAILED, payAmount, 0);
            return FAILED;
        }
    }
    else
    {
        printf("卡号对应学生账户不存在或已注销，支付失败!\n");
//        saveOpeLogToStu(stu, PAY, FAILED, payAmount, 0);
        return FAILED;
    }
}

//初始化窗口日志，使日志从指定位置开始存储
int initWindow(int index, int position)
{
    OpeLog *log = initOpeLog(EMPTY, -1, 0, OK, 0, TIMENOW);
    if (windows[index].rear == NULL)
    { // 窗口日志为空， 生成一个初始日志
        windows[index].rear = log;
        log->next = log;
        windows[index].logQuantity++;
    }
    for (int i = 0; i < position - 2; i++)
    { // 生成position - 2个占位日志
        OpeLog *log = initOpeLog(EMPTY, -1, 0, OK, 0, TIMENOW);
        log->next = windows[index].rear->next;
        windows[index].rear->next = log;
        windows[index].rear = log;
        windows[index].logQuantity++;
    }
    return OK;
}

//模拟在窗口出消费，输入窗口序号，卡号，支付金额，时间戳
int payAtWindow(int index, int cardNum, float payAmount, int time)
{
    Card *card = getCard(cardNum);
    if (card == NULL)
    {
        printf("卡号: %d不存在， 支付失败！\n", cardNum);
        return FAILED;
    }
    OpeLog *temp = initOpeLog(PAY, getCard(cardNum)->studentNum, cardNum, OK, payAmount, time);
    if (pay(cardNum, payAmount) == OK)
    {
        if (windows[index].logQuantity < 60000)
        {
            if (windows[index].rear)
            { //该窗口日志不为空
                temp->next = windows[index].rear->next;
                windows[index].rear->next = temp;
                windows[index].rear = temp;
            }
            else
            {
                windows->rear = temp;
                temp->next = temp;
            }
        }
        else // 60000条日志已满
        {
            temp->next = windows[index].rear->next->next;
            free(windows[index].rear->next);
            windows[index].rear->next = temp;
            windows[index].rear = temp;
        }
        return OK;
    }
    else
    {
        return FAILED;
    }
}

//从文件中读入信息，进行批量开户操作
int importOpenDisInfo()
{
    FILE *file = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/kh001.txt", "r"); //#TODO change to relative path
    char *num = NULL;
    char *name = NULL;
    char *ptr;
    long numLong = 0;
    if (file == NULL)
    {
        printf("ERROR: 文件路径有误！\n");
        exit(FAILED);
    }
    char str[30];
    if (fgets(str, 30, file))
    {
        if (!strncmp(str, "KH", 2))
        { //检测验证文件正确性
            while (fgets(str, 30, file))
            { //逐行读入信息
                num = strtok(str, ",");
                name = strtok(NULL, ";");        //从str中拆分出学号和姓名两部分
                numLong = strtol(num, &ptr, 10); // str转换为long形式，便于存储
                openAccount(name, numLong);
                openCard(numLong); // #TODO开户开卡应在函数外
            }
        }
        else
        {
            printf("file error\n");
            exit(1);
        }
    }
    fclose(file);
    return OK;
}

int importPositionInfo()
{
    FILE *file = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/wz003.txt", "r");

    char *index = NULL;
    char *position = NULL;
    int i;
    int p;
    char str[30];
    if (file == NULL)
    {
        printf("ERROR: 文件路径有误！");
        exit(1);
    }
    if (fgets(str, 10, file))
    {
        if (!strncmp(str, "WZ", 2))
        {
            while (fgets(str, 30, file))
            {
                index = strtok(str, ",");
                position = strtok(NULL, ";");
                i = atoi(index);
                p = atoi(position);
                initWindow(i, p);
            }
        }
        else
        {
            printf("FILE ERROR\n");
            exit(1);
        }
    }
    return OK;
}

int importOpeInfo()
{
    FILE *opeFile = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/cz002.txt", "r");
    char opeStr[50];
    char *ope = NULL;
    char *ptr;
    long stuNum;
    int type;
    long time;
    int value = 0; //充值金额
    if (opeFile == NULL)
    {
        printf("ERROR: 文件路径有误！");
        exit(1);
    }

    if (fgets(opeStr, 50, opeFile))
    {
        if (!strncmp(opeStr, "CZ", 2))
        {
            while (fgets(opeStr, 50, opeFile))
            {
                time = strtol(strtok(opeStr, ","), &ptr, 10); //时间戳转为long存储
                ope = strtok(NULL, ",");
                if (!strcmp(ope, "挂失"))
                    type = REPOLOSS;
                else if (!strcmp(ope, "解挂"))
                    type = CANCELLOSS;
                else if (!strcmp(ope, "销户"))
                    type = DELETEACC;
                else if (!strcmp(ope, "补卡"))
                    type = OPENCARD;
                else if (!strcmp(ope, "充值"))
                {
                    type = TOPUP;
                    stuNum = strtol((strtok(NULL, ",")), &ptr, 10);
                    value = balanceToInt(strtof(strtok(NULL, ";"), NULL)); //将读入的str转为float再转为int存储
                    Student *stu = getStudent(stuNum);
                    if (stu == NULL)
                    {
                        printf("ERROR: 学生未开户，请先进行开户后充值\n");
                        return FAILED;
                    }
                    if (stu->rear == NULL)
                    {
                        printf("ERROR: 学生未开卡，请先开卡后充值\n");
                        return FAILED;
                    }
                    int cardNum = stu->rear->cardNum;
                    OpeLog *log = initOpeLog(type, stuNum, cardNum, 0, value, time);
                    windowRec[0].push_back(log);
                    continue;
                }
                else
                    printf("ERROR: 数据读取有误!\n");
                stuNum = strtol((strtok(NULL, ";")), &ptr, 10);
                OpeLog *log = initOpeLog(type, stuNum, 0, 0, value, time);
                windowRec[0].push_back(log);
            }
        }
    }
    else
    {
        return FAILED;
    }
}

int importPayInfo()
{
    FILE *payFile = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/xf014.txt", "r");
    char payStr[50];
    OpeLog *log = NULL;
    char *ptr, *p;
    int cardNum;
    char *time1;
    char *time2;
    int windowIndex;
    long time;
    int value;
    if (payFile == NULL)
    {
        printf("ERROR: 文件路径有误！");
        exit(1);
    }

    if (fgets(payStr, 50, payFile))
    {
        if (!strncmp(payStr, "XF", 2))
        {
            while (fgets(payStr, 50, payFile))
            {
                if (payStr[0] == 'W')
                {
                    int i = 0;
                    while (payStr[i] != '\0')
                    {
                        payStr[i] = payStr[i + 1];
                        i++;
                    }
                    windowIndex = strtol(payStr, &ptr, 10);
                }
                else
                {
                    cardNum = atoi(strtok(payStr, ","));
                    time1 = strtok(NULL, ",");
                    time2 = strtok(NULL, ",");

                    char *_time = (char *) malloc(100);
                    memset(_time, 0, 100);
                    strcat(_time, time1);
                    strcat(_time, time2);
                    time = strtol(_time, &ptr, 10);
//                    strcat(time1, time2);
//                    time = strtol(time1, &ptr, 10);
                    value = balanceToInt(stof(strtok(NULL, ";")));
                    log = initOpeLog(PAY, -1, cardNum, windowIndex, value, time); // result存储windowIndex便于调用pay函数
                    windowRec[windowIndex].push_back(log);
                }
            }
        }
    }
    else
    {
        return FAILED;
    }
    return OK;
}

vector<OpeLog *> mergesort(vector<OpeLog *> *array)
{
    clock_t start = clock();
    vector<OpeLog *> result;
    priority_queue<OpeLog *, vector<OpeLog *>, comp> pq;
    for (int i = 0; i < 100; i++)
    {
        if (array[i].size() > 0)
        {
            pq.push(array[i][0]);
        }
    }
    int count = 0;
    while (!pq.empty())
    {
        OpeLog *tmp = pq.top();
        pq.pop();
        result.push_back(tmp);
        array[tmp->result].erase(array[tmp->result].begin());
        if (array[tmp->result].size() > 0)
        {
            pq.push(array[tmp->result][0]);
        }
        count++;
        if (count % 10000 == 0)
        {
            cout << count << endl;
        }
    }
    cout << "merge_k time: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s" << endl;
    cout << result.size();
    return result;
}

int executeOpe(OpeLog *ope)
{
    switch (ope->type)
    {
        case OPENACC:
            openAccount(ope->name, ope->studentNum);
            break;
        case OPENCARD:
            openCard(ope->studentNum);
            break;
        case DELETEACC:
            deleteAccount(ope->studentNum);
            break;
        case REPOLOSS:
            reportLoss(ope->studentNum);
            break;
        case CANCELLOSS:
            cancelLoss(ope->studentNum);
            break;
        case TOPUP:
            topupBalance(ope->studentNum, (float)(ope->value) / 100);
            break;
        case PAY:
            payAtWindow(ope->result, ope->cardNum, (float)(ope->value) / 100, ope->time);
            break;
        default:
            break;
    }
}

//根据归并排序后的操作顺序逐个执行操作, 传入result和当前时间
int opeByResult(vector<OpeLog *> result)
{
    for (int i = 0; i<result.size() && result[i]->time < TIMENOW; i++)
    {
        executeOpe(result[i]);
    }
}

string searchByStuId(string pattern) {
    int index;
    char str[256];
    while ((index = pattern.find("?")) != pattern.npos) {
        pattern.replace(index, 1, "(.{1,1})");
    }
    while ((index = pattern.find("*")) != pattern.npos) {
        pattern.replace(index, 1, "(.{0,})");
    }
    regex patten_re(pattern);
    auto iter = students.begin();   //iterator遍历学生寻找与正则表达式匹配的学号
    string out_str;
    while (iter != students.end()) {
        if (regex_match(to_string(iter->second->studentNum), patten_re)) {
            out_str.append(to_string(iter->second->studentNum));
            out_str.append(", ");
        }
        iter++;
    }
    return out_str;
}

int fuzzySearch() {
    string input;
    cout << "Input: " << endl;
    cin >> input;
    string result = searchByStuId(input);
    cout << result << endl;
}

int analysis() {}

int main()
{
    // int abc = cardNumberFactory();
    // initCard(123, NORMAL, 100.12, EXPDATE, 8888);
    long stuNum = 2020130027;
    initStatus();
    importOpenDisInfo();
    importPositionInfo();
    importOpeInfo();
    importPayInfo();
    vector<OpeLog *> result = mergesort(windowRec);
    for(int i = 0; i < result.size(); i++) {
        if(result[i] == NULL) {
            printf("ERROR");
        }
    }
    opeByResult(result);
    fclose(logFile);
//    while(1) {
//        scanf("%ld", &stuNum);
//        Student *stu = students.find(stuNum)->second;
//        printf("%d", stu->rear->balance);
//    }
    fuzzySearch();


    return 0;
}