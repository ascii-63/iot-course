#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>
#include <errno.h>

int main() {
    modbus_t *ctx;
    int rc;
    uint8_t response_buffer[MODBUS_TCP_MAX_ADU_LENGTH];

    // Modbus device parameters
    const char *dev = "/dev/ttyUSB0"; // Modify this according to your device
    int baud = 9600;
    char parity = 'N';
    int data_bit = 8;
    int stop_bit = 1;

    // Create a new RTU context
    ctx = modbus_new_rtu(dev, baud, parity, data_bit, stop_bit);
    if (ctx == NULL) {
        fprintf(stderr, "Unable to create the libmodbus context\n");
        return -1;
    }
    modbus_set_response_timeout(ctx, 3,0);
    // Connect to the Modbus device
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    // Set slave ID (device address)
    int slave_id = 2; // Change this according to your device
    modbus_set_slave(ctx, slave_id);
    // Modbus write message data
    uint8_t write_data[] = {0x02, 0x03, 0x00 ,0x48 ,0x00, 0x05, 0x05 ,0xDF};
    //uint8_t write_data[] = {0x01, 0x05, 0x00,0x00, 0xFF, 0x00, 0x8C, 0x3A};
    int write_num_bytes = sizeof(write_data);

    // Write the data to the device using function code 0x0F (Write Multiple Coils)
    rc = modbus_send_raw_request(ctx, write_data, write_num_bytes);
    if (rc == -1) {
        fprintf(stderr, "Write failed: %s\n", modbus_strerror(errno));
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    } else {
        printf("Write successful\n");
    }

    // Read the response from the device (optional)
    int response_length = modbus_receive_confirmation(ctx, response_buffer);
    if (response_length == -1) {
        fprintf(stderr, "Read feedback failed: %s\n", modbus_strerror(errno));
    } else {
        printf("Read feedback successful. Feedback: ");
        for (int i = 0; i < response_length; ++i) {
            printf("%02X ", response_buffer[i]);
            // printf("%d",response_buffer[i]);
        }
        printf("\n");
    }

    // Close connection and free the context
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
