//
// serial.c / serial.cpp
//
// To compile with MinGW:
//
//      gcc -o serial.exe serial.c
//
// To compile with cl, the Microsoft compiler:
//
//      cl serial.cpp
//
// To run:
//
//      serial.exe
//
 
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
 
int main()
{
 
    printf("stage 1...\n");
    // Declare variables and structures
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};
         
    // Open the highest available serial port number
    fprintf(stderr, "Opening serial port...");
    hSerial = CreateFile("\\\\.\\COM3", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
            fprintf(stderr, "Error\n");
            return 1;
    }
    else fprintf(stderr, "OK\n");
     
    // Set device parameters (115200 baud, 1 start bit, 1 stop bit, no parity)
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error getting device state\n");
        CloseHandle(hSerial);
        return 1;
    }
     
    dcbSerialParams.BaudRate = 115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if(SetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error setting device parameters\n");
        CloseHandle(hSerial);
        return 1;
    }
 
    // Set COM port timeout settings
    timeouts.ReadIntervalTimeout = 10;
    timeouts.ReadTotalTimeoutConstant = 10;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    
    timeouts.WriteTotalTimeoutConstant = 10;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if(SetCommTimeouts(hSerial, &timeouts) == 0){
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hSerial);
        return 1;
    }


    bool running = true;
    char * reading_buffer = (char *)malloc(1024*sizeof(char));
    DWORD bytes_read = 0;


    uint8_t data_length = 0;
    int time = 0;
    int16_t IMU_data[49] = { 0 };
    int8_t gait_data[2] = { 0 };
    int16_t slip_detection_data[2] = { 0 };
    int16_t control_torque_data[2] = { 0 };
    int16_t teensy_data[49] = { 0 };

    
    printf("stage 2...\n");
    char * write_buffer = "start\n";
    DWORD bytes_written = 0;
    int start = 0;
    clock_t start1 = 0, start2 = 0, end1 = 0, end2 = 0;



    while(true){
        int i = 0;
        
        if(start == 0){
            WriteFile(hSerial, write_buffer, 6, &bytes_written, NULL);

            // start timer here after sending the last byte
            clock_t start1 = clock();

            printf("Number of bytes written: %d\n", bytes_written);
            start = 1;

        }

        ReadFile(hSerial, &reading_buffer[i], 1, &bytes_read, NULL);
        if(bytes_read > 0){
            // stop timer here after recieving the first byte
            clock_t end1 = clock();

            // start clock to see how long it takes for all the data to send
            clock_t start2 = clock();
        }
        while (bytes_read > 0){
            i++;
            ReadFile(hSerial, &reading_buffer[i], 1, &bytes_read, NULL);
        }

        if(i > 0){
            clock_t end2 = clock();

            memcpy(&data_length, &reading_buffer[0], 1 * 1 * sizeof(char));
            memcpy(&time, &reading_buffer[1], 4 * 1 * sizeof(char));
            memcpy(IMU_data, &reading_buffer[5], 2 * 49 * sizeof(char));
            memcpy(gait_data, &reading_buffer[103], 1 * 2 * sizeof(char));
            memcpy(slip_detection_data, &reading_buffer[105], 2 * 2 * sizeof(char));
            memcpy(control_torque_data, &reading_buffer[109], 2 * 2 * sizeof(char));
            memcpy(teensy_data, &reading_buffer[113], 2 * 49 * sizeof(char));
            
            // printing data
            float seconds1 = (float)(((float)((float)(end1 - start1) * (1000)))/ CLOCKS_PER_SEC);
            float seconds2 = (float)(((float)((float)(end2 - start2) * (1000)))/ CLOCKS_PER_SEC);
            
            printf("time it took to send to read 1 byte: %f milliseconds\n", seconds1);
            printf("time it took to recieve rst of data from Pi: %f milliseconds\n", seconds2);

            printf("\ndata length: %d\n", data_length);
            printf("time: %d\n", time);
        
            int j = 0;
        
            printf("IMU data: \n\t");
            for (j = 0; j < 49; j++) {
                printf("\t%d, ", IMU_data[j]);
            }
        
            printf("\ngait data: \n\t");
            for (j = 0; j < 2; j++) {
                printf("\t%d, ", gait_data[j]);
            }

            printf("\nslip detection data: \n\t");
            for (j = 0; j < 2; j++) {
                printf("\t%d, ", slip_detection_data[j]);
            }

            printf("\ncontrol torque data: \n\t");
            for (j = 0; j < 2; j++) {
                printf("\t%d, ", control_torque_data[j]);
            }

            printf("\nteensy data: \n\t");
            for (j = 0; j < 49; j++) {
                printf("\t%d, ", teensy_data[j]);
            }printf("\n");

        }            

    }

    printf("stage 4...\n");
    // Close serial port
    fprintf(stderr, "Closing serial port...");
    if (CloseHandle(hSerial) == 0)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    fprintf(stderr, "OK\n");
 
    // exit normally
    return 0;
}
