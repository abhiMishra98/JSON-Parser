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

#include "CRingBuffer.h"

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

/**
 * <Description>
 * \param <first para>
 * \return <return value>
 */
//RC_t FILE_function(type para)

//If a create operation is done by a task on the ringbuffer and another task attempts to write/read/delete the same ringbuffer.
RC_t ringBufferCreate(CRingBuffer *rb){       
        rb->m_size= 255;
    	rb->m_readIdx= 0;
    	rb->m_writeIdx= 0;
    	rb->m_fillLevel= 0;
        rb->m_pBuffer= (uint8_t *)malloc(rb->m_size * sizeof(uint8_t));
        return RC_SUCCESS;
}

//If a delete operation is done by a task on the ringbuffer and another task attempts to write/read/instantiate the same ringbuffer.
RC_t ringBufferDelete(CRingBuffer *rb){
    StatusType status = GetResource(erika_res);
    if (rb == NULL || rb->m_pBuffer == NULL) {
        return RC_ERROR; // Fail if the buffer is not initialized
    }

    free(rb->m_pBuffer); // Free the allocated memory
    rb->m_pBuffer = NULL; // Set the pointer to NULL for safety
    rb->m_size = 0;       // Reset the buffer size
    rb->m_writeIdx = 0;   // Reset the write index
    rb->m_fillLevel = 0;  // Reset the fill level

    return RC_SUCCESS;    // Return success

}

//If a write operation is done by a task on the ringbuffer and another task attempts to delete/read/instantiate the same ringbuffer.
RC_t ringBufferWrite(CRingBuffer *rb, uint8_t data){
    if (rb == NULL || rb->m_pBuffer == NULL) {
        return RC_ERROR; // Fail if the buffer is not initialized
    }
        
        if(rb->m_fillLevel < rb->m_size){
            GetResource(erika_res);
            rb->m_pBuffer[rb->m_writeIdx++] = data;
            rb->m_writeIdx%=rb->m_size;
            rb->m_fillLevel++;
            ReleaseResource(erika_res);
            return RC_SUCCESS;
        }
        else{
            return RC_BUFFERUNDERFLOW;
        }
}

//If a read operation is done by a task on the ringbuffer and another task attempts to delete/write/instantiate the same ringbuffer.
RC_t ringBufferRead(CRingBuffer *rb, uint8_t *data){
    if (rb == NULL || rb->m_pBuffer == NULL) {
        return RC_ERROR; // Fail if the buffer is not initialized
    }
    if(rb->m_fillLevel >0){
        GetResource(erika_res);
        *data = rb->m_pBuffer[rb->m_readIdx++];
        rb->m_readIdx %= rb->m_size; // Wrap the read index
        rb->m_fillLevel--;
        ReleaseResource(erika_res);
        return RC_SUCCESS;
    }
    else{
        return RC_BUFFERUNDERFLOW;
    }
}
