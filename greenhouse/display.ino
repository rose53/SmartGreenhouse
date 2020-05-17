
const int icon_wifi_pos_x = 0;  
const int icon_wifi_pos_y = 1; 
const int icon_data_pos_x = icon_wifi_pos_x;  
const int icon_data_pos_y = icon_wifi_pos_y + 22;  
const int icon_disturber_pos_x = icon_wifi_pos_x;  
const int icon_disturber_pos_y = icon_data_pos_y + 22; 
const int icon_noise_pos_x = icon_wifi_pos_x;  
const int icon_noise_pos_y = icon_data_pos_y + 22;
const int data_pos_x = 25;

const int    displaysCount      = 3;
volatile int currentDisplayIndx = 0;

void displayTemperature() {
    display.setFont(&FreeSans12pt7b);
    display.setCursor(data_pos_x,(display.height() + 12) / 2);
    display.setTextSize(1);
    display.print(displayData.temperature,1);
    display.drawBitmap(icon_data_pos_x,icon_data_pos_y, temperature_icon_bmp, temperature_icon_width, temperature_icon_height, BLACK); 
    display.setFont();
}

void displayHumidity() {
    display.setFont(&FreeSans12pt7b);
    display.setCursor(data_pos_x,(display.height() + 12) / 2);
    display.setTextSize(1);
    display.print(displayData.humidity,0);
    display.drawBitmap(icon_data_pos_x,icon_data_pos_y, humidity_icon_bmp, humidity_icon_width, humidity_icon_height, BLACK); 
    display.setFont();
}

void displayMoisture() {
    display.setFont(&FreeSans12pt7b);
    display.setCursor(data_pos_x,(display.height() + 12) / 3);
    display.setTextSize(1);
    display.print(displayData.moisture1);
    display.setCursor(data_pos_x,(display.height() + 12) / 3 + 24);
    display.print(displayData.moisture2);
    //display.drawBitmap(icon_data_pos_x,icon_data_pos_y, temperature_icon_bmp, temperature_icon_width, temperature_icon_height, BLACK); 
    display.setFont();
}


void (*displays[displaysCount])(void)= {displayTemperature, displayHumidity, displayMoisture};

void doDisplay() {

    display.clearDisplay();

    if (displayData.wifiConnected) {        
        display.drawBitmap(icon_wifi_pos_x,icon_wifi_pos_y, wifi_icon_bmp, wifi_icon_width, wifi_icon_height, BLACK);    
    } else {
        display.drawBitmap(icon_wifi_pos_x,icon_wifi_pos_y, no_wifi_icon_bmp, no_wifi_icon_width, no_wifi_icon_height, BLACK);    
    }    

    (*displays[currentDisplayIndx])();   
    
    currentDisplayIndx = (currentDisplayIndx + 1) % displaysCount; 

    display.display();
}
