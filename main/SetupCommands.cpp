#include "SetupCommands.h"
#include "GeneralSettings.h"
#include "Json.h"
#include <string>
#include "System.h"

SaveSettingsCommand::SaveSettingsCommand(GeneralSettings& pSettings) : _settings(pSettings) {
    
}
void SaveSettingsCommand::Process(Json& pJson,Json& pResult)  {
    pResult.CreateStringProperty("Status","true");
    
    if(pJson.HasProperty("DeviceName")) 
        _settings.SetDeviceName(pJson.GetStringProperty("DeviceName"));
    

    if(pJson.HasProperty("EnableDhcpNtp")) 
        _settings.SetEnableDhcpNtp(pJson.GetBoolProperty("EnableDhcpNtp"));

    if(pJson.HasProperty("EnablePowerSave")) 
        _settings.SetEnablePowerSave(pJson.GetBoolProperty("EnablePowerSave"));
    
    if(pJson.HasProperty("PrimaryNtpServer")) 
        _settings.SetPrimaryNtpServer(pJson.GetStringProperty("PrimaryNtpServer"));
    
    if(pJson.HasProperty("SecondaryNtpServer")) 
        _settings.SetSecondaryNtpServer(pJson.GetStringProperty("SecondaryNtpServer"));
           
    
    if(pJson.HasProperty("Co2SensorType"))
    {
        auto sensor = pJson.GetIntProperty("Co2SensorType");
        if(sensor < (int)CO2SensorType::MaxType && sensor >= 0) {
            _settings.SetCO2SensorType((CO2SensorType)sensor);
        }
    }


    _settings.Save();
}
std::string SaveSettingsCommand::GetName()  {
    return "SAVESETTINGS";
}



LoadSettingsCommand::LoadSettingsCommand(GeneralSettings& pSettings) : _settings(pSettings) {
    
}
void LoadSettingsCommand::Process(Json& pJson,Json& pResult)  {
    pResult.CreateStringProperty("Status","true");
    pResult.CreateStringProperty("DeviceName",_settings.GetDeviceName());   
    pResult.CreateBoolProperty("EnableDhcpNtp",_settings.GetEnableDhcpNtp());
    pResult.CreateBoolProperty("EnablePowerSave",_settings.GetEnablePowerSave());
    pResult.CreateStringProperty("PrimaryNtpServer",_settings.GetPrimaryNtpServer());
    pResult.CreateStringProperty("SecondaryNtpServer",_settings.GetSecondaryNtpServer());
    pResult.CreateNumberProperty("Co2SensorType", (int)_settings.GetCO2SensorType());


}
std::string LoadSettingsCommand::GetName() {
    return "LOADSETTINGS";
}


GetSystemInfoCommand::GetSystemInfoCommand(GeneralSettings& pSettings) : _settings(pSettings) {
    
}
void GetSystemInfoCommand::Process(Json& pJson,Json& pResult)  {
    pResult.CreateStringProperty("Status","true");
    pResult.CreateStringProperty("MCUName",System::GetMCUName());
    pResult.CreateNumberProperty("CoreCount",(int)System::GetCoreCount());
    pResult.CreateNumberProperty("FreeHeap",(int)System::GetFreeHeap());
    pResult.CreateNumberProperty("LeastFreeHeap",(int)System::GetLeastHeapFreeSinceBoot());
}

std::string GetSystemInfoCommand::GetName() {
    return "SYSTEMINFO";
}


