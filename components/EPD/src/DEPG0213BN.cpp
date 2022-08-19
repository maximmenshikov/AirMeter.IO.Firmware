#include "DEPG0213BN.h"
#include "EPDDrawTarget.h"


DEPG0213BN::DEPG0213BN(SSD1680& pSsd1680) : _ssd1680(pSsd1680) {
    _backBuffer->Fill(White);
    _gfx = new EPDDrawTarget(*_backBuffer);
}

DEPG0213BN::~DEPG0213BN() {
    delete _gfx;
}

SSD1680Lut DEPG0213BN::LUTPartialUpdate = {    
    .Data = {
    //     0x0,    0x40,   0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x80,   0x80,   0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x40,   0x40,   0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x80,   0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,  
    // 0xF,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   
    // 0x4,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x1,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x1,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0, 
    // 0x22,   0x22,   0x22,   0x22,   0x22,   0x22,   
    // 0x0,    0x0,    0x0, 

        // 0x0,    0x40,   0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x80,   0x80,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x40,   0x40,   0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x80,   0x0,    0x0,    0x0,    0x0,

        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0xF,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x1,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,

        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x1,    0x0,    0x0,    0x0,
        
        // 0x0,    0x0,    0x0,    0x1,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
        // 0x0,    0x0,    0x0,    0x0,    0x22,   0x22,   0x22,
        // 0x22,   0x22,   0x22,   0x0,    0x0,    0x0 

// // rather good
// 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 
// 0x00, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
// 0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
// 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x80, 0x00, 0x80, 
// 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x0A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x00, 
// 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 
// 0x00, 0x0A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x00, 0x00, 0x00, 
// 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 
// 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x0A, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
// 0x00, 0x00, 0x00, 
// //better?
// 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
// 0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
// 0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
// 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x80, 0x80, 0x80, 
// 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x04, 0x00, 
// 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 
// 0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 
// 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
// 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x05, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
// 0x00, 0x00, 0x00, 

// was working
// 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
// 0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
// 0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
// 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 
// 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0A, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 
// 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x04, 0x00, 
// 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 
// 0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 
// 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x04, 0x00, 0x00, 0x00, 
// 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 
// 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x00, 0x00, 
// 0x00, 0x00, 0x00, 0x05, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
// 0x00, 0x00, 0x00, 

// new block
0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
0x00, 0x00, 0x00,  
//   0x0,    0x40,   0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x80,   0x80,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x40,   0x40,   0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x80,   0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0xF,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x1,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x1,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x1,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
    // 0x0,    0x0,    0x0,    0x0,    0x22,   0x22,   0x22,
    // 0x22,   0x22,   0x22,   0x0,    0x0,    0x0


//    0x0,    0x40,   0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x80,   0x80,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x40,   0x40,   0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x80,   0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0xF,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x1,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x1,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x1,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,
//     0x0,    0x0,    0x0,    0x0,    0x22,   0x22,   0x22,
//     0x22,   0x22,   0x22,   0x0,    0x0,    0x0,
    },
    .GateDrivingVoltage = 200, // 20.0V
    .SourceDrivingVoltage = {
        .Vsh1 = 150, // 15.0V
        .Vsh2 = 58,   // Forces 0 value which from SSD1680's datasheet is an unsupported value
        .Vsl = -150  // -15.0V
    },
    .VCOM = 0x32 // This does not seem to be a supported value either!
};

void DEPG0213BN::RenderToDisplay(bool pFastUpdate) {
    if(pFastUpdate) {
        _ssd1680.ResetAll();
        _ssd1680.SetRamDataEntryMode(XIncreaseYIncrease, WIDTH-1, HEIGHT);

        _ssd1680.SendLUT(LUTPartialUpdate);
        _ssd1680.SendEndLUT(SSD1680LutEndOption::Normal);
    
        _ssd1680.EnablePingPong();
        _ssd1680.SetBorderWaveForm(0x80);
        _ssd1680.WriteToBWRam(*_backBuffer);
        _ssd1680.SetDisplayUpdateSequence(SSD1680DisplayUpdateSequence::ClockOnAnalogOnMode2AnalogOffOSCOff);
        _ssd1680.ActivateDisplayUpdateSequence2(300);
        vTaskDelay(300 / portTICK_RATE_MS);
        _ssd1680.SetSleepMode(SSD1306SleepMode::DeepSleepMode1);
    } else {
        _ssd1680.ResetAll();
        _ssd1680.SetRamDataEntryMode(XIncreaseYIncrease, WIDTH-1, HEIGHT);
        _ssd1680.WriteToBWRam(*_backBuffer);
        _ssd1680.ActivateDisplayUpdateSequence(1200);
        _ssd1680.SetSleepMode(SSD1306SleepMode::DeepSleepMode1);
    }
}
DrawTarget* DEPG0213BN::GetDrawTarget() {
    return _gfx;
}