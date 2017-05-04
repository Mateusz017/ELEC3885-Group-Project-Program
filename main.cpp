/*

Copyright (c) 2012-2014 RedBearLab

Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/*
 *    The application works with the BLEController iOS/Android App.
 *    Type something from the Terminal to send
 *    to the BLEController App or vice verse.
 *    Characteristics received from App will print on Terminal.
 */
 
#include "mbed.h"
#include "ble/BLE.h"


#define BLE_UUID_TXRX_SERVICE            0x0000 /**< The UUID of the Nordic UART Service. */
#define BLE_UUID_TX_CHARACTERISTIC       0x0002 /**< The UUID of the TX Characteristic. */
#define BLE_UUIDS_RX_CHARACTERISTIC      0x0003 /**< The UUID of the RX Characteristic. */

#define TXRX_BUF_LEN                     20

BLE  ble;

//Serial pc(USBTX, USBRX);
Serial device(P0_9, P0_11);  // tx, rx

// The Nordic UART Service
static const uint8_t uart_base_uuid[] = {0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_tx_uuid[]   = {0x71, 0x3D, 0, 3, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_rx_uuid[]   = {0x71, 0x3D, 0, 2, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_base_uuid_rev[] = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0, 0, 0x3D, 0x71};


uint8_t txPayload[TXRX_BUF_LEN] = {0,};
uint8_t rxPayload[TXRX_BUF_LEN] = {0,};

static uint8_t rx_buf[TXRX_BUF_LEN];
static uint8_t rx_len=0;


GattCharacteristic  txCharacteristic (uart_tx_uuid, txPayload, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
                                      
GattCharacteristic  rxCharacteristic (uart_rx_uuid, rxPayload, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
                                      
GattCharacteristic *uartChars[] = {&txCharacteristic, &rxCharacteristic};

GattService         uartService(uart_base_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));



void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    //device.printf("Disconnected \r\n");
    //device.printf("Restart advertising \r\n");
    ble.startAdvertising();
}

void WrittenHandler(const GattWriteCallbackParams *Handler) //app to ble to mcu 
{   
    uint8_t buf[TXRX_BUF_LEN];
    uint16_t bytesRead, index;
    
    if (Handler->handle == txCharacteristic.getValueAttribute().getHandle()) 
    {
        ble.readCharacteristicValue(txCharacteristic.getValueAttribute().getHandle(), buf, &bytesRead); //app to ble
        memset(txPayload, 0, TXRX_BUF_LEN);
        memcpy(txPayload, buf, TXRX_BUF_LEN);       
        //device.printf("WriteHandler \r\n");
        //device.printf("Length: ");
        device.putc(bytesRead);  //device is ble puts msg from app to mcu
        //device.printf("\r\n");
        //device.printf("Data: ");
        for(index=0; index<bytesRead; index++)
        {
            device.putc(txPayload[index]);        
        }
        //device.printf("\r\n");
    }
}
void uartCB(void) //mcu to ble to app 
{   
    while(device.readable())    
    {
        rx_buf[rx_len++] = device.getc();    //get msg from mcu as soon as its 6 characters 
        if(rx_len>=6 || rx_buf[rx_len-1]=='\0' || rx_buf[rx_len-1]=='\n')  //when 6 characters are stored in buffer it sends the msg to ble
        {
            ble.updateCharacteristicValue(rxCharacteristic.getValueAttribute().getHandle(), rx_buf, rx_len); //this changes characteristic value which is updated in app
            //device.printf("RecHandler \r\n");
            //device.printf("Length: ");
            device.putc(rx_len); 
            //device.printf("\r\n");
            rx_len = 0;
            break;
        }
    }
}
int main(void)
{
    ble.init(); //initialize ble protocol stack
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(WrittenHandler);  
    
    //pc.baud(9600);
    device.baud(9600);
    //device.printf("SimpleChat Init \r\n");
    
    device.attach( uartCB , device.RxIrq); //event interrupt wakes up ble when smth is sent to mcu or received from mcu
   // setup advertising 
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED); 
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                    (const uint8_t *)"Biscuit", sizeof("Biscuit") - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                                    (const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid));
    // 100ms; in multiples of 0.625ms. 
    ble.setAdvertisingInterval(160);
    //once connected to peripheral stop advertising

    ble.addService(uartService); //exchanges data service which reads or writes to ble to communicate in both directions
    //assign service, ble wakes up otherwsie sleeps to save power
    
    ble.startAdvertising(); 
    //device.printf("Advertising Start \r\n");
    
    while(1)
    {
        ble.waitForEvent(); 
    }
}








