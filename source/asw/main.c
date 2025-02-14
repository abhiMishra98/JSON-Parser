/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <stdio.h>
#include "global.h"
#include "parserNew.h"
#include "CRingBuffer.h"
#include "drawer.h"
#include "messageBox.h"
#include "tft/tft.h"

CRingBuffer ringBuffer;
Parser_t jsonParser;
MSG_messagebox_t messageBox;

CY_ISR_PROTO(isr_uartRx);
void systick_handler(void) {
    CounterTick(cnt_systick);
}
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    isr_uart_StartEx(isr_uartRx);
    UART_LOG_Start();
    TFT_init();
    TFT_setBacklight(100);
    TFT_print("Hello world\n");
    ringBufferCreate(&ringBuffer);
    PARSER_init(&jsonParser); 
    MSG_init(&messageBox,100,ev_sendMsg,tsk_hmi);
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    EE_systick_set_period(MILLISECONDS_TO_TICKS(1, BCLK__BUS_CLK__HZ));
    EE_systick_enable_int();
    
    for(;;)
    {
         StartOS(OSDEFAULTAPPMODE);
        /* Place your application code here. */
    }
}
TASK(tsk_init){
    EE_system_init();
    EE_systick_start();
    ActivateTask(tsk_json);
    ActivateTask(tsk_hmi);
    TerminateTask();
}

TASK(tsk_background){
    while(1)
    {
        //do something with low prioroty
        __asm("nop");
    }
    TerminateTask();
}

TASK(tsk_hmi){
    EventMaskType activeEvents;
    while(1){
        WaitEvent(ev_sendMsg);
        GetEvent(tsk_hmi,&activeEvents);
        if(activeEvents & ev_sendMsg){
            char buffer[128]={0};
            uint16_t sizeMsg;           
            if (MSG_getSizeOfNextMessage(&messageBox, &sizeMsg) == RC_SUCCESS) {
                sprintf(buffer, "Size of the msg from task hmi : %d\r\n", sizeMsg);
                UART_LOG_PutString(buffer);
                message_t *rcvMsg = (message_t *)malloc(sizeof(message_t));
                MSG_receiveMessage(&messageBox, rcvMsg, sizeMsg);
                UART_LOG_PutString("Received Message:\r\n");
                    sprintf(buffer, "Size: %d\r\n", rcvMsg->size);
                    UART_LOG_PutString(buffer);
                    if (rcvMsg->drawer != NULL) {
                        sprintf(buffer, "Drawer Coordinates from HMI TASK: %d, %d, %d, %d\r\n",
                                rcvMsg->drawer->data.coordinate[0],
                                rcvMsg->drawer->data.coordinate[1],
                                rcvMsg->drawer->data.coordinate[2],
                                rcvMsg->drawer->data.coordinate[3]);
                        UART_LOG_PutString(buffer);
                        TFT_writeLine(rcvMsg->drawer->data.coordinate[0], rcvMsg->drawer->data.coordinate[1], rcvMsg->drawer->data.coordinate[2], rcvMsg->drawer->data.coordinate[3], RED);
                        freeMessage(rcvMsg);
                    }
            }
        }
       ClearEvent(activeEvents); 
    }
    TerminateTask();
}