GetAvailableWifiNetworksCommand::GetAvailableWifiNetworksCommand(GeneralSettings& pSettings, WifiTask& pWifi) : _settings(pSettings), _wifi(pWifi) {

}
void GetAvailableWifiNetworksCommand::Process(Json& pJson,Json& pResult) {
    pResult.CreateStringProperty("Status","true");
    std::vector<Json*> networks;
    WifiAvailableNetworks availableNetworks;
    _wifi.ScanForAvailableNetworks(availableNetworks);
    auto configured = _wifi.GetNetworks();
    for(auto availableNetwork : availableNetworks)
    {
        if(configured.contains(availableNetwork->ssid)) continue;
        auto network = new Json();
        network->CreateStringProperty("ssid", availableNetwork->ssid.c_str());
        char apMacAddr[20];
        snprintf(apMacAddr, sizeof(apMacAddr), "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                availableNetwork->bssid[0],
                availableNetwork->bssid[1],
                availableNetwork->bssid[2],
                availableNetwork->bssid[3],
                availableNetwork->bssid[4],
                availableNetwork->bssid[5]);
        network->CreateStringProperty("apMacAddr",      apMacAddr);
        network->CreateNumberProperty("channel",       availableNetwork->channel);
        network->CreateNumberProperty("signalStrength", availableNetwork->rssi);
        network->CreateStringProperty("authMode",       availableNetwork->authmode);        
        networks.push_back(network);
    }
    pResult.CreateArrayProperty("Networks", networks);
}

std::string GetAvailableWifiNetworksCommand::GetName() {
    return "GETNETWORKS";
}

GetCurrentWifiNetworkCommand::GetCurrentWifiNetworkCommand(GeneralSettings& pSettings) : _settings(pSettings) {

}

void GetCurrentWifiNetworkCommand::Process(Json& pJson,Json& pResult) {
    pResult.CreateStringProperty("Status","true");
    for(auto groupPair : ValueController::GetCurrent().GetGroups()) {
        Json* groupJson = nullptr;
        for(auto keyPair : groupPair.second->SourcesByName) {
            auto source = keyPair.second->DefaultSource;
            if(source->GetFlags() & NETWORK_INFO ) {
                if(groupJson == nullptr) 
                    groupJson = pResult.CreateObjectProperty(groupPair.first);
                source->SerialiseToJsonProperty(*groupJson);
            }            
        }   
        if(groupJson!=nullptr)
            delete groupJson; 
    }   
}

std::string GetCurrentWifiNetworkCommand::GetName() {
    return "GETNETWORKINFO";
}


SelectWifiNetworkCommand::SelectWifiNetworkCommand(GeneralSettings& pSettings, WifiTask& pWifi) : _settings(pSettings), _wifi(pWifi) {
    
}

void SelectWifiNetworkCommand::TestConnectToNetwork() {
    _testingConnect = true;  
    _testSucceeded= _wifi.TestConfiguration(_ssid, _auth, _password);
    _testingConnect = false;
}

void SelectWifiNetworkCommand::SetConnectToNetwork() {
    printf("Applying network\n");
    _applyingConnect = true;    
     _wifi.AddConfiguration(_ssid, _auth, _password, _makeDefault);   
    _applyingConnect = false;
}

