#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

#define MAX_PATH_LENGTH 1024



static char program_name[] = "tpkmain";

static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", no_argument, 0, 'v'},
    {"output", required_argument, 0, 'o'},
    {"trace", no_argument, 0, 't'},
    {"temperature-sensor", no_argument, 0, 'T'},
    {"cpu-temperature", no_argument, 0, 'C'},
    {"real-time-clock", no_argument, 0, 'R'},
    {"hardware-version", no_argument, 0, 'H'},
    {"led-test", no_argument, 0, 'L'},
    {"emmc-test", no_argument, 0, 'E'},
    {"can1-test", no_argument, 0, '1'},
    {"can2-test", no_argument, 0, '2'},
    {"rs485-1-test", no_argument, 0, '3'},
    {"rs485-2-test", no_argument, 0, '4'},
    {"gigabit-ethernet-test", no_argument, 0, '5'},
    {"usb2-1-test", no_argument, 0, '6'},
    {"usb2-2-test", no_argument, 0, '7'},
    {"usb3-test", no_argument, 0, '8'},
    {0, 0, 0, 0}
};

void usage(void)
{
    fprintf(stdout, "Usage: %s [options]\n",program_name);
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "  -h, --help\t\t显示帮助信息\n");
    fprintf(stdout, "  -v, --verbose\t\t详细模式\n");
    fprintf(stdout, "  -o, --output\t是否输出日志文件\n");
    fprintf(stdout, "  -t, --trace\t\t是否在屏幕上显示测试过程与结果\n");
    fprintf(stdout, "  -T, --temperature-sensor\t单板温度传感器测试\n");
    fprintf(stdout, "  -C, --cpu-temperature\tCPU内部温度传感器测试\n");
    fprintf(stdout, "  -R, --real-time-clock\t实时时钟测试\n");
    fprintf(stdout, "  -H, --hardware-version\t硬件版本功能测试\n");
    fprintf(stdout, "  -L, --led-test\t\tLED测试\n");
    fprintf(stdout, "  -E, --emmc-test\t\tEMMC硬盘读写测试\n");
    fprintf(stdout, "  -1, --can1-test\t\t背板第一路CAN通讯测试\n");
    fprintf(stdout, "  -2, --can2-test\t\t背板第二路CAN通讯测试\n");
    fprintf(stdout, "  -3, --rs485-1-test\t背板第一路RS485通讯测试\n");
    fprintf(stdout, "  -4, --rs485-2-test\t背板第二路RS485通讯测试\n");
    fprintf(stdout, "  -5, --gigabit-ethernet-test\t背板千兆以太网通讯测试\n");
    fprintf(stdout, "  -6, --usb2-1-test\t背板第一路USB2.0测试\n");
    fprintf(stdout, "  -7, --usb2-2-test\t背板第二路USB2.0测试\n");
    fprintf(stdout, "  -8, --usb3-test\t前面USB3.0测试\n");
}



typedef struct {  
    char **paths; // Array of paths  
    int count;    // Number of paths found  
    int capacity; // Total capacity of the array  
} PathList;  
  
// Function to add a path to the PathList  
void addPath(PathList *list, const char *path) {  
    if (list->count >= list->capacity) {  
        // Double the capacity if not enough space  
        list->capacity = (list->capacity == 0) ? 8 : list->capacity * 2;  
        char **temp = realloc(list->paths, list->capacity * sizeof(char *));  
        if (temp == NULL) {  
            perror("Failed to allocate memory");  
            exit(EXIT_FAILURE);  
        }  
        list->paths = temp;  
    }  
    // Duplicate the path string  
    list->paths[list->count] = strdup(path);  
    if (list->paths[list->count] == NULL) {  
        perror("Failed to duplicate path string");  
        exit(EXIT_FAILURE);  
    }  
    list->count++;  
}  
  
// Function to free the PathList  
void freePathList(PathList *list) {  
    for (int i = 0; i < list->count; i++) {  
        free(list->paths[i]); // Free the duplicated path string  
    }  
    free(list->paths); // Free the array of paths  
    list->count = 0;  
    list->capacity = 0;  
}  
  

