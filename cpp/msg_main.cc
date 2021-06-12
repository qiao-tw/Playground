#include "msgbus.h"

struct WeatherInfo
{
    int temperature;
    int humidity;
};

struct StockInfo
{
    uint16_t stock_id;
    double price;
};

constexpr MsgId WEATHER_INFO = 0x000000001;
constexpr MsgId STOCK_INFO = 0x000000002;

struct WeatherInfoMsg : public MsgBase
{
    WeatherInfo weather_info;
    WeatherInfoMsg(): MsgBase(WEATHER_INFO, "WEATHER_INFO") {}
    WeatherInfoMsg(WeatherInfo wi): MsgBase(WEATHER_INFO, "WEATHER_INFO"), weather_info(wi) {}
};
using WeatherInfoMsgPtr = shared_ptr<WeatherInfoMsg>;

DEFINE_MSG(StockInfo, STOCK_INFO, stock_info)

class WeatherMan : public MsgBus::Subscriber
{
public:
    WeatherMan(): MsgBus::Subscriber("weatherman") {}
    bool ProcMsg(MsgPtr msg) {
        if (msg->msg_id == WEATHER_INFO) {
            // WeatherInfoMsgPtr wmp = dynamic_pointer_cast<WeatherInfoMsgPtr>(msg);
            shared_ptr<WeatherInfoMsg> wmp = dynamic_pointer_cast<WeatherInfoMsg>(msg);
            cout << "temperature: " << wmp->weather_info.temperature << endl;
            cout << "humidity: " << wmp->weather_info.humidity << endl;
        } else {
            cout << "WeatherMan ignore msg: " << msg->msg_name; 
        }
        return true;
    }
};

class StockTrader : public MsgBus::Subscriber
{
public:
    StockTrader(): MsgBus::Subscriber("stock_trader") {}
    void Activate() {
        Subscribe(STOCK_INFO);
        Subscriber::Activate();
    }

    bool ProcMsg(MsgPtr msg) {
        if (msg->msg_id == STOCK_INFO) {
            // WeatherInfoMsgPtr wmp = dynamic_pointer_cast<WeatherInfoMsgPtr>(msg);
            StockInfoMsgPtr sp = dynamic_pointer_cast<StockInfoMsg>(msg);
            cout << "stock id: " << sp->stock_info.stock_id << endl;
            cout << "stock price: " << sp->stock_info.price << endl;
        } else {
            cout << "StockTrader ignore msg: " << msg->msg_name; 
        }
        return true;
    }        
    
};

class WeatherInfoBroadcaster : public Looper, public MsgBus::Publisher 
{
public:
    WeatherInfoBroadcaster() : Looper("weather_broadcase", 500) {}

    void SpinOnce() override {
        static int temperature = 20;
        static int humidity = 1;
        WeatherInfoMsgPtr wp = make_shared<WeatherInfoMsg>();
        wp->weather_info.temperature = temperature++;
        wp->weather_info.humidity = humidity++;
        Publish(wp);
    }

    bool WaitCondition() override {
        return true;
    }
};

int main(int argc, char* argv[])
{
    WeatherInfo wi;
    wi.temperature = 23;
    wi.humidity = 11;

    StockInfo si;
    si.stock_id = 123;
    si.price = 12345.6;

    WeatherInfoMsgPtr wp = make_shared<WeatherInfoMsg>(wi);
    StockInfoMsgPtr si_ptr = make_shared<StockInfoMsg>(si);

    shared_ptr<WeatherMan> wm = make_shared<WeatherMan>();
    wm->Subscribe(WEATHER_INFO);
    wm->Activate();
    // MsgBus::Subscribe(WEATHER_INFO, wm);

    shared_ptr<StockTrader> stock_trader = make_shared<StockTrader>();
    stock_trader->Activate();

    // this_thread::sleep_for(std::chrono::milliseconds(2000));

    MsgBus::Publish(wp);
    MsgBus::Publish(si_ptr);

    WeatherInfoBroadcaster wb;
    wb.Activate();

    int i;
    cin >> i;

    return 0;
}