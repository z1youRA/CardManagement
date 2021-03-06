
#include "header.h"

unordered_map<long, Student *> students;
unordered_map<int, Card *> cards;
vector<OpeLog *> windowRec[100];
int cardSum = 0;
Window windows[100];
fstream opeLog;
int importFlag = 0;
int balanceNotEnough = 0;
int accountNotExisted = 0;
int cardBanned = 0;
int abc = 0;
//所有学生状态初始化为不存在
void initStatus()
{
    for (int i = 0; i < 100; i++)
    {
        windows[i].index = i;
        windows[i].rear = nullptr;
        windows[i].logQuantity = 0;
    }
}

//用卡号计算出验证位并返回
int getValidDigit(int num)
{
    int sum = 0;
    while (num != 0)
    {
        sum += num % 10;
        num /= 10;
    }
    return 9 - (sum % 10);
}

//生成当前时间的时间戳，返回类型为long
long generateTimeNow() {
    char* ptr;
    timeval curTime;
    long timeNow;
    gettimeofday(&curTime, nullptr);
    int milli = curTime.tv_usec / 10000;

    char buffer [80];
    strftime(buffer, 80, "%Y%m%d%H%M%S", localtime(&curTime.tv_sec));

    char currentTime[84] = "";
    sprintf(currentTime, "%s%02d", buffer, milli); //利用系统时间生成时间戳
    timeNow = strtol(currentTime, &ptr, 10);
    return timeNow;
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
    if (stu == nullptr)
    {
        printf("动态空间申请失败！\n");
        exit(1);
    }
    for (; name[i] != '\0'; i++)
    {
        stu->name[i] = name[i];
    }
    stu->name[i] = '\0';
    stu->studentNum = studentNum;
    stu->front = nullptr;
    stu->rear = nullptr;
    stu->status = NORMAL;
    return stu;
}

//通过已知信息初始化卡片并返回指针
Card *initCard(long studentNum, int status, float balance, int expDate, string pwd)
{
    Card *card;
    card = (Card *)malloc(sizeof(Card));
    if (card == nullptr)
    {
        printf("动态空间申请失败!\n");
        exit(1);
    }
    card->cardNum = cardNumberFactory();
    card->balance = balanceToInt(balance);
    card->expDate = expDate;
    card->status = status;
    card->studentNum = studentNum;
    strcpy(card->pwd, pwd.c_str());
    card->next = nullptr;
    cards[card->cardNum] = card; //将卡指针存入unordered_map
    return card;
}

//通过卡号查找对应卡并返回其指针，若查找失败返回nullptr#TODOrewrite
Card *getCard(int cardNum)
{
    unordered_map<int, Card *>::iterator iter = cards.find(cardNum);
    if (cards.find(cardNum) == cards.end())
    {
        printf("未找到该卡号，查找失败！\n");
        return nullptr;
    }
    pair<int, Card *> pr = *iter;
    return pr.second;
}

//通过学号索引对应学生并返回其指针， 若学号超出范围exit
Student *getStudent(long studentNum)
{
    auto iter = students.find(studentNum);
    if (students.find(studentNum) == students.end())
    {
        return nullptr;
    }
    pair<long, Student *> pr = *iter;
    return pr.second;
}

int sumEveryDigits(long num) {
    int sum = 0;
    while(num > 0) {
        sum += num % 10;
        num = num / 10;
    }
    return sum;
}

int generateValid(OpeLog *log) {
    return 9 - ((sumEveryDigits(log->studentNum) + sumEveryDigits(log->value) + sumEveryDigits(log->type)) % 10);
}

//初始化操作日志为OpeLog格式并返回
OpeLog *initOpeLog(int type, long studentNum, int cardNum, int result, int value, long time)
{
    OpeLog *opelog;
    Student *stu;
    opelog = (OpeLog *)malloc(sizeof(OpeLog));
    if (opelog == nullptr)
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
    if(stu == nullptr) {
        printf("ERROR\n\n\n");
        exit(1);
    }
    strcpy(opelog->name, getStudent(studentNum)->name);
    return opelog;
}

//生成日志item并返回
string generateLog(Student *stu, int type, int result, float value, long time, const char* message, int typeflag = 1) {
    long stuNum = stu->studentNum;
    int cardNum = 0;
    float balance = 0;
    string log;
    string typeStr;
    string statusStr;
    stringstream valueStream;
    stringstream balanceStream;
    int max_len = 150;
    int j;
    if(stu->rear) {
        cardNum = stu->rear->cardNum;
        balance = ((float )stu->rear->balance) / 100;
    }
    switch (type) {
        case OPENACC:
            typeStr = "开户";
            break;
        case DELETEACC:
            typeStr = "销户";break;
        case OPENCARD:
            typeStr = "开卡";break;
        case REPOLOSS:
            typeStr = "挂失";break;
        case CANCELLOSS:
            typeStr = "解挂";break;
        case TOPUP:
            typeStr = "充值";break;
        case PAY:
            typeStr = "消费";break;
        case EMPTY:
            typeStr = "空";break;
        default:
            typeStr = "ERROR";break;
    }
    if(typeflag == 1) {
        if(result == OK) {
            statusStr = "SUCCESS";
        }
        else if(result == FAILED) {
            statusStr = "FAILED";
        }
    }
    else if(typeflag == 0) {
        statusStr = "Window:" + to_string(result);
    }
    valueStream << std::fixed << std::setprecision(2) << value;
    balanceStream << fixed << setprecision(2) << balance;
//    j = snprintf (log, max_len, "%ld StuID: %ld cardID: %d %s | %s value: %.2f balance: %.2f #%s", time, stuNum, cardNum, typeStr, statusStr, value, balance, message);
    log = to_string(time) + " StuId: " + to_string(stuNum) + " cardID: " + to_string(cardNum) + " " + typeStr + " | " + statusStr + " value: " +
            valueStream.str() + "balance: " + balanceStream.str() + "#" + message;
    return log;
}