TASK(tsk_json) {
    EventMaskType ev = 0;
    EventMaskType activeEvents;
    while(1) {
        WaitEvent(ev_eos);
        GetEvent(tsk_json, &activeEvents); 
        if(activeEvents & ev_eos) {
            PARSER_clear(&jsonParser);
            uint8_t ringBData;
            unsigned short fillLevel = ringBuffer.m_fillLevel;
            char debug[32];
            for(unsigned short count = 0; count < fillLevel; count++) {
                ringBufferRead(&ringBuffer, &ringBData); //Reading from ringbuffer
                PARSER_addChar(&jsonParser, (char)ringBData); //Writing characters to the jsonparser char type
            }
            UART_LOG_PutString("Received JSON: ");
            UART_LOG_PutString(jsonParser.content);
            UART_LOG_PutString("\r\n");
            int parseResult = jsmn_parse(&jsonParser.parser, 
                                jsonParser.content, 
                                strlen(jsonParser.content), 
                                jsonParser.token, 
                                PARSERMAXTOKEN);       
            jsmntok_t token;
            PARSER_resetNextToken(&jsonParser); //Reset next token to 0
            RC_t tokenResult = PARSER_getNextToken(&jsonParser, &token);
            UART_LOG_PutString("Starting to process tokens\r\n");
            while(PARSER_getNextToken(&jsonParser, &token) == RC_SUCCESS) {
                if(token.type == JSMN_STRING) {
                        char key[10];
                        int keyLen = token.end - token.start;
                        strncpy(key, &jsonParser.content[token.start], keyLen);
                        key[keyLen] = '\0';
                        sprintf(debug, "Found key: %s\r\n", key);
                        UART_LOG_PutString(debug);
                        jsmntok_t valueToken;
                        RC_t valueResult = PARSER_getNextToken(&jsonParser, &valueToken);                      
                        if(valueResult != RC_SUCCESS) {
                            UART_LOG_PutString("Failed to get value token\r\n");
                            continue;
                        }                       
                        if(strcmp(key, "c") == 0) {
                            if(valueToken.type == JSMN_STRING) {
                                char colorValue[20];
                                drawer_t *drawer = malloc(sizeof(drawer_t)); //dynamically creating drawer object
                                if (drawer != NULL) {
                                    memset(drawer, 0, sizeof(drawer_t));
                                }
                                message_t *message = malloc(sizeof(message_t)); //dynamically creating message object containing drawer and size of the drawer
                                if (message != NULL) {
                                    memset(drawer, 0, sizeof(message_t));
                                }
                                initMessage(message);
                                int valueLen = valueToken.end - valueToken.start;
                                drawer->data.color = malloc(valueLen + 1);
                                if (drawer->data.color == NULL) {
                                        freeDrawer(drawer); 
                                        UART_LOG_PutString("Memory allocation for color failed!\r\n");
                                        continue;
                                }
                                strncpy(drawer->data.color, &jsonParser.content[valueToken.start], valueLen);
                                drawer->data.color[valueLen] = '\0';
                               
                                drawer->command = DRAWER_CMD_COLOR;                    
                                createMessage(message,sizeof(message_t),drawer);
                                MSG_sendMessage(&messageBox,message,sizeof(message_t));
                                sprintf(debug, "Color value: %s\r\n", drawer->data.color);
                                UART_LOG_PutString(debug);
                            }
                        }
                        else if(strcmp(key, "d") == 0) {
                            if(valueToken.type == JSMN_ARRAY) {
                                drawer_t *drawer = malloc(sizeof(drawer_t));
                                message_t *message = malloc(sizeof(message_t));
                                initMessage(message);
                                sprintf(debug, "Found array with size: %d\r\n", valueToken.size);
                                UART_LOG_PutString(debug);
                                    
                                for(int i = 0; i < valueToken.size; i++) {
                                    jsmntok_t arrayToken;
                                    PARSER_getNextToken(&jsonParser, &arrayToken);
                                    
                                    char numStr[10];
                                    int numLen = arrayToken.end - arrayToken.start;
                                    strncpy(numStr, &jsonParser.content[arrayToken.start], numLen);
                                    numStr[numLen] = '\0'; //Added for atoi
                                   
                                    int number = atoi(numStr);//string to integer
                                    drawer->data.coordinate[i]= number;
                                    createMessage(message,sizeof(drawer),drawer);
                                    MSG_sendMessage(&messageBox,message,sizeof(message_t));
                                    sprintf(debug, "Array value[%d]: %d\r\n", i, number);
                                    UART_LOG_PutString(debug);
                                }
                            }
                        }
                    }
                }         
        }
        ClearEvent(activeEvents);
    }
    TerminateTask();
}

void unhandledException()
{
    //Ooops, something terrible happened....check the call stack to see how we got here...
    __asm("bkpt");
}

// If a READ opern. is done through UART and a ClearRxBuffer() is called over the same UART by another task/ISR, 
// it might lead to data corruption.
ISR2(isr_uartRx){
    isr_uart_ClearPending();
    static int count = 0;
    uint8_t rxData;
    rxData = UART_LOG_ReadRxData();
    if(rxData !='\0'){
        ringBufferWrite(&ringBuffer,rxData);    
    } else{
       SetEvent(tsk_json,ev_eos);
    }
}