void SelectWifiNetworkCommand::Process(Json& pJson,Json& pResult) {
    if(pJson.HasProperty("Mode")) {
        auto mode = pJson.GetStringProperty("Mode");
        if(mode=="Test" || mode == "Apply") {   
            if(_testingConnect || _applyingConnect) {
                return;   
            }
            if(!pJson.HasProperty("Ssid") || !pJson.HasProperty("Password")  || !pJson.HasProperty("Auth") || !pJson.HasProperty("Id") || 
               (mode=="Apply" && !pJson.HasProperty("MakeDefault") ) ) {
                pResult.CreateBoolProperty("Status", false);
                return;
            }      
            auto lastId = pJson.GetIntProperty("Id");
            if(lastId == _lastId) return;
            _lastId = lastId;

            _ssid = pJson.GetStringProperty("Ssid");
            _password = pJson.GetStringProperty("Password");
            _auth = pJson.GetStringProperty("Auth");

            if(mode=="Test") {
                TestConnectToNetwork();
                pResult.CreateStringProperty("ConnectStatus",  "Testing");
            } else {
                _makeDefault = pJson.GetBoolProperty("MakeDefault");
                SetConnectToNetwork();
                pResult.CreateStringProperty("ConnectStatus",  "Applying");
                        
            }
        } else if (mode == "List") {
            std::vector<Json*> networks;
            auto connectionInfo = _wifi.GetConnectionInfo();
            for(auto configuredNetwork : _wifi.GetNetworks()) {
                auto network = new Json();
                network->CreateStringProperty("ssid", configuredNetwork.second->ssid);
                network->CreateStringProperty("authMode", configuredNetwork.second->authMode);
                network->CreateNumberProperty("priority", (int)configuredNetwork.second->priority); 
                if(configuredNetwork.second->ssid == connectionInfo->ssid) {
                    auto connectionJson = network->CreateObjectProperty("connection");
                    connectionJson->CreateNumberProperty("channel", (int)connectionInfo->channel); 
                    connectionJson->CreateStringProperty("ipv4Address", connectionInfo->ipv4Address);
                    connectionJson->CreateStringProperty("ipv4Gateway", connectionInfo->ipv4Gateway);
                    connectionJson->CreateStringProperty("ipv4Netmask", connectionInfo->ipv4Netmask);
                    std::string dnsServers = "";
                    std::string ntpServers = "";
                    for(auto server : connectionInfo->dnsServers) {
                        if(dnsServers.size()!=0)
                            dnsServers+=", ";
                        dnsServers+=server;
                    }
                    for(auto server : connectionInfo->ntpServers) {
                        if(ntpServers.size()!=0)
                            ntpServers+=", ";
                        ntpServers+=server;
                    }
                    connectionJson->CreateStringProperty("dnsServers", dnsServers);
                    connectionJson->CreateStringProperty("ntpServers", ntpServers);

                    
                            
                    delete connectionJson;
                }              
                networks.push_back(network);
            }
            delete connectionInfo;
            pResult.CreateArrayProperty("Networks", networks);                        
        } else if (mode == "Remove") {
            if(!pJson.HasProperty("Ssid"))
            {
                pResult.CreateBoolProperty("Status", false);
                return;
            }
            auto ssid = pJson.GetStringProperty("Ssid");
            auto connectionInfo = _wifi.GetConnectionInfo();
            if(ssid == connectionInfo->ssid ||! _wifi.RemoveConfiguration(ssid)) {
                delete connectionInfo;
                pResult.CreateBoolProperty("Status", false);
                return;
            }
            delete connectionInfo;
        } else if (mode == "Priority") {
            if(!pJson.HasProperty("Ssid") || !pJson.HasProperty("Priority"))
            {
                pResult.CreateBoolProperty("Status", false);
                return;
            }
            auto ssid = pJson.GetStringProperty("Ssid");
            auto priority = pJson.GetUIntProperty("Priority");
            auto result = _wifi.SetConfigurationPriority(ssid, priority);
            pResult.CreateBoolProperty("Status", result);            
        } else {
            pResult.CreateBoolProperty("Testing", _testingConnect);
            pResult.CreateBoolProperty("TestSuccess", _testSucceeded);
            pResult.CreateBoolProperty("Applying", _applyingConnect);
        }                

        pResult.CreateBoolProperty("Status", true);
    } else {
        pResult.CreateBoolProperty("Status", false);
    }
}

std::string SelectWifiNetworkCommand::GetName() {
    return "SELECTNETWORK";   
}

DataManagementCommand::DataManagementCommand(GeneralSettings& pSettings, DataManagerStore& pManager) : _settings(pSettings), _manager(pManager) {

}