//传入学生信息及日志信息，生成一条日志并保存进日志文件
int saveOpeLogToFile(Student *stu, int type, int result, float value, long time, const char* message)
{
    string log = generateLog(stu, type, result, value, time, message);
    opeLog << log << "\n";
//    fprintf(logFile, "%s\n", log);
    return OK;
}

//手动开户操作
int openAccount(char *name, long studentNum)
{
    Student *temp;
    Student *target = getStudent(studentNum); //学生在学生数组中的位置指针
    //#TODO 检查输入学号年份和专业是否存在
    if (target != nullptr)
    {
        printf("ERROR: 该学号已开户！\n");
        exit(1);
    }
    temp = initStu(name, studentNum);
    // *target = *temp;
    students[studentNum] = temp; //将生成的学生指针存入unordered_map
    saveOpeLogToFile(temp, OPENACC, OK, 0, 0, "OK");
//    if (temp->head == nullptr)
//    {
//        temp->head = initOpeLog(OPENACC, studentNum, 0, OK, 0, 0); //生成循环链表存储日志 #TODO日志从中间位置存储
//        temp->tail = temp->head;
//    }
//    else
//    {
//        printf("该学生日志不为空，日志记录失败！");
//        return FAILED;
//    }
    return OK;
}

//为指定学生开卡
int openCard(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        Card *card = initCard(studentNum, NORMAL, 0, EXPDATE, "8888"); //默认密码8888
        if (stu->rear == nullptr)
        { //该学生卡片数量为0
            stu->front = card;
            stu->rear = card;
            card->next = nullptr;
            saveOpeLogToFile(stu, OPENCARD, OK, 0, 0, "开卡成功");
        }
        else if (stu->rear->status == LOSS)
        {
            stu->rear->status = BANNED;
            stu->rear->next = card;
            card->balance = stu->rear->balance; //将上张卡余额转入本张卡， 上张卡balance清零
            stu->rear->balance = 0;
            stu->rear = card;
            card->next = nullptr;
            saveOpeLogToFile(stu, OPENCARD, OK, 0, 0, "开卡成功");
        }
        else if (stu->rear->status == BANNED)
        {
//            printf("ERROR: 上张卡被禁用，开卡失败！\n");
            saveOpeLogToFile(stu, OPENCARD, FAILED, 0, 0, "上张卡被禁用");
            exit(FAILED);
        }
        else
        { // 上张卡正常使用，未挂失
//            printf("卡未挂失，请先挂失后开卡！\n");
            saveOpeLogToFile(stu, OPENCARD, FAILED, 0, 0, "卡未挂失");
            return FAILED;
        }
    }
    else
    {
        printf("ERROR: 账户不可用，开卡失败！\n");
//        saveOpeLogToFile(stu, OPENCARD, FAILED, 0, 0, "账户不可用");
        return FAILED;
    }
}

//向学号指定的账户充值，充值成功返回OK，失败返回FAILED
int topupBalance(long studentNum, float topupAmount)
{
    Student *stu = getStudent(studentNum);
    int temp = balanceToInt(topupAmount);
    if (stu->status == NORMAL)
    {
        if (stu->rear == nullptr)
        {
            printf("该学生无卡，请先开卡！\n");
            saveOpeLogToFile(stu, TOPUP, FAILED, topupAmount, 0, "该学生无卡");
            return FAILED;
        }
        else if (stu->rear->status == NORMAL)
        {
            if (temp <= 0)
            {
                saveOpeLogToFile(stu, TOPUP, FAILED, topupAmount, 0, "充值金额需大于0");
//                printf("充值金额需大于0, 充值失败！\n");
                return FAILED;
            }
            if ((temp + stu->rear->balance) >= 100000)
            { //充值后金额大于1000元
                saveOpeLogToFile(stu, TOPUP, FAILED, topupAmount, 0, "卡内余额需小于1000元");
//                printf("卡内余额需小于1000元， 充值失败！\n");
                return FAILED;
            }
            stu->rear->balance += temp;
            saveOpeLogToFile(stu, TOPUP, OK, topupAmount, 0, "充值成功");
//            printf("充值成功！\n");
            return OK;
        }
        else
        {
            saveOpeLogToFile(stu, TOPUP, FAILED, topupAmount, 0, "该学生卡已被挂失或禁用");
//            printf("该学生卡已被挂失或禁用，充值失败！\n");
            return FAILED;
        }
    }
    else
    {
        saveOpeLogToFile(stu, TOPUP, FAILED, topupAmount, 0, "账户被注销或不存在");
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
        if (stu->rear == nullptr)
        {
            saveOpeLogToFile(stu, REPOLOSS, FAILED, 0, 0, "该学生无卡");
//            printf("该学生无卡，请先开卡！\n");
            return FAILED;
        }
        else if (stu->rear->status == NORMAL)
        {
            stu->rear->status = LOSS;
            saveOpeLogToFile(stu, REPOLOSS, OK, 0, 0, "挂失成功");
//            printf("挂失成功！\n");
            return OK;
        }
        else
        {
            saveOpeLogToFile(stu, REPOLOSS, FAILED, 0, 0, "该学生卡已被挂失或禁用");
//            printf("该学生卡已被挂失或禁用，挂失失败！\n");
            return FAILED;
        }
    }
    else
    {
        saveOpeLogToFile(stu, REPOLOSS, FAILED, 0, 0, "账户不可用");
//        printf("ERROR: 账户不可用，挂失失败！\n");
        return FAILED;
    }
}

