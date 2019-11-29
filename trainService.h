class trainService
{
private:
    /* data */
public:
    std::string stationName;
    std::string originStation;
    std::string destination;
    std::string operator;
    std::string operatorCode;
    std::string* nrccMessage;
    std::string platform;
    std::string cancelReason;
    std::string delayReason;
    std::string std;
    std::string etd;
    std::string callingPoints;

    trainService(/* args */);
    ~trainService();
};