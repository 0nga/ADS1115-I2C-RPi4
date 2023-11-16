#include <bcm2835.h>
#include <stdio.h>
#include <stdint.h>


int main(int argc, char **argv) {
	
	uint8_t adsAddress = 0x48;// I2C address of ADS1115
	
	for(;;){
		
		//Initialise the library, return 1 if successful, 0 otherwise
		if (!bcm2835_init()) {
			printf("bcm2835_init failed. Are you running as root?\n");
			return 1;
		}

		bcm2835_i2c_begin(); //Enables the pin (P1-03 (SDA) and P1-05(SCL)) for I2C
		bcm2835_i2c_setSlaveAddress(adsAddress); // Set the I2C address for communication
		bcm2835_i2c_set_baudrate(100000); // Set the I2C clock speed to 100kHz

		char config[3]; // 3 Configuration Bytes
		char read_data[2]; // 2 Bytes the ADS1115 send me (16bit)
	
		// ADS1115 Read Configuration (page 18 of datasheet)
		
		config[0] = 0b00000001; // config register address (from datasheet)
		
		// Read from A0, FS = +-6.144V, pwr-down single-shot mode
		config[1] = 0b11000001; // MSB from 15 to 8
		
		// 128 SPS, Comparator disabled
		config[2] = 0b10000011; // LSB from 7 to 0
		
		read_data[0] = 0;
		read_data[1] = 0;
		
		// Write the configuration Bytes in the config register
		bcm2835_i2c_write(&config[0], 3);
    
		// Wait for the conversion to complete, this requires bit 15 to change from 0 -> 1
		while((read_data[0] & 0x80) == 0){
			bcm2835_i2c_read(&read_data[0], 2);
		}
		
		char conversion[1];
		
		conversion[0] = 0b00000000; //conversion register
		
		/* In the while before i read the Bytes from config register,
		   now i set the conversion register*/
		bcm2835_i2c_write(&conversion[0], 1);
		
		// Read the Bytes from conversion register to get results*/
		bcm2835_i2c_read(&read_data[0], 2);
		
		// Combine the bytes to get the 16-bit ADC value
		int16_t value = (read_data[0] <<8) | read_data[1];
		
		// Convert to voltage (assuming ±6.144 V range)
		float voltage = (float)value * 6.144 / 32767.0;
	
		printf("Raw ADC Value: %d\n", value);
		printf("Voltage: %f V\n", voltage);

		bcm2835_i2c_end();	//close i2c
		bcm2835_close();	//close BCM2835
		delay(1000);		//wait 1 sec
	}
    return 0;
}
