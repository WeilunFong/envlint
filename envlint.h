/*****************************************************************************/
/** 
 * \file        envlint.h
 * \author      Weilun Fong | wlf@zhishan-iot.tk
 * \brief       
 * \note        
 * \version     v0.0
 * \ingroup     envlint
******************************************************************************/

#ifndef _ENVLINT_H_
#define _ENVLINT_H_

/*****************************************************************************
 *                             header file                                   *
 *****************************************************************************/
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#if (HAVE_CJSON != 1)
    #define CJSONEMBED "(libcjson embed)"
#else
    #define CJSONEMBED ""
#endif

/**
 * \brief getopt index defin
 */
#define ENVLINT_GETOPT_INDEX_IGNORE_ALL_FAILURE  0x01

/**
 * \brief JSON key define
 */
#define ENVLINT_JSON_KEY_CHECK    "check"
#define ENVLINT_JSON_KEY_COMMAND  "command"
#define ENVLINT_JSON_KEY_FEATURE  "feature"
#define ENVLINT_JSON_KEY_FORCE    "force"
#define ENVLINT_JSON_KEY_ITEMS    "items"
#define ENVLINT_JSON_KEY_NAME     "name"
#define ENVLINT_JSON_KEY_RETURN   "return"

/**
 * \brief buffer size define
 */
#define ENVLINT_BUFFERSIZE        256

/**
 * \brief version define
 */
#define ENVLINT_MAJOR_VERSION      0
#define ENVLINT_MINOR_VERSION      0
#define ENVLINT_PATCH_VERSION      0

/*****************************************************************************
 *                          function declare                                 *
 *****************************************************************************/
int   envlint_check_featrues(cJSON *feature, bool ignore_failure);
int   envlint_check_items(cJSON *items, bool ignore_failure);
char* envlint_get_json_file_content(char *path);
/*****************************************************************************/
int   envlint_usage(void);
int   envlint_version(void);

#endif

