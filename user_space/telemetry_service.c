#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "MQTTClient.h" // Include the MQTT client library header

// MQTT Configuration
#define ADDRESS "ssl://b6f1609fde1a4a99bf6693546aa16a50.s1.eu.hivemq.cloud:8883"
#define CLIENTID "HybridIoTClient_RPI4B"
#define TOPIC "iot/telemetry"

#define USERNAME "vishu1997"
#define PASSWORD "7070070@Vkt#"

#define QOS 1
#define TIMEOUT 10000L

#define DEVICE_PATH "/dev/iot_uart"
#define BUFFER_SIZE 256

// Flag to control the main loop for graceful shutdown
volatile int keep_running = 1;

// Signal handler for graceful shutdown
void signal_handler(int sig) { keep_running = 0; }

// Function to handle MQTT reconnection logic
int reconnect_mqtt(MQTTClient client, MQTTClient_connectOptions *conn_opts) {
  int ret;

  printf("Attempting MQTT reconnect...\n");

  while ((ret = MQTTClient_connect(client, conn_opts)) != MQTTCLIENT_SUCCESS) {
    printf("Reconnect failed: %d\n", ret);

    sleep(2);
  }

  printf("MQTT Reconnected Successfully\n");

  return ret;
}

// Main function for the telemetry service
int main()
{
  int fd;
  char buffer[BUFFER_SIZE];
  ssize_t bytes_read;

  // MQTT Client setup
  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
  int ret_status;

  printf("Telemetry Service Starting...\n");

  // Set up signal handlers for graceful shutdown
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  // Initialize MQTT Client
  MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,
                    NULL);

  // Set MQTT connection options
  conn_opts.username = USERNAME;
  conn_opts.password = PASSWORD;
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;

  // Set SSL options
  ssl_opts.enableServerCertAuth = 0;
  conn_opts.ssl = &ssl_opts;

  // Initial MQTT broker connection
  ret_status = MQTTClient_connect(client, &conn_opts);

  if (ret_status != MQTTCLIENT_SUCCESS) {
    printf("Initial MQTT connection failed: %d\n", ret_status);

    reconnect_mqtt(client, &conn_opts);
  }

  printf("Connected to HiveMQ Cloud\n");

  fd = open(DEVICE_PATH, O_RDONLY);

  if (fd < 0) {
    perror("Failed to open /dev/iot_uart");
    return -1;
  }

    printf("Connected to %s\n", DEVICE_PATH);

    while (keep_running) {
      memset(buffer, 0, sizeof(buffer));

      bytes_read = read(fd, buffer, sizeof(buffer) - 1);

      if (bytes_read > 0) {
        buffer[bytes_read] = '\0';

        printf("Telemetry Frame: %s", buffer);

        if (!MQTTClient_isConnected(client)) {
          printf("MQTT disconnected\n");

          reconnect_mqtt(client, &conn_opts);
        }

        // Publish to MQTT
        MQTTClient_message pubmsg = MQTTClient_message_initializer;

        // MQTTClient_deliveryToken is used to track the delivery of the message
        MQTTClient_deliveryToken token;

        // Set the MQTT message properties
        pubmsg.payload = buffer;
        pubmsg.payloadlen = strlen(buffer);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;

        // Publish the message to the specified topic
        MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);

        // Wait for the message to be delivered
        MQTTClient_waitForCompletion(client, token, TIMEOUT);

        printf("Published telemetry to HiveMQ Cloud\n");

        fflush(stdout);
      } else if (bytes_read < 0) {
        if (!keep_running) {
          printf("Telemetry service shutting down\n");
        } else {
          perror("Read error");
        }

        break;
      }

      usleep(100000);
    }

    close(fd);

    // Disconnect from MQTT broker and clean up
    MQTTClient_disconnect(client, 1000);
    // Destroy the MQTT client instance
    MQTTClient_destroy(&client);

    return 0;
}
