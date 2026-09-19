/*标准设备读入计算表达式并从标准设备输出结果*/
/*读入可以是一个赋值语句，左端是一个符号名右端是表达式*/
/*表达式包含四则运算、已经赋值的符号名和预定义的符号常量（pi和e）也可以只有单个整数和常数值*/
/*输入内容不符或将导致非法计算时输出出错提示并计算出错次数*/

#define pi (double)3.14159265358979323846
#define e (double)2.71828182845904523536

#include<stdio.h>
#include<math.h>
#include<ctype.h>
#include<string.h>

// ========== 符号表：存储已赋值的变量 ==========
char varNames[26];      // 变量名数组（支持a-z共26个字母）
double varValues[26];   // 变量值数组（与varNames一一对应）
int varCount = 0;       // 当前已存储的变量数量

// ========== 错误统计 ==========
int errorCount = 0;     // 出错次数统计

/**
 * 查找变量的值
 * @param name 变量名（单个字母）
 * @return 变量对应的值，如果未找到则返回0并输出错误提示
 */
double getVarValue(char name) {
    // 遍历符号表查找匹配的变量名
    for (int i = 0; i < varCount; i++) {
        if (varNames[i] == name) {
            return varValues[i];  // 找到则返回对应的值
        }
    }
    // 未找到该变量
    printf("Error: Undefined variable '%c'\n", name);
    errorCount++;
    return 0;
}

/**
 * 存储或更新变量的值
 * @param name 变量名（单个字母）
 * @param value 要存储的值
 */
