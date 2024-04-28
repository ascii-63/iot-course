#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>
#include <errno.h>

// Function to send a preset command to the Modbus device
void send_command(modbus_t *ctx, uint8_t *command, int command_length) {
    int rc;
    uint8_t response_buffer[MODBUS_TCP_MAX_ADU_LENGTH];

    // Write the command to the device
    rc = modbus_send_raw_request(ctx, command, command_length);
    if (rc == -1) {
        fprintf(stderr, "Command failed: %s\n", modbus_strerror(errno));
    } else {
        printf("Command sent successfully\n");

        // Read the response from the device (optional)
        int response_length = modbus_receive_confirmation(ctx, response_buffer);
        if (response_length > 0) {
            printf("Response: ");
            for (int i = 0; i < response_length; ++i) {
                printf("%02X ", response_buffer[i]);
            }
            printf("\n");
        } else if (response_length == -1) {
            fprintf(stderr, "Read feedback failed: %s\n", modbus_strerror(errno));
        } else {
            printf("No response received\n");
        }
    }
}

int main() {
    modbus_t *ctx;

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

    // Set response timeout (in seconds and microseconds)
    modbus_set_response_timeout(ctx, 5, 0); // 5 seconds

    // Connect to the Modbus device
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    // Set slave ID (device address)
    int slave_id = 2; // Change this according to your device
    modbus_set_slave(ctx, slave_id);

    // Example preset command (replace with your own command)
    // uint8_t preset_command[] = {0x02, 0x03, 0x00, 0x48, 0x00, 0x05, 0x05, 0xDF};
    uint8_t preset_command[]= {0x01 ,0x05, 0x00, 0x00 ,0xFF, 0x00 ,0x8C ,0x3A};
    int preset_command_length = sizeof(preset_command);

    // Call the function to send the preset command
    send_command(ctx, preset_command, preset_command_length);

    // Close connection and free the context
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
