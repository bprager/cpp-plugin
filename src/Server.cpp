#include <mosquitto.h>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <thread>
#include <chrono>
#include <cstring> // for strcmp
#include <condition_variable>

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
        mosq = mosquitto_new(id, true, this); // Passing this as userdata
        if (!mosq)
        {
            LOG_ERROR << "Can't initialize Mosquitto library";
            exit(1);
        }

        // Set up callbacks
        mosquitto_message_callback_set(mosq, MqttServer::onMessageWrapper);

        // Connect to the broker
        int ret = mosquitto_connect(mosq, host, port, keepalive);
        if (ret)
        {
            LOG_ERROR << "Can't connect to Mosquitto server";
            exit(1);
        }

        isRunning = true;

        LOG_INFO << "Connected to Mosquitto server. Listening to topic: mqtt/test ... " << std::endl
                 << "Send 'exit' to stop the server.";
    }

    ~MqttServer()
    {
        if (isRunning)
        {
            stop();
        }

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

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            isRunning = false;
        }
        cv.notify_one();
        mosquitto_disconnect(mosq);
        mosquitto_loop_stop(mosq, true); // non-blocking call
    }

    bool is_running() { return isRunning; }

    void wait_until_stopped()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]
                { return !this->isRunning; });
    }

private:
    const char *id;
    const char *host;
    bool isRunning; // Used to stop the loop
    int port;
    int keepalive = 60;
    struct mosquitto *mosq;

    std::mutex mtx;
    std::condition_variable cv;

    static void onMessageWrapper(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
    {
        static_cast<MqttServer *>(obj)->onMessage(mosq, message);
    }

    void onMessage(struct mosquitto *mosq, const struct mosquitto_message *message)
    {
        if (message)
        {
            LOG_INFO << "Received message: " << static_cast<char *>(message->payload);
            if (strcmp(static_cast<char *>(message->payload), "exit") == 0)
            {
                LOG_INFO << "Exiting ...";
                stop();
            }
        }
    }
};

int main()
{
    MqttServer server("mqtt_subscriber", "localhost", 1883);
    server.subscribe("mqtt/test");
    server.loop();

    server.wait_until_stopped();

    return 0;
}