int cancelLoss(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        if (stu->rear == nullptr)
        {
            saveOpeLogToFile(stu, CANCELLOSS, FAILED, 0, 0, "该学生无卡");
//            printf("该学生无卡，请先开卡！\n");
            return FAILED;
        }
        else if (stu->rear->status == LOSS)
        {
            stu->rear->status = NORMAL;
            saveOpeLogToFile(stu, CANCELLOSS, OK, 0, 0, "解挂成功");
//            printf("解挂成功！\n");
            return OK;
        }
        else
        {
//            printf("该学生卡未挂失或已禁用，解挂失败！\n");
            saveOpeLogToFile(stu, CANCELLOSS, FAILED, 0, 0, "该学生卡未挂失或已禁用");
            return FAILED;
        }
    }
    else
    {
        printf("ERROR: 账户不可用，解挂失败！\n");
        saveOpeLogToFile(stu, CANCELLOSS, FAILED, 0, 0, "账户不可用");
        return FAILED;
    }
}

int deleteAccount(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu == nullptr)
    {
        printf("学号: %ld\n该学生不存在， 注销失败！\n", studentNum);
        return FAILED;
    }
    if (stu->status == NORMAL)
    {
        stu->status = DELETED;
        saveOpeLogToFile(stu, DELETEACC, OK, 0, 0, "该学生不存在");
//        printf("账户注销成功！\n");
        return OK;
    }
    else
    {
        printf("ERROR: 账户不可用，注销失败！\n");
        saveOpeLogToFile(stu, DELETEACC, FAILED, 0, 0, "账户不可用");
        return FAILED;
    }
}

//食堂支付
int pay(int cardNum, float payAmount)
{
    Card *card = getCard(cardNum);
    Student *stu = getStudent(card->studentNum);
    int payInt = balanceToInt(payAmount);
    if (stu->status == NORMAL)
    { //学生账户状态正常
        if (card->status == NORMAL)
        { //卡状态正常
            if (card->balance - payInt >= 0)
            { //卡中余额充足
                card->balance -= payInt;
//                 printf("支付成功！\n");
//                saveOpeLogToFile(stu, PAY, OK, payAmount, 0);
                return OK;
            }
            else
            {
                balanceNotEnough++;
//            printf("卡中余额不足，支付失败!\n");
//                saveOpeLogToFile(stu, PAY, FAILED, payAmount, 0);
                return FAILED;
            }
        }
        else
        {
            cardBanned++;
//        printf("卡已挂失或禁用，支付失败！\n");
//            saveOpeLogToFile(stu, PAY, FAILED, payAmount, 0);
            return FAILED;
        }
    }
    else
    {
        accountNotExisted++;
//        printf("卡号对应学生账户不存在或已注销，支付失败!\n");
//        saveOpeLogToFile(stu, PAY, FAILED, payAmount, 0);
        return FAILED;
    }
}

//初始化窗口日志，使日志从指定位置开始存储
int initWindow(int index, int position)
{
    OpeLog *log = initOpeLog(EMPTY, -1, 0, index, 0, TIMENOW);
    if (windows[index].rear == nullptr)
    { // 窗口日志为空， 生成一个初始日志
        windows[index].rear = log;
        log->next = log;
        windows[index].logQuantity++;
    }
    for (int i = 0; i < position - 2; i++)
    { // 生成position - 2个占位日志
        OpeLog *log = initOpeLog(EMPTY, -1, 0, index, 0, TIMENOW);
        log->next = windows[index].rear->next;
        windows[index].rear->next = log;
        windows[index].rear = log;
        windows[index].logQuantity++;
    }
    return OK;
}

int getHourFromTime(long time) {
    return (int)(time / 1000000 % 100);
}

int getMinsFromTime(long time) {
    return (int)(time / 10000 % 100);
}