void setVarValue(char name, double value) {
    // 先查找变量是否已存在
    for (int i = 0; i < varCount; i++) {
        if (varNames[i] == name) {
            varValues[i] = value;  // 已存在则更新值
            return;
        }
    }
    // 不存在则添加新变量
    varNames[varCount] = name;
    varValues[varCount] = value;
    varCount++;  // 变量数量加1
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

double appLyop(double a,double b,char op){
    if(op =='/' && b ==0){
        printf("Error: Division by zero\n");
        errorCount++;
        return 0;
    }
    switch(op){
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b; 
    }
    return 0;
}

double evaluate(char *tokens){
    double numstack[100];
    char opstack[100];
    int numTop = -1;
    int opTop = -1;
    for(int i =0;tokens[i] != '\0';i++){
        if(tokens[i] == ' '){
            continue;
        }
        else if(tokens[i] == 'e'){
            numstack[++numTop] = e;
        }
        else if(tokens[i] == 'p' && tokens[i+1] == 'i'){
            numstack[++numTop] = pi;
            i++;
        }
        else if(isalpha(tokens[i]) && tokens[i] != 'e' && tokens[i] != 'p'){
            numstack[++numTop] = getVarValue(tokens[i]);
        }
        else if(isdigit(tokens[i]) || tokens[i] == '.'){
            double num = 0;
            double decimal = 0;
            double decimalPlace = 1;
            while(isdigit(tokens[i]) || tokens[i] == '.'){
                if(tokens[i] == '.'){
                    decimal = 1;
                } else if(decimal == 0){
                    num = num * 10 + (tokens[i] - '0');
                } else {
                    decimalPlace *= 10;
                    num = num + (tokens[i] - '0') / decimalPlace;
                }
                i++;
            }
            numstack[++numTop] = num;
            i--;
        }
        else if(tokens[i] == '('){
            opstack[++opTop] = '(';
        }
        else if(tokens[i] == ')'){
            // 检查是否有匹配的左括号
            int hasLeftBracket = 0;
            for (int j = opTop; j >= 0; j--) {
                if (opstack[j] == '(') {
                    hasLeftBracket = 1;
                    break;
                }
            }
            if (!hasLeftBracket) {
                printf("Error: Mismatched ')'\n");
                errorCount++;
                return 0;
            }
            while(opTop >= 0 && opstack[opTop] != '('){
                double b = numstack[numTop--];
                double a = numstack[numTop--];
                char op = opstack[opTop--];
                numstack[++numTop] = appLyop(a, b, op);
            }
            if (opTop >= 0) opTop--;  // 弹出左括号（防止栈空时越界）
        }
        else if(tokens[i] == '+' || tokens[i] == '-' || tokens[i] == '*' || tokens[i] == '/'){
            while (opTop >= 0 && precedence(opstack[opTop]) >= precedence(tokens[i])) {
                double b = numstack[numTop--];
                double a = numstack[numTop--];
                char op = opstack[opTop--];
                numstack[++numTop] = appLyop(a, b, op);
            }
            opstack[++opTop] = tokens[i];
        }
        else {
            // 非法字符
            printf("Error: Invalid character '%c'\n", tokens[i]);
            errorCount++;
            return 0;
        }
    }
    // 检查括号是否匹配
    for (int i = 0; i <= opTop; i++) {
        if (opstack[i] == '(') {
            printf("Error: Mismatched '('\n");
            errorCount++;
            return 0;
        }
    }
    // 处理剩余的运算符
    while(opTop >= 0){
        char op = opstack[opTop--];
        // 跳过左括号（括号应该已匹配，但防止异常情况）
        if (op == '(') continue;
        double b = numstack[numTop--];
        double a = numstack[numTop--];
        numstack[++numTop] = appLyop(a, b, op);
    }
    return numstack[numTop];
}

int main(){
    // 读取用户输入的一行字符串
    char str[100];
    fgets(str, sizeof(str), stdin);
    
    // 去除末尾的换行符
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
    
    // 查找等号位置，判断是否为赋值语句
    char *eqPos = strchr(str, '=');
    
    if (eqPos != NULL) {
        // ========== 有等号：赋值语句 ==========
        
        // 将等号替换为字符串结束符，分割左右两部分
        *eqPos = '\0';
        
        // 左边：变量名（跳过前导空格）
        char *varName = str;
        while (*varName == ' ') varName++;
        
        // 右边：赋值值 和 可能的表达式
        char *rightSide = eqPos + 1;
        
        // 查找右边最后一个空格，判断是否有后续表达式
        char *spacePos = strrchr(rightSide, ' ');
        
        if (spacePos != NULL) {
            // ========== 有空格：赋值 + 计算 ==========
            // 格式: x = 3.2 x*8
            
            // 将空格替换为字符串结束符，分割赋值值和表达式
            *spacePos = '\0';
            
            char *valueStr = rightSide;    // 赋值的值部分
            char *expr = spacePos + 1;     // 要计算的表达式部分
            
            // 跳过前导空格
            while (*valueStr == ' ') valueStr++;
            while (*expr == ' ') expr++;
            
            // 计算赋值部分的值
            double value = evaluate(valueStr);
            
            // 如果有错误，不输出结果
            if (errorCount > 0) {
                // 不输出结果
            } else {
                // 验证变量名是否合法（单个字母，且不是e或p）
                if (isalpha(varName[0]) && varName[0] != 'e' && varName[0] != 'p') {
                    // 存储变量到符号表
                    setVarValue(varName[0], value);
                    
                    // 使用已赋值的变量计算表达式
                    double result = evaluate(expr);
                    if (errorCount == 0) {
                        printf("%g\n", result);
                    }
                } else {
                    printf("Error: Invalid variable name\n");
                }
            }
        } else {
            // ========== 没有空格：只赋值，不计算 ==========
            // 格式: x = 3.2
            
            char *valueStr = rightSide;
            while (*valueStr == ' ') valueStr++;
            
            // 计算赋值部分的值
            double value = evaluate(valueStr);
            
            // 如果有错误，不输出结果
            if (errorCount > 0) {
                // 不输出结果
            } else {
                // 验证变量名并存储
                if (isalpha(varName[0]) && varName[0] != 'e' && varName[0] != 'p') {
                    setVarValue(varName[0], value);
                    printf("%c = %g\n", varName[0], value);
                } else {
                    printf("Error: Invalid variable name\n");
                }
            }
        }
    } else {
        // ========== 没有等号：直接计算表达式 ==========
        // 格式: 3*e+1
        
        double result = evaluate(str);
        // 如果有错误，不输出结果
        if (errorCount == 0) {
            printf("%g\n", result);
        }
    }
    
    // 输出出错次数
    if (errorCount > 0) {
        printf("Total errors: %d\n", errorCount);
    }
    
    // 等待用户按回车键退出
    printf("Press Enter to exit");
    getchar();
    return 0;
}