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

#include "parserNew.h"
#include <string.h>
#include <stdio.h>

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
RC_t PARSER_init(Parser_t *const me) {
    if (!me) return RC_ERROR;
    
    // Initialize the jsmn parser
    jsmn_init(&me->parser);
    
    // Clear the content buffer
    memset(me->content, 0, PARSERSTRINGSIZE);
    
    // Initialize positions
    me->nextFreePos = 0;
    me->nextToken = 0;
    
    return RC_SUCCESS;
}

RC_t PARSER_clear(Parser_t *const me) {
    if (!me) return RC_ERROR;
    
    // Reset the parser
    jsmn_init(&me->parser);
    
    // Clear content buffer
    memset(me->content, 0, PARSERSTRINGSIZE);
    
    // Reset positions
    me->nextFreePos = 0;
    me->nextToken = 0;
    
    return RC_SUCCESS;
}

RC_t PARSER_addChar(Parser_t *const me, char data) {
    if (!me) return RC_ERROR;
    
    // Check if there's space in the buffer
    if (me->nextFreePos >= PARSERSTRINGSIZE - 1) {
        return RC_ERROR_BUFFER_FULL;
    }
    
    // Add character to buffer
    me->content[me->nextFreePos++] = data;
    me->content[me->nextFreePos] = '\0';  // Ensure null termination
    
    return RC_SUCCESS;
}

RC_t PARSER_addEndl(Parser_t *const me) {
    if (!me) return RC_ERROR;
    
    // Check if there's space in the buffer
    if (me->nextFreePos >= PARSERSTRINGSIZE - 1) {
        return RC_ERROR_BUFFER_FULL;
    }
    
    // Add newline character
    me->content[me->nextFreePos++] = '\n';
    me->content[me->nextFreePos] = '\0';  // Ensure null termination
    
    return RC_SUCCESS;
}

RC_t PARSER_parse(Parser_t *const me) {
    if (!me) return RC_ERROR;
    
    // Reset the parser
    jsmn_init(&me->parser);
    
    // Parse the JSON string
    int result = jsmn_parse(&me->parser, 
                           me->content, 
                           strlen(me->content), 
                           me->token, 
                           PARSERMAXTOKEN);
    
    // Reset token iterator
    me->nextToken = 0;
    
    // Check parsing result
    if (result < 0) {
        return RC_ERROR_PARSER;
    }
    
    return RC_SUCCESS;
}

RC_t PARSER_getNextToken(Parser_t *const me, jsmntok_t *const token) {
    if (!me || !token) return RC_ERROR;
    
    // Check if we've reached the end of tokens
    if (me->nextToken >= PARSERMAXTOKEN) {
        return RC_ERROR_TOKEN_END;
    }
    
    // Check if the current token is valid
    if (me->token[me->nextToken].type == JSMN_UNDEFINED) {
        return RC_ERROR_TOKEN_INVALID;
    }
    
    // Copy token data
    *token = me->token[me->nextToken++];
    
    return RC_SUCCESS;
}

RC_t PARSER_resetNextToken(Parser_t *const me) {
    if (!me) return RC_ERROR;
    
    me->nextToken = 0;
    return RC_SUCCESS;
}

RC_t PARSER_dbg_printContent(Parser_t *const me) {
    if (!me) return RC_ERROR;
    
    printf("Parser Content:\n%s\n", me->content);
    return RC_SUCCESS;
}

RC_t PARSER_dbg_printToken(Parser_t *const me) {
    if (!me) return RC_ERROR;
    
    printf("Parser Tokens:\n");
    for (uint16_t i = 0; i < PARSERMAXTOKEN; i++) {
        if (me->token[i].type == JSMN_UNDEFINED) break;
        
        printf("Token %d: type=%d, start=%d, end=%d, size=%d\n",
               i,
               me->token[i].type,
               me->token[i].start,
               me->token[i].end,
               me->token[i].size);
    }
    
    return RC_SUCCESS;
}