/**  
 * @brief Find files in a specified directory using the 'find' command.  
 *   
 * @param base_path The base directory to start searching from.  
 * @param filename The name of the file to search for.  
 * @param list A pointer to a PathList where found paths will be stored.  
 * @return int Returns 0 if files are found and paths are added to the list, -1 otherwise.  
 *   
 * @example  
 * PathList list = {NULL};  
 * if (find("/home/user", "example.txt", &list) == 0) {  
 *     // Successfully found files, process list...  
 * } else {  
 *     // No files found  
 * }  
 * freePathList(&list); // Don't forget to free the allocated memory!  
 *   
 * @note  
 * - Ensure that the calling process has sudo privileges, as the 'find' command is run with sudo.  
 * - Handle errors appropriately, especially when using system() and memory allocation.  
 */  
int find(const char* base_path, const char* filename, PathList *list) {  
    char cmd[256];  
    snprintf(cmd, sizeof(cmd), "sudo bash -c \"find %s -name \"%s\"\"", base_path, filename);  
  
    // Use popen to execute the command and read the output  
    FILE *fp = popen(cmd, "r");  
    if (fp == NULL) {  
        perror("popen");  
        return -1;  
    }  
  
    char buffer[256];  
    int found = 0;  
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {  
        // Remove newline character at the end of the string if it exists  
        buffer[strcspn(buffer, "\n")] = 0;  
          
        // Add the path to the list  
        addPath(list, buffer);  
        found = 1;  
    }  
  
    // Close the pipe  
    pclose(fp);  
  
    // Return 0 if files were found, -1 if not  
    return found ? 0 : -1;  
}

// Function to test the single board temperature sensor
void testSingleBoardTemperatureSensor() {
    // Find the path of the "temp1_input" file
    const char* base_path = "/sys";
    const char* filename = "temp1_input";
    PathList list = {NULL, 0, 0};

    find(base_path, filename, &list);
    // Check if the file was found
    if (list.count==0) {
        printf("Error: File 'temp1_input' not found.\n");
        return;
    }

    printf("Found %d files named '%s':\n", list.count, filename);

    for (int i = 0; i < list.count; i++) {
        // Try to read the content of the file and log any errors
        FILE *file = fopen(list.paths[i], "r");
        if (file == NULL) {
            printf("Error: Failed to open the temperature file \"%s\".\n",list.paths[i]);
            return;
        }

        char buffer[128];
        if (fgets(buffer, sizeof(buffer), file) == NULL) {
            printf("Error: Failed to read the temperature file \"%s\".\n",list.paths[i]);
            fclose(file);
            return;
        }
        fclose(file);

        // Log the temperature reading
        int tempInMilliDegrees = atoi(buffer);  
        float tempInDegrees = (float)tempInMilliDegrees / 1000.0;  
        printf("temperature at %s is %.3f degrees Celsius\n",list.paths[i],tempInDegrees);
        
    }

    freePathList(&list);

    // Log success message
    printf("Temperature sensor test script completed successfully.");
}



void testCPUInternalTemperatureSensor(void)
{
    // Find the path of the "temp1_input" file
    const char* base_path = "/sys";
    const char* filename = "temp";
    PathList list = {NULL, 0, 0};

    find(base_path, filename, &list);
    // Check if the file was found
    if (list.count==0) {
        printf("Error: File 'temp' not found.\n");
        return;
    }

    printf("Found %d files named '%s':\n", list.count, filename);


    for (int i = 0; i < list.count; i++) {
        // Try to read the content of the file and log any errors
        FILE *file = fopen(list.paths[i], "r");
        if (file == NULL) {
            printf("Error: Failed to open the cpu temperature file \"%s\".\n",list.paths[i]);
            return;
        }

        
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            int temp_raw = atoi(buffer);
            int temp_celsius = temp_raw / 1000;
            int temp_fraction = temp_raw % 1000;


            int tempInMilliDegrees = atoi(buffer);  
            float tempInDegrees = (float)tempInMilliDegrees / 1000.0;  
            printf("cpu temperature at %s is %.3f degrees Celsius\n",list.paths[i],tempInDegrees);
        }else{
            printf("Error: Failed to read the cpu temperature file \"%s\".\n",list.paths[i]);
            fclose(file);
            return;
        }
        fclose(file);

    }

    freePathList(&list);

    return 0;
}



// 实时时钟测试
void testRealTimeClock(void)
{
    
}

// 硬件版本功能测试
void testHardwareVersion(void);

// LED测试
void testLED(void);

