#include <mosquitto.h>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <thread>
#include <chrono>

class MqttServer
{
public:
    MqttServer(const char *id, const char *host, int port)
        : id(id), host(host), port(port)
    {
        // Initialize logging
        static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
        plog::init(plog::info, &consoleAppender);

        // Initialize mosquitto library
        mosquitto_lib_init();
        mosq = mosquitto_new(id, true, nullptr);
        if (!mosq)
        {
            LOG_ERROR << "Can't initialize Mosquitto library";
            exit(1);
        }

        // Set up callbacks
        mosquitto_message_callback_set(mosq, onMessage);

        // Connect to the broker
        int ret = mosquitto_connect(mosq, host, port, keepalive);
        if (ret)
        {
            LOG_ERROR << "Can't connect to Mosquitto server";
            exit(1);
        }
    }

    ~MqttServer()
    {
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    }

    void subscribe(const char *topic, int qos = 0)
    {
        mosquitto_subscribe(mosq, nullptr, topic, qos);
    }

    void loop()
    {
        mosquitto_loop_start(mosq);
    }

private:
    const char *id;
    const char *host;
    int port;
    int keepalive = 60;
    struct mosquitto *mosq;

    static void onMessage(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
    {
        if (message)
        {
            LOG_INFO << "Received message: " << static_cast<char *>(message->payload);
        }
    }
};

int main()
{
    MqttServer server("mqtt_subscriber", "localhost", 1883);
    server.subscribe("mqtt/test");
    server.loop();

    // We will run for a short period of time, to demonstrate
    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
