#include "WifiTask.h"
#include "CaptiveDNS.h"
#include "CommonValueNames.h"

static void WifiStationTaskEntry(void *arg)
{
    printf("*******************EntryP point wifi station task %d\n", (int)arg);
    (((WifiTask*)arg))->StationConnectLoop();
     printf("*******************Exit point wifi station task\n");
	vTaskDelete(NULL);
}

static void WifiApTaskEntry(void *arg)
{
    printf("*******************Entry point wifi AP task %d\n", (int)arg);
    ((WifiTask *)arg)->ApTaskMain();
	vTaskDelete(NULL);
}

WifiConfiguredNetwork* WifiTask::GetConfiguredNetwork(const std::string& pSSID, const std::string& pAuthMode) {
    if(_testConfiguration!=nullptr) {
        if(_testConfiguration->ssid==pSSID && _testConfiguration->authMode==pAuthMode)
            return _testConfiguration;
        return nullptr;
    }
    for(auto configuredNetwork : _wifiSettings->GetNetworks()) 
        if(configuredNetwork.second->ssid == pSSID &&
           (pAuthMode == "" || configuredNetwork.second->authMode == pAuthMode))
            return configuredNetwork.second;
    return nullptr;
}

WifiSettings* WifiTask::_wifiSettings = nullptr;

bool WifiTask::CompareAvailableNetworks(WifiAvailableNetwork* pNetwork1, WifiAvailableNetwork* pNetwork2)
{
    auto network1 = _wifiSettings->GetNetworks().contains(pNetwork1->ssid) ? _wifiSettings->GetNetworks()[pNetwork1->ssid] : nullptr;
    auto network2 = _wifiSettings->GetNetworks().contains(pNetwork2->ssid) ? _wifiSettings->GetNetworks()[pNetwork2->ssid] : nullptr;
    if(network1 == nullptr && network2!=nullptr) return false;
    if(network1 != nullptr && network2==nullptr) return true;
    if(network1 != nullptr && network2!=nullptr && network1->ssid!=network2->ssid) return network1->priority < network2->priority;
    return (pNetwork1->rssi > pNetwork2->rssi);
}