// EMMC硬盘读写测试
void testEMMCDiskReadWrite(void);

// 背板第一路CAN通讯测试
void testBackplaneFirstCANCommunication(void);

// 背板第二路CAN通讯测试
void testBackplaneSecondCANCommunication(void);

// 背板第一路RS485通讯测试
void testBackplaneFirstRS485Communication(void);

// 背板第二路RS485通讯测试
void testBackplaneSecondRS485Communication(void);

// 背板千兆以太网通讯测试
void testBackplaneGigabitEthernetCommunication(void);

// 背板第一路USB2.0测试
void testBackplaneFirstUSB20(void);

// 背板第二路USB2.0测试
void testBackplaneSecondUSB20(void);

// 前面USB3.0测试
void testFrontUSB30(void);



int main(int argc, char *argv[]) {
    int option_index = 0;
    int c;
    int verbose_flag=0; // 默认省略模式
    int trace_flag=0; //默认无回显
    char *log_file = NULL; // 默认不输出日志文件

    char ops[100]=""; // 操作选择记录表
    int ops_num=0; // 当前记录的操作数目


    // 解析输入，以收集数据、配置环境、记录操作序列
    while ((c = getopt_long(argc, argv, "hvRo:tTCRHL1E2345678", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                usage();
                //exit(EXIT_SUCCESS);
                break;
            case 'v':
                verbose_flag=1;
                printf("Verbose mode\n");
                break;
            case 'o':
                log_file = optarg;
                printf("Output log file: %s\n", optarg);
                break;
            case 't':
                trace_flag=1;
                printf("Trace mode");
                break;
            case 'T':
                ops[ops_num]='T';
                ops_num++;
                break;
            case 'C':
                ops[ops_num]='C';
                ops_num++;
                break;
            case 'R':
                ops[ops_num]='R';
                ops_num++;
                break;
            case 'H':
                ops[ops_num]='H';
                ops_num++;
                break;
            case 'L':
                ops[ops_num]='C';
                ops_num++;
                break;
            case 'E':
                ops[ops_num]='E';
                ops_num++;
                break;
            case '1':
                ops[ops_num]='1';
                ops_num++;
                break;
            case '2':
                ops[ops_num]='2';
                ops_num++;
                break;
            case '3':
                ops[ops_num]='3';
                ops_num++;
                break;
            case '4':
                ops[ops_num]='4';
                ops_num++;
                break;
            case '5':
                ops[ops_num]='5';
                ops_num++;
                break;
            case '6':
                ops[ops_num]='6';
                ops_num++;
                break;
            case '7':
                ops[ops_num]='7';
                ops_num++;
                break;
            case '8':
                ops[ops_num]='8';
                ops_num++;
                break;
            case '?':
                printf("Unknown option: %c\n", optopt);
                break;
            default:
                printf("?? getopt returned character code 0%o ??\n", c);
                abort();
        }
    }


    for (int index = optind; index < argc; index++) {
        printf("Non-option argument %s\n", argv[index]);
    }

    // 根据操作记录情况，按照操作输入次序来执行相关函数
    for(int i=0;i<ops_num;i++){
        switch (ops[i]) {
            case 'T':
                printf("单板温度传感器测试\n");
                testSingleBoardTemperatureSensor();
                break;
            case 'C':
                printf("CPU内部温度传感器测试\n");
                testCPUInternalTemperatureSensor();
                break;
            case 'R':
                printf("实时时钟测试\n");;
                break;
            case 'H':
                printf("硬件版本功能测试\n");
                break;
            case 'L':
                printf("LED测试\n");
                break;
            case 'E':
                printf("EMMC硬盘读写测试\n");
                break;
            case '1':
                printf("背板第一路CAN通讯测试\n");
                break;
            case '2':
                printf("背板第二路CAN通讯测试\n");
                break;
            case '3':
                printf("背板第一路RS485通讯测试\n");
                break;
            case '4':
                printf("背板第二路RS485通讯测试\n");
                break;
            case '5':
                printf("背板千兆以太网通讯测试\n");
                break;
            case '6':
                printf("背板第一路USB2.0测试\n");
                break;
            case '7':
                printf("背板第二路USB2.0测试\n");
                break;
            case '8':
                printf("前面USB3.0测试\n");
                break;
        }
    }

    return 0;
}