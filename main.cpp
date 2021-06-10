#include <mbed.h>
#include <string.h>
#include <USBSerial.h>
#include <I2C.h>
#include <DigitalIn.h>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

USBSerial serial;
I2C i2c(PB_7,PB_6);   //SDA: PB_7 ,SCL: PB_6


const int add_write = 0x30;
const int add_read = 0x31;

char data[4];
u_int32_t pressure_data[4];

int main() {

    //write textfile 
    int flag = 1;   // indicator for increasing pressure , if flag ==0: measuring decreasing pressure
    int i= 0;
    int num = 1;
    int m_final_index = 0;
    float data_arr[1500] = {0.00};
    float prev_data = 0;
    char cmd[3];

    serial.printf("-------pressure mesure is started---------\n\n");
    while(1){
        cmd[0] = 0xAA;      
        cmd[1] = 0x00;      
        cmd[2] = 0x00;      

        // I2C Communication
        i2c.write(add_write, cmd, 3); //8-bit I2C slave add , pointer to the byte-arr data to send, (send 3 bytes)

        wait_ms(5); // wait at least 5ms 

        i2c.read(add_read, data, 1);
        
        wait_ms(5);

        // // read status bytes 
        i2c.read(add_read, data, 4); // 8-bit I2C slave add, pointer to the byte-arr to read data in to, Number of Bytes to read (read from an I2C slave)

          pressure_data[0] = data[0];
          pressure_data[1] = (u_int32_t)data[1] << 16;
          pressure_data[2] = (u_int32_t)data[2] << 8;
          pressure_data[3] = (u_int32_t)data[3];
          
          // Transfer function 
      
          // Variables for transfer function
          float output = float(pressure_data[1]|pressure_data[2]|pressure_data[3]); // float or int

          float output_max = 3774873.6;  //3774873.6  :  22.5 %
          float output_min = 419430.4;   //419430.4    :  2.5  %
          int p_max = 300;

          // // Apply transfer function
          float pressure_result = ((output - output_min)*(p_max))/(output_max- output_min);
          // -----------measure done --------------
        
        // conditions 
        if (pressure_result <180){                      // < 180 mmHg
          if (flag ==1){                                // flag == 1 : pressure has not been reached 150 mmHg
            //serial.printf("i value   %i \n",i);
            data_arr[i] = float(pressure_result);
            serial.printf("data arr val %3.2f \n",data_arr[i]);
            serial.printf("%3.2f\n", pressure_result);

          }
  
          else{                                        // flag == 0:  pressure decreasing
            data_arr[i] = (float)pressure_result;
            serial.printf("data arr val %3.2f \n",data_arr[i]);
            serial.printf("%f\n", pressure_result);

            if (prev_data - pressure_result > 4){           // if pressure_result - prev_data > 0.04/10ms ( = 4mmHg/s) provide notice
              serial.printf("please slow down relieving your valve \n");
                }

            if (pressure_result<=30){
                  serial.printf("Done \n");
                  m_final_index = i;
                  serial.printf("m_final_index %i \n",i);
                  break;
                }
                
              }                   // reached 40 mmHg, Stop measuring
              // serial.printf("Done");
              // break;
            }
      
        if (pressure_result >=180){                     //reached 180mmHg
          flag  = 0;
          i=i-1;                                               // flag == 0: wait till pressure <150mmHg
          }   
        
      
        prev_data = pressure_result;                    //record previous pressure_result to get deflation rate 
      
        wait_ms(10);
        i++;                                    //take sample every 10 ms  
        

    }// while loop ends
    for(int j=0; j<i+1; j++){
      serial.printf("%3.2f ,",data_arr[j]);
    }
}