void DataManagementCommand::Process(Json& pJson,Json& pResult) {
    if(pJson.HasProperty("Mode")) {
        auto mode = pJson.GetStringProperty("Mode");
        if(mode=="GetValues") {   
            std::vector<Json*> availableValues;
            std::vector<Json*> setValues;
            for(auto groupPair : ValueController::GetCurrent().GetGroups()) {                
                for(auto keyPair : groupPair.second->SourcesByName) {
                    auto source = keyPair.second->DefaultSource;
                    if(source->IsIncludedInDataLog() ) {
                        Json* setJson = new Json();
                        setJson->CreateStringProperty("Group", groupPair.first);    
                        setJson->CreateStringProperty("Name", keyPair.first);
                        setValues.push_back(setJson);
                    }
                    if(source->GetFlags()!=VALUESOURCE_NOFLAGS && source->GetDataType()!=ValueDataType::String && source->GetDataType()!=ValueDataType::Bool ) {
                        Json* availJson = new Json();
                        availJson->CreateStringProperty("Group", groupPair.first);    
                        availJson->CreateStringProperty("Name", keyPair.first);
                        availableValues.push_back(availJson);
                    }               
                }   
            } 
            pResult.CreateNumberProperty("SensorUpdateInterval", (int)_settings.GetSensorUpdateInterval());
            pResult.CreateArrayProperty("Available", availableValues);
            pResult.CreateArrayProperty("Values", setValues);
        } else if (mode == "SetValues") {
            if(pJson.HasArrayProperty("Values") ) {
                for( const auto &group : ValueController::GetCurrent().GetGroups()) {
                    for(const auto &source : group.second->SourcesByName) {
                        auto valueByName = source.second;
                        for(const auto valueSource : valueByName->Sources)
                        {
                            valueSource->SetIsIncludedInDatalog(false);                            
                        }                
                    }
                }
                auto valuesProp = pJson.GetObjectProperty("Values");
                std::vector<Json*> valueElements;
                valuesProp->GetAsArrayElements(valueElements);
                delete valuesProp;
                for(auto setElement : valueElements) {
                    if(setElement->HasProperty("Group") && setElement->HasProperty("Name")) {
                        auto group = setElement->GetStringProperty("Group");
                        auto name = setElement->GetStringProperty("Name");
                        
                        if(!ValueController::GetCurrent().GetGroups().contains(group)) {
                            continue;
                        }
                        auto  valueGroup = ValueController::GetCurrent().GetGroups()[group];
                        if(!valueGroup->SourcesByName.contains(name)) continue;
                        auto valueContainer = valueGroup->SourcesByName[name];
                        valueContainer->DefaultSource->SetIsIncludedInDatalog(true);
                    }
                }               
                ValueController::GetCurrent().Save();
            }    
            
            if(pJson.HasProperty("SensorUpdateInterval"))
            {
                auto interval = pJson.GetIntProperty("SensorUpdateInterval");
                if(interval < (int)3600 && interval > 0) {
                    _settings.SetSensorUpdateInterval(interval);
                }
            }      

            _settings.Save(); 
        } else if(mode=="Clear")  {
             _manager.EraseAll();
        } else if (mode=="GetFlashInfo") {
            std::vector<Json*> buckets;
            DataManagerStoreCurrentBucketInfo currentBucketInfo;
            _manager.GetCurrentBucketInfo(currentBucketInfo);
            for(auto i = 0; i < _manager.GetNumBuckets();i++) {
                auto bucketJson = new Json();
                if(i == currentBucketInfo.currentIndex) {
                    bucketJson->CreateNumberProperty("Offset", (int)currentBucketInfo.info.Offset);
                    bucketJson->CreateNumberProperty("Index", (int)currentBucketInfo.info.Index);
                    if(currentBucketInfo.info.BlockStartTime!=0xFFFFFFFFFFFFFFFF)
                        bucketJson->CreateNumberProperty("BlockStartTime", (double)currentBucketInfo.info.BlockStartTime);
                    if(currentBucketInfo.info.BlockEndTime!=0xFFFFFFFFFFFFFFFF)
                        bucketJson->CreateNumberProperty("BlockEndTime", (double)currentBucketInfo.info.BlockEndTime);
                    bucketJson->CreateNumberProperty("DataLength", (double)currentBucketInfo.info.DataLength);
                    bucketJson->CreateNumberProperty("PayloadOffset", (int)currentBucketInfo.offset);
                    bucketJson->CreateNumberProperty("NumReadings", (int)currentBucketInfo.numReadings);
                    bucketJson->CreateNumberProperty("CurrentTime", (double)currentBucketInfo.currentTime);
                    buckets.push_back(bucketJson);
                } else {
                    auto bucket = _manager.GetBucket(i);
                     printf("Offset: %x, Index: %x, Start: %x, End: %x, DL: %X\n",(int)bucket.Offset, (int)bucket.Index, (int)bucket.BlockStartTime, (int)bucket.BlockEndTime, (int)bucket.DataLength);
                    bucketJson->CreateNumberProperty("Offset", (int)bucket.Offset);
                    bucketJson->CreateNumberProperty("Index", (int)bucket.Index);
                    if(bucket.BlockStartTime!=0xFFFFFFFFFFFFFFFF)
                        bucketJson->CreateNumberProperty("BlockStartTime", (double)bucket.BlockStartTime);
                    if(bucket.BlockEndTime!=0xFFFFFFFFFFFFFFFF)
                        bucketJson->CreateNumberProperty("BlockEndTime", (double)bucket.BlockEndTime);
                    bucketJson->CreateNumberProperty("NumReadings", (int)bucket.NumReadings);
                    bucketJson->CreateNumberProperty("DataLength", (double)bucket.DataLength);
                    buckets.push_back(bucketJson);
                }
            }
            pResult.CreateArrayProperty("Buckets", buckets);
        } else {
            pResult.CreateBoolProperty("Status", false);
            return;    
        }

        pResult.CreateBoolProperty("Status", true);
    } else {
        pResult.CreateBoolProperty("Status", false);
    }   
}

