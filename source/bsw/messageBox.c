/**
* \file <filename>
* \author <author-name>
* \date <date>
*
* \brief <Symbolic File name>
*
* \copyright Copyright ©2016
* Department of electrical engineering and information technology, Hochschule Darmstadt - University of applied sciences (h_da). All Rights Reserved.
* Permission to use, copy, modify, and distribute this software and its documentation for educational, and research purposes in the context of non-commercial
* (unless permitted by h_da) and official h_da projects, is hereby granted for enrolled students of h_da, provided that the above copyright notice,
* this paragraph and the following paragraph appear in all copies, modifications, and distributions.
* Contact Prof.Dr.-Ing. Peter Fromm, peter.fromm@h-da.de, Birkenweg 8 64295 Darmstadt - GERMANY for commercial requests.
*
* \warning This software is a PROTOTYPE version and is not designed or intended for use in production, especially not for safety-critical applications!
* The user represents and warrants that it will NOT use or redistribute the Software for such purposes.
* This prototype is for research purposes only. This software is provided "AS IS," without a warranty of any kind.
*/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/

#include "message.h"
#include "drawer.h"
#include "messageBox.h"

/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/

/*****************************************************************************/
/* Global variable definitions (declared in header file with 'extern')       */
/*****************************************************************************/

/*****************************************************************************/
/* Local type definitions ('typedef')                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Local variable definitions ('static')                                     */
/*****************************************************************************/



/*****************************************************************************/
/* Local function prototypes ('static')                                      */
/*****************************************************************************/


/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/

RC_t MSG_init(MSG_messagebox_t *const me, uint16_t const size, EventMaskType const ev, TaskType const task){    
    me->readIdx = 0;
    me->writeIdx = 0;
    me->size= 100;
    me->fillLevel = 0;
    me->m_task = task;
    me->m_ev = ev;
    me->p_Buffer = (message_t *)malloc(me->size * (sizeof(message_t)));
    if (me->p_Buffer == NULL) {
        UART_LOG_PutString("Error: Memory allocation failed for ring buffer\r\n");
        return RC_ERROR_MEMORY;
    }
    return RC_SUCCESS;
}

RC_t MSG_sendMessage(MSG_messagebox_t *const me, void const* const pData, uint16_t const size){
    if(me->fillLevel < me->size){
        message_t *msg = (message_t *)pData;
        me->p_Buffer[me->writeIdx++] = *msg;
        me->writeIdx%=me->size;
        me->fillLevel++;
        SetEvent(me->m_task,me->m_ev);
        return RC_SUCCESS;
    }
    else{ 
        return RC_BUFFERUNDERFLOW;
    }
}

RC_t MSG_waitNextMessage(MSG_messagebox_t *const me){
    if(me->fillLevel>0){
        WaitEvent(me->m_ev);
        ClearEvent(me->m_ev);
        return RC_SUCCESS;
    }else{
        return RC_ERROR;
    }
}

RC_t MSG_getSizeOfNextMessage(MSG_messagebox_t *const me, uint16_t *const size){
    if(me->fillLevel > 0){
        uint8_t nextMsg = (me->readIdx) % me->size;
        *size = me->p_Buffer[nextMsg].size;
        return RC_SUCCESS;
    }else{
        return RC_BUFFERUNDERFLOW;
    }
}

RC_t MSG_receiveMessage(MSG_messagebox_t *const me, void* pData, uint16_t const size){
    if(me->fillLevel>0){
       message_t *msg = (message_t*)pData;
        *msg = me->p_Buffer[me->readIdx]; 
        if (me->p_Buffer[me->readIdx].drawer != NULL) {
            msg->drawer = malloc(sizeof(drawer_t));
            if (msg->drawer != NULL) {
                *(msg->drawer) = *(me->p_Buffer[me->readIdx].drawer);
            }
        }

        me->readIdx = (me->readIdx + 1) % me->size;
        me->fillLevel--;
    }else{
        return RC_BUFFERUNDERFLOW;
    }
}