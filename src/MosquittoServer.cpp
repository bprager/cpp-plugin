#include <mosquitto.h>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

class MosquittoServer
{
public:
    MosquittoServer() : mosq(nullptr)
    {
        mosquitto_lib_init();
        mosq = mosquitto_new("server", true, nullptr);

        static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
        plog::init(plog::debug, &consoleAppender);
    }

    ~MosquittoServer()
    {
        if (mosq)
        {
            mosquitto_destroy(mosq);
        }
        mosquitto_lib_cleanup();
    }

    bool checkBroker()
    {
        if (mosq)
        {
            int result = mosquitto_connect(mosq, "localhost", 1883, 60);
            if (result == MOSQ_ERR_SUCCESS)
            {
                LOG_INFO << "Successfully connected to the broker";
                return true;
            }
            else
            {
                LOG_ERROR << "Failed to connect to the broker: " << mosquitto_strerror(result);
            }
        }
        else
        {
            LOG_ERROR << "Mosquitto instance is not initialized";
        }
        return false;
    }

private:
    struct mosquitto *mosq;
};

int main()
{
    MosquittoServer server;
    server.checkBroker();

    return 0;
}