std::string DataManagementCommand::GetName() {
    return "DATA";
}


MqttManagementCommand::MqttManagementCommand(MqttManager& pMqttManager, GeneralSettings& pSettings) : _mqttManager(pMqttManager), _settings(pSettings) {

}

void MqttManagementCommand::Process(Json& pJson,Json& pResult) {
    if(pJson.HasProperty("Mode")) {
        auto mode = pJson.GetStringProperty("Mode");
        if(mode=="GetValues") {   
            std::vector<Json*> availableValues;
            std::vector<Json*> setReadingsValues;
            std::vector<Json*> setInfoValues;
            for(auto groupPair : ValueController::GetCurrent().GetGroups()) {                
                for(auto keyPair : groupPair.second->SourcesByName) {
                    auto source = keyPair.second->DefaultSource;
                    if(source->IsIncludedInMQTTReadings() ) {
                        Json* setJson = new Json();
                        setJson->CreateStringProperty("Group", groupPair.first);    
                        setJson->CreateStringProperty("Name", keyPair.first);
                        setReadingsValues.push_back(setJson);
                    }
                    if(source->IsIncludedInMQTTInfo() ) {
                        Json* setJson = new Json();
                        setJson->CreateStringProperty("Group", groupPair.first);    
                        setJson->CreateStringProperty("Name", keyPair.first);
                        setInfoValues.push_back(setJson);
                    }

                    if(source->GetFlags()!=VALUESOURCE_NOFLAGS ) {
                        Json* availJson = new Json();
                        availJson->CreateStringProperty("Group", groupPair.first);    
                        availJson->CreateStringProperty("Name", keyPair.first);
                        availableValues.push_back(availJson);
                    }                
                }   
            } 
            pResult.CreateBoolProperty("Enable",_mqttManager.GetEnable());
            pResult.CreateStringProperty("ServerAddress",_mqttManager.GetServerAddress());
            pResult.CreateStringProperty("Username",_mqttManager.GetUsername());
            pResult.CreateStringProperty("Password",_mqttManager.GetPassword());
            pResult.CreateStringProperty("ReadingsTopicPath",_mqttManager.GetReadingsTopic());
            pResult.CreateStringProperty("InfoTopicPath",_mqttManager.GetInfoTopic());
            pResult.CreateNumberProperty("PublishDelay",_mqttManager.GetPublishDelay());
            pResult.CreateArrayProperty("Available", availableValues);
            pResult.CreateArrayProperty("ReadingsTopic", setReadingsValues);
            pResult.CreateArrayProperty("InfoTopic", setInfoValues);
        } else if (mode == "SetValues") {
            if(pJson.HasArrayProperty("ReadingsTopic") && pJson.HasArrayProperty("InfoTopic")) {
                for( const auto &group : ValueController::GetCurrent().GetGroups()) {
                    for(const auto &source : group.second->SourcesByName) {
                        auto valueByName = source.second;
                        for(const auto valueSource : valueByName->Sources)
                        {
                            valueSource->SetIsIncludedInMQTTReadings(false);
                            valueSource->SetIsIncludedInMQTTInfo(false);
                        }                
                    }
                }
                auto readingsProp = pJson.GetObjectProperty("ReadingsTopic");
                std::vector<Json*> readingsElements;
                readingsProp->GetAsArrayElements(readingsElements);
                delete readingsProp;
                for(auto setElement : readingsElements) {
                    if(setElement->HasProperty("Group") && setElement->HasProperty("Name")) {
                        auto group = setElement->GetStringProperty("Group");
                        auto name = setElement->GetStringProperty("Name");
                        
                        if(!ValueController::GetCurrent().GetGroups().contains(group)) {
                            continue;
                        }
                        auto  valueGroup = ValueController::GetCurrent().GetGroups()[group];
                        if(!valueGroup->SourcesByName.contains(name)) continue;
                        auto valueContainer = valueGroup->SourcesByName[name];
                        valueContainer->DefaultSource->SetIsIncludedInMQTTReadings(true);
                    }
                }
                auto infoProp = pJson.GetObjectProperty("InfoTopic");
                std::vector<Json*> infoElements;
                infoProp->GetAsArrayElements(infoElements);
                delete infoProp;
                for(auto setElement : infoElements) {
                    if(setElement->HasProperty("Group") && setElement->HasProperty("Name")) {
                        auto group = setElement->GetStringProperty("Group");
                        auto name = setElement->GetStringProperty("Name");
                        
                        if(!ValueController::GetCurrent().GetGroups().contains(group)) {
                            continue;
                        }
                        auto  valueGroup = ValueController::GetCurrent().GetGroups()[group];
                        if(!valueGroup->SourcesByName.contains(name)) continue;
                        auto valueContainer = valueGroup->SourcesByName[name];
                        valueContainer->DefaultSource->SetIsIncludedInMQTTInfo(true);                    
                    }
                }
                ValueController::GetCurrent().Save();
            }    
            
            if(pJson.HasProperty("Enable")) 
                _mqttManager.SetEnable(pJson.GetBoolProperty("Enable"));        

            if(pJson.HasProperty("ServerAddress")) 
                _mqttManager.SetServerAddress(pJson.GetStringProperty("ServerAddress"));

            if(pJson.HasProperty("Username")) 
                _mqttManager.SetUsername(pJson.GetStringProperty("Username"));

            if(pJson.HasProperty("Password")) 
                _mqttManager.SetPassword(pJson.GetStringProperty("Password"));
            
            if(pJson.HasProperty("ReadingsTopicPath")) 
                _mqttManager.SetReadingsTopic(pJson.GetStringProperty("ReadingsTopicPath"));

            if(pJson.HasProperty("InfoTopicPath")) 
                _mqttManager.SetInfoTopic(pJson.GetStringProperty("InfoTopicPath"));

            if(pJson.HasProperty("PublishDelay")) 
                _mqttManager.SetPublishDelay(pJson.GetIntProperty("PublishDelay"));        

            _mqttManager.Save(); 
        }             

        pResult.CreateBoolProperty("Status", true);
    } else {
        pResult.CreateBoolProperty("Status", false);
    }
}

std::string MqttManagementCommand::GetName() {
    return "MQTT";
}