bool isSamePeriod(long time1, long time2) {
    if(time1 / 100000000 == time2 / 100000000){
        int hour1 = getHourFromTime(time1);
        int hour2 = getHourFromTime(time2);
        if(hour1 < 10 && hour1 > 6) {
            if(hour2 < 10 && hour2 > 6) {
                return true;
            }
        }
        else if(hour1 > 10 && hour1 < 14) {
            if(hour2 > 10 && hour2 < 14) {
                return true;
            }
        }
        else if(hour1 > 16 && hour1 <20) {
            if(hour2 > 16 && hour2 < 20) {
                return true;
            }
        }
    }
    return false;
}

int getDateFromTime(long time) {
    return time / 100000000 % 10000;
}

//模拟在窗口出消费，输入窗口序号，卡号，支付金额，时间戳
int payAtWindow(int index, int cardNum, float payAmount, long time) //默认密码为8888
{
    abc++;
    Card *card = getCard(cardNum);
    OpeLog *temp = initOpeLog(PAY, getCard(cardNum)->studentNum, cardNum, index, payAmount * 100, time);
    if (pay(cardNum, payAmount) == OK)  //支付成功， 将日志存入对应窗口的循环链表
    {
        if (windows[index].logQuantity < 60000)
        {
            if (windows[index].rear)
            { //该窗口日志不为空
                temp->next = windows[index].rear->next;
                windows[index].rear->next = temp;
                windows[index].rear = temp;
                windows[index].logQuantity++;
            }
            else
            {
                windows->rear = temp;
                temp->next = temp;
                windows[index].logQuantity++;
            }
        }
        else // 60000条日志已满
        {
//            if(getDateFromTime(windows[index].rear->next->time) == 923 && index != 24) {
//                printf("abc");#TODO
//            }
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

int payCheckPwd(int index, int cardNum, float payAmount, long time, int flag = 0) {
    Card *card = getCard(cardNum);
    if (card == nullptr)
    {
        printf("卡号: %d不存在， 支付失败！\n", cardNum);
        return FAILED;
    }
    Student *stu = getStudent(card->studentNum);
//    if(stu->lastConsump == 0) { //上次消费记录不存在，说明为第一次消费
//        stu->lastConsump = time;
//        stu->amountThisPeriod = balanceToInt(payAmount);
//    }
    if(isSamePeriod(stu->lastConsump, time)) {
        stu->lastConsump = time;
        stu->amountThisPeriod += balanceToInt(payAmount);
    }
    else if(!isSamePeriod(stu->lastConsump, time)) {
        stu->lastConsump = time;
        stu->amountThisPeriod = balanceToInt(payAmount);
    }
    if(stu->amountThisPeriod > 2000) {
        if(flag == 0) { //flag为0,默认输入密码8888,用于批量操作
            payAtWindow(index, cardNum, payAmount, time);
        }
        else {
            string password;
            cout << "当前时段支付金额大于20元，请输入密码:";
            cin >> password;
            if(password == card->pwd) {
                payAtWindow(index, cardNum, payAmount, time);
                stu->amountThisPeriod = 0;
            }
            else {
                cout << "密码输入错误，支付取消\n";
            }
        }
    }
    else {
        payAtWindow(index, cardNum, payAmount, time);
    }
}

//从文件中读入信息，进行批量开户操作
int importOpenAccInfo()
{
    FILE *file = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/kh001.txt", "r"); //#TODO change to relative path
    char *num = nullptr;
    char *name = nullptr;
    char *ptr;
    long numLong = 0;
    if (file == nullptr)
    {
        printf("ERROR: 文件路径有误！\n");
        return FAILED;
    }
    char str[30];
    if (fgets(str, 30, file))
    {
        if (!strncmp(str, "KH", 2))
        { //检测验证文件正确性
            while (fgets(str, 30, file))
            { //逐行读入信息
                num = strtok(str, ",");
                name = strtok(nullptr, ";");        //从str中拆分出学号和姓名两部分
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

    char *index = nullptr;
    char *position = nullptr;
    int i;
    int p;
    char str[30];
    if (file == nullptr)
    {
        printf("ERROR: 文件路径有误！");
        return FAILED;
    }
    if (fgets(str, 10, file))
    {
        if (!strncmp(str, "WZ", 2))
        {
            while (fgets(str, 30, file))
            {
                index = strtok(str, ",");
                position = strtok(nullptr, ";");
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
    char *ope = nullptr;
    char *ptr;
    long stuNum;
    int type;
    long time;
    int value = 0; //充值金额
    if (opeFile == nullptr)
    {
        printf("ERROR: 文件路径有误！");
        return FAILED;
    }

    if (fgets(opeStr, 50, opeFile))
    {
        if (!strncmp(opeStr, "CZ", 2))
        {
            while (fgets(opeStr, 50, opeFile))
            {
                time = strtol(strtok(opeStr, ","), &ptr, 10); //时间戳转为long存储
                ope = strtok(nullptr, ",");
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
                    stuNum = strtol((strtok(nullptr, ",")), &ptr, 10);
                    value = balanceToInt(strtof(strtok(nullptr, ";"), nullptr)); //将读入的str转为float再转为int存储
                    Student *stu = getStudent(stuNum);
                    if (stu == nullptr)
                    {
                        printf("ERROR: 学生未开户，请先进行开户后充值\n");
                        return FAILED;
                    }
                    if (stu->rear == nullptr)
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
                stuNum = strtol((strtok(nullptr, ";")), &ptr, 10);
                OpeLog *log = initOpeLog(type, stuNum, 0, 0, value, time);
                windowRec[0].push_back(log);
            }
        }
        return OK;
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
    OpeLog *log = nullptr;
    char *ptr, *p;
    int cardNum;
    char *time1;
    char *time2;
    int windowIndex;
    long time;
    int value;
    if (payFile == nullptr)
    {
        printf("ERROR: 文件路径有误！");
        return FAILED;
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
                    time1 = strtok(nullptr, ",");
                    time2 = strtok(nullptr, ",");

                    char *_time = (char *) malloc(100);
                    memset(_time, 0, 100);
                    strcat(_time, time1);
                    strcat(_time, time2);
                    time = strtol(_time, &ptr, 10);
                    value = balanceToInt(stof(strtok(nullptr, ";")));
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
    cout << "归并消费申请中..." << endl;
    vector<OpeLog *> result;
    priority_queue<OpeLog *, vector<OpeLog *>, comp_time> pq;
    int position[100];
    for (int i = 0; i < 100; i++)
    {
        if (array[i].size() > 0)
        {
            pq.push(array[i][0]);
            position[i] = 0;
        }
    }
    while (!pq.empty())
    {
        OpeLog *tmp = pq.top();
        pq.pop();
        result.push_back(tmp); //#TODO maybe change to queue to increase speed
        if(++position[tmp->result] != array[tmp->result].size()) {
            pq.push(array[tmp->result][position[tmp->result]]);
        }
//        array[tmp->result].erase(array[tmp->result].begin());
//        if (!array[tmp->result].empty())
//        {
//            pq.push(array[tmp->result][0]);
//        }
    }
    cout << "归并消费申请时间: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s" << endl;
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
            payCheckPwd(ope->result, ope->cardNum, (float)(ope->value) / 100, ope->time); //flag置为0,表示为批量操作，默认输入密码8888
//            payAtWindow(ope->result, ope->cardNum, (float)(ope->value) / 100, ope->time, "8888");
            break;
        default:
            break;
    }
    return OK;
}

//根据归并排序后的操作顺序逐个执行操作, 传入result和当前时间
int opeByResult(vector<OpeLog *> result)
{
    balanceNotEnough = 0;
    accountNotExisted = 0;
    cardBanned = 0;
    cout << "执行消费操作中" << endl;
    for (int i = 0; i<result.size() && result[i]->time < TIMENOW; i++)
    {
        executeOpe(result[i]);
    }
    cout << "共支付失败" << balanceNotEnough + accountNotExisted + cardBanned << "次" << endl;
    cout << "其中因余额不足" << balanceNotEnough << "次" << endl;
    cout << "因账户被注销" << accountNotExisted << "次" << endl;
    cout << "因卡被禁用" << cardBanned << "次" << endl;
}

string toRegex(string str) {
    int index;
    while ((index = str.find('?')) != str.npos) {
        str.replace(index, 1, "(.{1,1})");
    }
    while ((index = str.find('*')) != str.npos) {
        str.replace(index, 1, "(.{0,})");
    }
    return str;
}

string toRegex_name(string str) {
    int index;
    while ((index = str.find('?')) != str.npos) {
        str.replace(index, 1, "(.{3,3})"); //#TODO中文在utf8下的正则表达式
    }
    while ((index = str.find('*')) != str.npos) {
        str.replace(index, 1, "(.{0,})");
    }
    return str;
}

void pressAnyKey() {
    cout << "按任意键返回主菜单" << endl;
    getchar();
    getchar();
    outputHome();
}

void searchByStuId(string str) {
    string pattern = toRegex(std::move(str));
    regex patten_re(pattern);
    vector <Student *> result;
    auto iter = students.begin();   //iterator遍历学生寻找与正则表达式匹配的学号
    string out_str;
    while (iter != students.end()) {
        if (regex_match(to_string(iter->second->studentNum), patten_re)) {
            result.push_back(iter->second);
        }
        iter++;
    }
    if(result.empty()) {
        cout << "未找到匹配内容" << endl;
    }
    else {
        for (auto i: result) {
            int cardNum = 0;
            float balance = 0;
            if(i->rear) {
                cardNum = i->rear->cardNum;
                balance = (float)i->rear->balance / 100;
            }
            cout << "学号:" << i->studentNum << "姓名:" << i->name << "卡号:" << cardNum << "余额:" << balance << endl;
        }
    }
    pressAnyKey();
}

//归并并排序食堂窗口日志, 返回指针
vector<OpeLog *> mergeLogs(Window* windows) {
    priority_queue<OpeLog *, vector<OpeLog *>, comp_time> pq;
    OpeLog* ptr[100];
    vector<OpeLog *> result;
    for(int i = 1; i < 100; i++) {
        ptr[i] = windows[i].rear->next;
        if(windows[i].rear != nullptr) {
            while(ptr[i] != windows[i].rear) { //ptr未遍历完整个窗口日志
                if(ptr[i]->type == EMPTY) {
                    ptr[i] = ptr[i]->next; //ptr为占位日志, 跳过
                }
                else {
                    pq.push(ptr[i]); //将该窗口第一个非占位日志push进pq
                    ptr[i] = ptr[i]->next;
                    break;
                }
            }
        }
    }
    while(!pq.empty()) {
        int flag = 0;
        OpeLog *temp = pq.top();
        pq.pop();
        result.push_back(temp);
        if(windows[temp->result].rear->next != ptr[temp->result]) { //windows中仍有剩余log
            while(ptr[temp->result]->type == EMPTY) { //ptr若指向占位日志
                if(ptr[temp->result] == windows[temp->result].rear->next) {
                    flag = 1; //若windows[i]中没有更多的pay日志，flag置为1
                    break;
                }
                ptr[temp->result] = ptr[temp->result]->next;
            }
            if(flag == 0) { //检查ptr中是否还有待导入的日志
                pq.push(ptr[temp->result]);
                ptr[temp->result] = ptr[temp->result]->next;
            }
        }
    }
    return result;
}

vector <OpeLog *> fuzzySearchByMulti(string str, vector<OpeLog*> logs) {
    string delimiter = "/";
    size_t pos = 0;
    string token[6];    /*token[0]:起始时间 token[1]:终止时间 token[2]：学号 token[3]：姓名
                        token[4]:最小金额 token[5]:最大金额*/
    vector<OpeLog*> result;
    int i = 0;
    char* ptr;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token[i++] = str.substr(0, pos);
        str.erase(0, pos + delimiter.length());
    }
    token[i++] = str.substr(0, str.find('~'));
    str.erase(0, str.find('~') + 1);
    token[i] = str;
    long startTime = strtol(token[0].c_str(), &ptr, 10);
    long endTime = strtol(token[1].c_str(), &ptr, 10);
    regex stuID_re(toRegex(token[2]));
    regex name_re(toRegex_name(token[3]));
    int minValue = (int)strtol(token[4].c_str(), &ptr, 10) * 100;
    int maxValue = (int)strtol(token[5].c_str(), &ptr, 10) * 100;
    for(int j = 0; j < logs.size(); j++) {
        if(startTime == 0 || logs[j]->time >= startTime) {
            if(endTime == 0 || logs[j]->time <= endTime) {
                if(regex_match(to_string(logs[j]->studentNum), stuID_re)) {
                    if(regex_match(logs[j]->name, name_re)) {
                        if(minValue == 0 || logs[j]->value >= minValue) {
                            if(maxValue == 0 || logs[j]->value <= maxValue) {
                                result.push_back(logs[j]);
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

void outputSearchResult(vector<OpeLog*> result) {
    for(auto i : result) {
        printf("时间: %ld 姓名: %s 学号: %ld 卡号: %d 消费金额: %.2f 余额: %.2f\n", i->time, i->name, i->studentNum, i->cardNum, (float)i->value,
               (float)getStudent(i->studentNum)->rear->balance / 100);
    }
}

int searchInLogs() {
    clock_t start = clock();
    vector<OpeLog *> logs = mergeLogs(windows);
    cout << "归并消费日志所花费时间" << (clock() - start) / (double)CLOCKS_PER_SEC << "s" << endl;
    string input;
    cout << "输入起始时间，终止时间，学号，姓名，消费金额范围（用/隔开, 范围用~表示）: ";
    cin.ignore();
    getline(cin, input);
    vector<OpeLog *> result = fuzzySearchByMulti(input, logs);
    outputSearchResult(result);
    return OK;
}

void deleteExistInfo() {
    students.clear();
    cards.clear();
    cardSum = 0;
    for(int i = 0; i < 100; i++) {
        windowRec[i].clear();
    }
    importFlag = 0;
    balanceNotEnough = 0;
    accountNotExisted = 0;
    cardBanned = 0;
}

bool isNearTime(long time1, long time2) {
    if(time1 / 100000000 == time2 / 100000000){ //判断年份月份日期相同
        int hour1 = getHourFromTime(time1);
        int hour2 = getHourFromTime(time2);
        int min1 = getMinsFromTime(time1);
        int min2 = getMinsFromTime(time2);
        if(hour1 == hour2) {
            if(min1 - min2 >= -10 && min1 - min2 <= 10) {
                return true;
            }
        }
        else if(hour1 == hour2 + 1) {
            if(min2 - min1 >= 50) {
                return true;
            }
        }
        else if(hour2 == hour1 + 1) {
            if(min1 - min2 >= 55) {
                return true;
            }
        }
    }
    return false;
}

//寻找指定学生的朋友，并返回存储他朋友的vector #TODO更改参数以产生更好的结果
vector <Student *> findFriends(long stuNum) {
    unordered_map<long, int> temp;
    vector <Student *> friends;
    string input = "//";
    input += to_string(stuNum);
    input += "/*/0~999";
    vector<OpeLog *>logs = mergeLogs(windows);
    vector<OpeLog *>stuLogs = fuzzySearchByMulti(input, logs);
    for(auto log : stuLogs) {
        for(int j = log->result - 5; j < log->result + 5; j++) {
            if(j < 1 || j > 99 || !windows[j].rear) {
                continue;
            }
            OpeLog * ptr = windows[j].rear->next;
            do {
                if(isNearTime(ptr->time, log->time)) {
                    temp[ptr->studentNum]++;
                }
                ptr = ptr->next;
            }while(ptr != windows[j].rear->next);
        }
    }
    for(auto i : temp) {
        if(i.second > 4) {
            friends.push_back(getStudent(i.first));
        }
    }
    return friends;
}



void outputImportPart() {
    int i;
    printf("                                ╔════╗\n");
    printf("                                ║信息导入║\n");
    printf("                                ╚════╝\n\n");
    printf("                            ※ 1.从文件导入\n");
    printf("                            ※ 2.从键盘导入\n");
    printf("                          3.上一级     4.退出系统\n");
    printf("________________________________________________________________________________\n");
    printf("请输入功能序号->");
    cin >> i;
    while(i > 3 || i < 1 ) {
        cout << "输入功能序号错误, 请重新输入" << endl;
        cin >> i;
    }
    switch (i) {
        case 1:
            if(importFlag == 0) {  //importFlag代表信息是否已被导入，导入后importFlag置于1
                if(importOpenAccInfo() == OK && importOpeInfo() == OK && importPositionInfo() == OK && importPayInfo() == OK) {
                    vector<OpeLog *> result = mergesort(windowRec);
                    opeByResult(result);
                    cout << "导入信息成功，";
                    importFlag = 1;
                    pressAnyKey();
                }
                else {
                    cout << "导入信息失败，请检查失败原因后重试，";
                    pressAnyKey();
                }
            }
            else {
                cout << "信息已导入，输入1重新导入，输入0返回主菜单" << endl;
                int j;
                cin >> j;
                if(j == 1) {
                    deleteExistInfo();
                    if(importOpenAccInfo() == OK && importOpeInfo() == OK && importPositionInfo() == OK && importPayInfo() == OK) {
                        vector<OpeLog *> result = mergesort(windowRec);
                        opeByResult(result);
                        cout << "导入信息成功，";
                        importFlag = 1;
                        pressAnyKey();
                    }
                    else {
                        cout << "导入信息失败，请检查失败原因后重试，";
                        pressAnyKey();
                    }
                }
                else {
                    outputHome();
                }
            }
            break;
        case 2: {
            int type;
            cout << "0)开户 1)销户 2)开卡/补卡 3)挂失 4)解挂 5)充值 6)支付 7)上一级\n输入操作类型：";
            cin >> type;
            while(type > 7 || type < 0 ) {
                cout << "输入功能序号错误, 请重新输入: " << endl;
                cin >> type;
            }
            switch (type)
            {
                case OPENACC: {
                    string name;
                    cout << "请输入姓名:";
                    cin >> name;
                    long studentNum;
                    cout << "请输入学号:";
                    cin >> studentNum;
                    openAccount(const_cast<char*>(name.c_str()), studentNum);
                    break;
                }
                case OPENCARD: {
                    long studentNum;
                    cout << "请输入学号:";
                    cin >> studentNum;
                    openCard(studentNum);
                    break;
                }
                case DELETEACC: {
                    long studentNum;
                    cout << "请输入学号:";
                    cin >> studentNum;
                    deleteAccount(studentNum);
                    break;
                }
                case REPOLOSS:
                {
                    long studentNum;
                    cout << "请输入学号:";
                    cin >> studentNum;
                    reportLoss(studentNum);
                    break;
                }
                case CANCELLOSS:
                {
                    long studentNum;
                    cout << "请输入学号:";
                    cin >> studentNum;
                    cancelLoss(studentNum);
                    break;
                }
                case TOPUP:
                {
                    long studentNum;
                    cout << "请输入学号:";
                    cin >> studentNum;
                    float value;
                    cout << "请输入充值金额:";
                    cin >> value;
                    topupBalance(studentNum, value);
                    break;
                }
                case PAY:
                {
                    int cardNum;
                    int index;
                    float value;
                    string password;
                    long time = generateTimeNow();
                    cout << "请输入卡号:";
                    cin >> cardNum;
                    cout << "请输入消费窗口序号(1-99):";
                    cin >> index;
                    cout << "请输入消费金额:";
                    cin >> value;
                    Card* card = getCard(cardNum);
                    while(card == nullptr) {
                        cout << "卡号不存在，请重新输入卡号:";
                        cin >> cardNum;
                        card = getCard(cardNum);
                    }
                    payCheckPwd(index, cardNum, (float)value, time, 1);
//                        payAtWindow(index, cardNum, (float)value / 100, time);
                    break;
                }
                case 7:
                    outputImportPart();
                default:
                    break;
            }
        }
        case 3:
            outputHome();
            break;
        case 4:
            exit(0);
        default:
            break;
    }
}

int analyWindowInfo(long date, int index) {
    OpeLog * p = windows[index].rear->next;
    if(20000000 > date || 20300000 < date) {
        printf("日期输入错误");
        return FAILED;
    }
    long start = date * 100000000;
    long end = date * 100000000 + 99999999;
    float totalValue = 0;
    int times = 0;
    do{
        if(p->time >= start && p->time <= end) {
            totalValue += (float)p->value / 100;
            times++;
            p = p->next;
        }
        else
            p = p->next;
    }while(p != windows[index].rear->next);
    printf("%d号于%d年%d月%d日, 共消费%d次，消费总金额%.2f元\n", index, date / 10000, date / 100 % 100, date % 100, times, totalValue);
    return OK;
}

void outputStudents(vector<Student*> result) {
    for(auto i : result) {
        printf("学号: %ld 姓名: %s", i->studentNum, i->name);
    }
}

void outputSearchPart() {
    int i;
    string stuNum;
    printf("                                ╔════╗\n");
    printf("                                ║信息查询║\n");
    printf("                                ╚════╝\n\n");
    printf("                            ※ 1.查询学生信息\n");
    printf("                            ※ 2.查询消费记录\n");
    printf("                            ※ 3.查询窗口交易次数/金额\n");
    printf("                            ※ 4.查询学生的朋友\n");
    printf("                          5.上一级     6.退出系统\n");
    printf("________________________________________________________________________________\n");
    printf("请输入功能序号->");
    cin >> i;
    while(i > 6 || i < 1 ) {
        cout << "输入功能序号错误, 请重新输入: " << endl;
        cin >> i;
    }
    switch (i) {
        case 1:
            cout << "请输入学号(支持模糊搜索): " << endl;
            cin >> stuNum;
            searchByStuId(stuNum);
            break;
        case 2:
            if(importFlag == 0) {
                cout << "还未导入消费信息，仍要搜索?(输入1继续, 输入其他返回主菜单)" << endl;
                int j;
                cin >> j;
                if(j != 1) {
                    outputHome();
                }
                else {
                    searchInLogs();
                }
            }
            else {
                searchInLogs();
            }
            break;
        case 3: {
            long date;
            int index;
            cout << "请输入窗口号：" ;
            cin >> index;
            cout << "请输入查询日期：";
            cin >> date;
            analyWindowInfo(date, index);
            pressAnyKey();
        }
        case 4:{
            long studentNum;
            cout << "请输入学号: \n";
            cin >> studentNum;
            vector<Student *>result = findFriends(studentNum);
            outputStudents(result);
        }
        case 5:
            outputHome();
            break;
        case 6:
            exit(1);
        default:
            break;
    }
}

int generatePayLogFile() {
    vector<OpeLog *>result = mergeLogs(windows);
    fstream payLogFile;
    payLogFile.open("/home/z1youra/CLionProjects/cardManagement/log/pay_log.txt", ios::out);
    for(auto i : result) {
        string log = generateLog(getStudent(i->studentNum), PAY, i->result, i->value, i->time, "支付成功", 0);
        string valid = "Valid:" + to_string(generateValid(i)) + "\n";
        payLogFile << log << valid;
    }
    return OK;
}

void outputAnalyPart() {
    int i;
    printf("                                ╔════╗\n");
    printf("                                ║信息导入║\n");
    printf("                                ╚════╝\n\n");
    printf("                            ※ 1.生成消费日志文件\n");
    printf("                            ※ 2.从键盘导入\n");
    printf("                          3.上一级     4.退出系统\n");
    printf("________________________________________________________________________________\n");
    printf("请输入功能序号->");
    cin >> i;
    while(i > 4 || i < 1 ) {
        cout << "输入功能序号错误, 请重新输入: " << endl;
        cin >> i;
    }
    switch (i) {
        case 1:
            generatePayLogFile();
    }
}

void outputHome() {
    int i;
    cout << "              [欢迎进入校园卡管理信息系统!]" << endl << endl;
    cout << "              ※ 1.信息导入" << endl;
    cout << "              ※ 2.信息查询" << endl;
    cout << "              ※ 3.信息统计" << endl;
    cout << "              ※ 4.退出系统" << endl;
    cout << "________________________________________________________________________________" << endl;
    cout << "请输入功能序号" << endl;
    cin >> i;
    while(i > 4 || i < 1 ) {
        cout << "输入功能序号错误, 请重新输入: " << endl;
        cin >> i;
    }
    switch (i) {
        case 1: outputImportPart();
            break;
        case 2: outputSearchPart();
            break;
        case 3:outputAnalyPart();
            break;
        case 4:
            exit(1);
            break;
        default:
            break;
    }
}

void cliSys() {
    outputHome();
}

int main()
{
    opeLog.open("/home/z1youra/CLionProjects/cardManagement/log/ope_log.txt", ios::out);
    initStatus();
    cliSys();
    return 0;
}