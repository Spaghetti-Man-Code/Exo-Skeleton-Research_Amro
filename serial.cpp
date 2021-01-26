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
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    //timeouts.WriteIntervalTimeout = 50;
    timeouts.WriteTotalTimeoutConstant = 50;
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



    while(true){
        int i = 0;
        
        if(start == 0){
            WriteFile(hSerial, write_buffer, 6, &bytes_written, NULL);
            
            // start timer here after sending the last byte
            printf("Number of bytes written: %d\n", bytes_written);
            start = 1;

        }

        ReadFile(hSerial, &reading_buffer[i], 1, &bytes_read, NULL);
        while (bytes_read > 0) {

            // stop timer here after recieving the first byte

            i++;
            ReadFile(hSerial, &reading_buffer[i], 1, &bytes_read, NULL);
        }

        if(i > 0){
            memcpy(&data_length, &reading_buffer[0], 1 * 1 * sizeof(char));
            memcpy(&time, &reading_buffer[1], 4 * 1 * sizeof(char));
            memcpy(IMU_data, &reading_buffer[5], 2 * 49 * sizeof(char));
            memcpy(gait_data, &reading_buffer[103], 1 * 2 * sizeof(char));
            memcpy(slip_detection_data, &reading_buffer[105], 2 * 2 * sizeof(char));
            memcpy(control_torque_data, &reading_buffer[109], 2 * 2 * sizeof(char));
            memcpy(teensy_data, &reading_buffer[113], 2 * 49 * sizeof(char));
            
            // printing data
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