void WifiTask::ApTaskMain() {
    printf("****************AP Task Main loop\n");
    CaptiveDns dns;
    ApDescription ap;
    ap.gatewayIP = _apIPv4Gateway;
    ap.netmask = _apIPv4Mask;
    ap.startIP = _apIPv4StartIP;
    ap.ssid = _apSSID;
    ap.password = _apPassword;
    _manager->EnableAP(ap);
    while(!HasConfiguredNetworks()) {      
       dns.ProcessRequest();
       esp_task_wdt_reset();
       vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    _manager->DisableAP();
    _valIsApActive.b = false;
}

WifiTask::WifiTask(const std::string& pDeviceName, const std::string& pApPassword) : _apSSID(pDeviceName), _apPassword(pApPassword) {
    _wifiSettings = new WifiSettings();
    _manager = new WifiManager(*this);
    _stationSemaphore = xSemaphoreCreateBinary();
    _searchSemaphore = xSemaphoreCreateBinary();
    _uiMutex = xSemaphoreCreateMutex();
    AddValueSource(new ValueSource(*this,WIFI_STA_CONNNECTED  ,Bool,   Dimensionless, _valIsStaConnected,   NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_STA_SSID        ,String, Dimensionless, _valStaSSID,          NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_STA_AUTH        ,String, Dimensionless, _valStaAuth,          NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_STA_RSSI        ,String, Dimensionless, _valStaRssi,          NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_STA_IPV4_ADDR   ,String, Dimensionless, _valStaIPv4Addr,      NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_STA_IPV4_MASK   ,String, Dimensionless, _valStaIPv4Mask,      NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_STA_IPV4_GATEWAY,String, Dimensionless, _valStaIPv4Gateway,   NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_CHANNEL         ,Int,    Dimensionless, _valChannel,          NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_AP_ACTIVE       ,Bool,   Dimensionless, _valIsApActive,       NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_AP_CONNECTED    ,Bool,   Dimensionless, _valIsApStaConnected, NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_AP_SSID         ,String, Dimensionless, _valApSSID,           NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_AP_AUTH         ,String, Dimensionless, _valApAuth,           NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_AP_PASSWORD     ,String, Dimensionless, _valApPassword,       VALUESOURCE_NOFLAGS));
    AddValueSource(new ValueSource(*this,WIFI_AP_IPV4_GATEWAY ,String, Dimensionless, _valApIPv4Gateway,    NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_AP_IPV4_MASK    ,String, Dimensionless, _valApIPv4Mask,       NETWORK_INFO));
    AddValueSource(new ValueSource(*this,WIFI_AP_IPV4_STARTIP ,String, Dimensionless, _valApIPv4StartIP,    NETWORK_INFO));
}

WifiTask::~WifiTask() {

}

void WifiTask::Init() {
    printf("****************Starting wifi station task\n");
    xTaskCreate(WifiStationTaskEntry, "wifista", 4096, this, 10, NULL);
}

void WifiTask::CreateAPTaskIfNotRunning() {
    if(_valIsApActive.b) return;
    _valIsApActive.b = true;
    xTaskCreate(WifiApTaskEntry, "wifiap", 4096, this, 10, NULL);
}

#define STARTING_SLEEP_INTERVAL 250
#define CONNECTING_SLEEP_INTERVAL 2500
#define CONNECTED_SLEEP_INTERVAL 7500

void WifiTask::StationConnectLoop() {
    printf("****************Connect loop\n");
    WifiAvailableNetworks* foundNetworks = new WifiAvailableNetworks();
    WifiAvailableNetworks* matchingNetworks = new WifiAvailableNetworks(false);
    auto connectIndex = 0;
    auto lastPhase = _phase;
    if(_wifiSettings->GetNetworks().size() == 0 ) {
            CreateAPTaskIfNotRunning();
    }

        
    while(true) {
        if(lastPhase!=_phase) {
            switch(_phase) {
                case WifiTaskConnectPhase::Starting :
                    ets_printf("Entered Starting Phase");
                    break;
                case WifiTaskConnectPhase::StartConnectSequence :
                    ets_printf("Entered StartConnectSequence Phase");
                    break;
                case WifiTaskConnectPhase::Searching :
                    ets_printf("Entered Searching Phase");
                    break;
                case WifiTaskConnectPhase::SearchResults :
                    ets_printf("Entered SearchResults Phase");
                    break;
                
                case WifiTaskConnectPhase::TryConnect :
                    ets_printf("Entered TryConnect Phase");
                    break;
                case WifiTaskConnectPhase::Connecting :
                    ets_printf("Entered Connecting Phase");
                    break;
                case WifiTaskConnectPhase::WaitingForIP :
                    ets_printf("Entered WaitingForIP Phase");
                    break;
                case WifiTaskConnectPhase::Failed :
                    ets_printf("Entered Failed Phase");
                    break;
                case WifiTaskConnectPhase::Connected :
                    ets_printf("Entered Connected Phase");
                    break;
                case WifiTaskConnectPhase::Disconnected :
                    ets_printf("Entered Disconnected Phase");
                    break;
                case WifiTaskConnectPhase::TestFailure :
                ets_printf("Entered TestFailure Phase");
                    break;
                case WifiTaskConnectPhase::TestSuccess :
                    ets_printf("Entered TestSuccess Phase");
                    break;
                }

        }   

        lastPhase = _phase; 
        switch(_phase) {
            case WifiTaskConnectPhase::Starting :
                xSemaphoreTake(_stationSemaphore, STARTING_SLEEP_INTERVAL / portTICK_PERIOD_MS);  
                break;
            case WifiTaskConnectPhase::StartConnectSequence :
                ets_printf("StationConnectLoop8\n");
                if(_testConfiguration == nullptr && _wifiSettings->GetNetworks().size() == 0 ) {
                    vTaskDelay(STARTING_SLEEP_INTERVAL / portTICK_PERIOD_MS);  
                    continue;
                }
                ets_printf("StationConnectLoop9\n");
                _gotIP = false;
                foundNetworks->clearAndFree();    
                matchingNetworks->clear();       
                _manager->Scan();
                SetPhase(WifiTaskConnectPhase::Searching);
                ets_printf("StationConnectLoop10\n");
                break;
            case WifiTaskConnectPhase::Searching :
                ets_printf("StationConnectLoop11\n");
                xSemaphoreTake(_stationSemaphore, CONNECTING_SLEEP_INTERVAL / portTICK_PERIOD_MS);  
                ets_printf("StationConnectLoop12\n");
                break;
            case WifiTaskConnectPhase::SearchResults :
                ets_printf("StationConnectLoop13\n");
                _manager->GetScanResults(*foundNetworks);
                std::sort(foundNetworks->begin(), foundNetworks->end(), CompareAvailableNetworks);
                
                
                for(auto network : *foundNetworks) {
                    auto configuredNetwork = GetConfiguredNetwork(network->ssid, network->authmode);
                    if(configuredNetwork!=nullptr)
                        matchingNetworks->push_back(network);
                }
                connectIndex = 0;
                SetPhase(WifiTaskConnectPhase::TryConnect);
                ets_printf("StationConnectLoop14\n");
                break;
            
            case WifiTaskConnectPhase::TryConnect :
                ets_printf("StationConnectLoop15\n");
                if(connectIndex<matchingNetworks->size()) {
                    auto networkToTry = (*matchingNetworks)[connectIndex];
                    auto configuredNetworkToTry = GetConfiguredNetwork(networkToTry->ssid, networkToTry->authmode);
                    _manager->ConnectStation(networkToTry, configuredNetworkToTry->password);
                    connectIndex++;    
                    SetPhase(WifiTaskConnectPhase::Connecting);
                } else 
                    SetPhase(WifiTaskConnectPhase::Disconnected);
                ets_printf("StationConnectLoop16\n");
                break;
            case WifiTaskConnectPhase::Connecting :
                ets_printf("StationConnectLoop17\n");
                xSemaphoreTake(_stationSemaphore, CONNECTING_SLEEP_INTERVAL / portTICK_PERIOD_MS); 
                ets_printf("StationConnectLoop18\n");
                break;
            case WifiTaskConnectPhase::WaitingForIP :
                ets_printf("StationConnectLoop19\n");
                xSemaphoreTake(_stationSemaphore, CONNECTING_SLEEP_INTERVAL / portTICK_PERIOD_MS); 
                ets_printf("StationConnectLoop20\n");
                break;
            case WifiTaskConnectPhase::Failed :
                ets_printf("StationConnectLoop21\n");
                xSemaphoreTake(_stationSemaphore, WifiTaskConnectPhase::TryConnect);
                ets_printf("StationConnectLoop22\n");
                break;
            case WifiTaskConnectPhase::Connected :
                ets_printf("StationConnectLoop23\n");
                xSemaphoreTake(_stationSemaphore, CONNECTED_SLEEP_INTERVAL / portTICK_PERIOD_MS);  
                ets_printf("StationConnectLoop24\n");
                break;
            case WifiTaskConnectPhase::Disconnected :
                {
                    ets_printf("StationConnectLoop25\n");
                    time_t elapsedSinceDisconnect = (time(nullptr) - _phaseStart);    
                    if(elapsedSinceDisconnect<_wifiSettings->GetWaitOnDisconnectTime()) {          
                        xSemaphoreTake(_stationSemaphore, ((_wifiSettings->GetWaitOnDisconnectTime()- elapsedSinceDisconnect )*1000) / portTICK_PERIOD_MS);  
                    } else {
                        SetPhase(WifiTaskConnectPhase::StartConnectSequence);
                    }
                    ets_printf("StationConnectLoop26\n");
                    break;
                }
            case WifiTaskConnectPhase::TestFailure :
                ets_printf("StationConnectLoop27\n");
                xSemaphoreTake(_stationSemaphore, CONNECTING_SLEEP_INTERVAL / portTICK_PERIOD_MS); 
                ets_printf("StationConnectLoop28\n");
                break;
            case WifiTaskConnectPhase::TestSuccess :
                ets_printf("StationConnectLoop29\n");
                xSemaphoreTake(_stationSemaphore, CONNECTING_SLEEP_INTERVAL / portTICK_PERIOD_MS); 
                ets_printf("StationConnectLoop30\n");
                break;
        }
    }
}

void WifiTask::OnWifiStarted() {
     SetPhase(WifiTaskConnectPhase::StartConnectSequence);
}

void WifiTask::OnStationConnected(std::string pSSID, uint8_t pChanel, uint8_t pBSSID[6], std::string pAuthMode) {
    SetPhase(WifiTaskConnectPhase::WaitingForIP);
    _staSSID = pSSID;
    _valChannel.i = pChanel;
    _staAuth = pAuthMode;
    _valIsStaConnected.b = true;
}

void WifiTask::OnStationDisconnected(WifiDisconnectReason pReason) {
    _valIsStaConnected.b = false;
    if(_testConfiguration!=nullptr) {
        SetPhase(WifiTaskConnectPhase::TestFailure);
    } else if(_gotIP) {
        SetPhase(WifiTaskConnectPhase::Disconnected);
    } else {
        SetPhase(WifiTaskConnectPhase::Failed);
    }
    time(&_phaseStart);
}

void WifiTask::OnStationGotIP(std::string pIp, std::string pNetmask, std::string pGateway) {
    _gotIP = true;
    _staIPv4Addr= pIp;
    _staIPv4Mask = pNetmask;
    _staIPv4Gateway = pGateway;
    if(_testConfiguration==nullptr)
        SetPhase(WifiTaskConnectPhase::Connected);
    else 
        SetPhase(WifiTaskConnectPhase::TestSuccess);
    
}

void WifiTask::OnStationLostIP() {
    switch(_phase) {
        case WifiTaskConnectPhase::Disconnected :
        case WifiTaskConnectPhase::Failed :
            break;
        default: 
            SetPhase(WifiTaskConnectPhase::WaitingForIP);            
            break;
    }
}

void WifiTask::OnScanComplete() {
    if(_phase == WifiTaskConnectPhase::Searching)
        SetPhase(WifiTaskConnectPhase::SearchResults);
    else
        xSemaphoreGive(_searchSemaphore);
}

void WifiTask::SetPhase(WifiTaskConnectPhase pPhase) {
    _phase = pPhase;
    time(&_phaseStart);
    xSemaphoreGive(_stationSemaphore);
}

bool WifiTask::HasConfiguredNetworks() {
    return _wifiSettings->GetNetworks().size()>0;
}

const std::string& WifiTask::GetValuesSourceName() const {
    return "Wifi";
}


void WifiTask::ScanForAvailableNetworks(WifiAvailableNetworks& pNetworks) {
    xSemaphoreTake(_uiMutex, portMAX_DELAY);
    _manager->Scan();
    xSemaphoreTake(_searchSemaphore, portMAX_DELAY); 
    _manager->GetScanResults(pNetworks);
    xSemaphoreGive(_uiMutex);
}

bool WifiTask::TestConfiguration(const std::string& pSSID, const std::string& pAuthMode, const std::string& pPassword) {
    ets_printf("Test 1\n");
    xSemaphoreTake(_uiMutex, portMAX_DELAY);
    
    ets_printf("Test 2\n");
    auto network = GetConfiguredNetwork(pSSID, pAuthMode);
    if(network != nullptr) {
        xSemaphoreGive(_uiMutex);
        ets_printf("Test 3\n");
        return false; 
    }

    ets_printf("Test 4\n");
    _testConfiguration = new WifiConfiguredNetwork();
    _testConfiguration->ssid = pSSID;
    _testConfiguration->authMode = pAuthMode;
    _testConfiguration->password = pPassword;

    if(_valIsStaConnected.b) {
        if(_manager->DisconnectStation()) {
            while(_valIsStaConnected.b) {
                esp_task_wdt_reset();
                taskYIELD();
            }
        }
    }
    ets_printf("Test 5\n");

    SetPhase(WifiTaskConnectPhase::StartConnectSequence);
    ets_printf("test 5 - Waiting\n");
    while(_phase!=WifiTaskConnectPhase::TestFailure && _phase!=WifiTaskConnectPhase::TestSuccess) {
        esp_task_wdt_reset();
        taskYIELD();
    }
    ets_printf("Test 6\n");
    auto result = _phase == WifiTaskConnectPhase::TestSuccess;
    delete _testConfiguration;
    _testConfiguration = nullptr;

    if(_valIsStaConnected.b) {
        if(_manager->DisconnectStation()) {
            while(_valIsStaConnected.b) {
                esp_task_wdt_reset();
                taskYIELD();
            }
        }
    }

    ets_printf("Test 7\n");
    SetPhase(WifiTaskConnectPhase::StartConnectSequence);

    xSemaphoreGive(_uiMutex);
    ets_printf("Test 8 result is %s\n", result? "true" :  "false");
    return result;
}

bool WifiTask::AddConfiguration(const std::string& pSSID, const std::string& pAuthMode, const std::string& pPassword, bool pMakePriority) {
    xSemaphoreTake(_uiMutex, portMAX_DELAY);
     ets_printf("Add Wifi 1\n");
    const std::string authMode = "";
    auto network = GetConfiguredNetwork(pSSID, authMode);
    if(network != nullptr) {
        xSemaphoreGive(_uiMutex);
        return false; 
    }
    ets_printf("Add Wifi 2\n");
    auto newNetwork = new WifiConfiguredNetwork();
    newNetwork->ssid = pSSID;
    newNetwork->authMode = pAuthMode;
    newNetwork->password = pPassword;
    ets_printf("Add Wifi 3\n");
    if(pMakePriority) {
        newNetwork->priority = 0; 
        for(auto existingNetwork : _wifiSettings->GetNetworks())
            existingNetwork.second->priority++;
    } else {
        newNetwork->priority = _wifiSettings->GetNetworks().size();
    }
    _wifiSettings->GetNetworks()[pSSID] = newNetwork;
    _wifiSettings->Save();
    ets_printf("Add Wifi 4\n");
    if(pMakePriority) {
        if(_valIsStaConnected.b) {
            if(_manager->DisconnectStation()) {
                while(_valIsStaConnected.b) {
                    esp_task_wdt_reset();
                    taskYIELD();
                }
            }
        }
        SetPhase(WifiTaskConnectPhase::StartConnectSequence);
    }
    ets_printf("Add Wifi 6\n");
    xSemaphoreGive(_uiMutex);
    return true;
}

bool WifiTask::RemoveConfiguration(std::string pSSID) {
    xSemaphoreTake(_uiMutex, portMAX_DELAY);
    if(_staSSID == pSSID) {
        xSemaphoreGive(_uiMutex);
        return false;
    }
    const std::string authMode = "";
    auto network = GetConfiguredNetwork(pSSID, authMode);
    if(network == nullptr) {
        xSemaphoreGive(_uiMutex);
        return false; 
    }

    _wifiSettings->GetNetworks().erase(pSSID);
    _wifiSettings->Save();    
    xSemaphoreGive(_uiMutex);
    return true;
}

    
WifiConnectionInfo* WifiTask::GetConnectionInfo() {
    auto result = new WifiConnectionInfo();
    if(_valIsStaConnected.b) {    
        result->ssid = _staSSID;
        result->authMode = _staAuth;
        result->channel = _valChannel.i;
        result->ipv4Address = _staIPv4Addr;
        result->ipv4Gateway = _staIPv4Gateway;
        result->ipv4Netmask = _staIPv4Mask;
    } else {
        result->ssid = "";
        result->authMode = "";
         result->channel = 0;
        result->ipv4Address = "";
        result->ipv4Gateway = "";
        result->ipv4Netmask = "";
    }
    return result;
}
