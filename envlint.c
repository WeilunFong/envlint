/*****************************************************************************/
/** 
 * \file        envlint.c
 * \author      Weilun Fong | wlf@zhishan-iot.tk
 * \brief       
 * \note        
 * \version     v0.0
 * \ingroup     envlint
******************************************************************************/

#include "envlint.h"

static const struct option long_options[] = {
        {"config", no_argument, NULL, 'c'},
        {"help", no_argument, NULL,'h'},
        {"ignore-all-failure", no_argument, NULL, ENVLINT_GETOPT_INDEX_IGNORE_ALL_FAILURE},
        {"version", no_argument, NULL, 'v'}
};

/*****************************************************************************/
/** 
 * \author      Weilun Fong
 * \date        2021/04/22
 * \brief       main function
 * \param[in]   argc: number of input arguments
 * \param[in]   argv: content of arguments
 * \return      0: success, -1: failure
 * \ingroup     envlint
 * \remarks     
******************************************************************************/
int main(int argc, char* argv[]) {

    bool config_parameter_flag = false;
    bool ingore_failure_flag = false;
    char *content = NULL;
    char *config = NULL;
    int  opt = 0;
    int  ret = 0;

    cJSON *root    = NULL;
    cJSON *item    = NULL;

    if (argc == 1) {
        printf("You must specify \'-c\' or \'--config\' option.\nTry \'envlint --help\' for more information.\r\n");
        return 0;
    }

    /**
     * \brief obtain parameters
     */
    opterr  = 1;   /* enable getopt error output */
    while ((opt = getopt_long(argc, argv, "c:hv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                config_parameter_flag = true;
                config = malloc(strlen(optarg) + 1);
                snprintf(config, strlen(optarg) + 1, "%s", optarg);
                break;
            case 'h':
                envlint_usage();
                goto EXIT;
            case 'v':
                envlint_version();
                goto EXIT;
            case ENVLINT_GETOPT_INDEX_IGNORE_ALL_FAILURE:
                ingore_failure_flag = true;
            default: break;
        }
    }

    if (!config_parameter_flag) {
        fputs("Please specify a config file via \'-c\' or \'--config\' option!\r\n", stderr);
        ret = -1;
        goto EXIT;
    }

    content = envlint_get_json_file_content(config);       /* for Address Sanitizer */
    root = cJSON_Parse(content);
    if (!root) {
        fputs("Failed to obtain JSON object.\r\n", stderr);
        goto EXIT;
    }

    /* read item list */
    item = cJSON_GetArrayItem(cJSON_GetObjectItem(root, ENVLINT_JSON_KEY_ITEMS), 0);
    if (!item) {
        fputs("Invalid config file format\r\n", stderr);
        ret = -1;
        goto EXIT;
    }

    /* start to check */
    if (envlint_check_items(item, ingore_failure_flag)) {
        ret = -1;
        goto EXIT;
    }

EXIT:
    if (config) {
        free(config);
    }

    if (content) {
        free(content);
    }

    if (root) {
        cJSON_Delete(root);
    }

    return ret;
}

/*****************************************************************************/
/** 
 * \author      Weilun Fong
 * \date        2021/04/22
 * \brief       check features of item
 * \param[in]   *feature: feature unit
 * \param[in]   ignore_failure: ignore check failure or not
 * \return      0: success, -1: failure
 * \ingroup     envlint
 * \remarks     
******************************************************************************/
int envlint_check_featrues(cJSON *feature, bool ignore_failure)
{
    cJSON *item = NULL;
    char cmd[ENVLINT_BUFFERSIZE];

    if (!feature) {
        fputs("Invalid JSON data\r\n", stderr);
        return -1;
    }

    if (!strcmp(cJSON_GetObjectItem(feature, ENVLINT_JSON_KEY_CHECK)->valuestring, "yes")) {
        item = cJSON_GetArrayItem(cJSON_GetObjectItem(feature, ENVLINT_JSON_KEY_ITEMS), 0);
        if (!item) {
            fputs("Invalid config file format", stderr);
            return -1;
        }

        while(item) {
            printf(" - Checking for %s ... ", cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_NAME)->valuestring);
            memset(cmd, 0x00, ENVLINT_BUFFERSIZE);
            snprintf(cmd, ENVLINT_BUFFERSIZE, "%s > /dev/null 2>&1", cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_COMMAND)->valuestring);
            if (system(cmd)) {
                printf("\033[1m\033[31mno \033[0m\r\n");
                if (!ignore_failure) {
                    if (!strcmp(cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_FORCE)->valuestring, "yes")) {
                        fputs("Abort!\r\n", stderr);
                        return -1;
                    }
                }
            }
            else {
                printf("yes\r\n");
            }
            item = item->next;
        }
    }

    return 0;
}

/*****************************************************************************/
/** 
 * \author      Weilun Fong
 * \date        2021/04/22
 * \brief       check items
 * \param[in]   *item: item list
 * \param[in]   ignore_failure: ignore check failure or not
 * \return      0: success, -1: failure
 * \ingroup     envlint
 * \remarks     
******************************************************************************/
int envlint_check_items(cJSON *item, bool ignore_failure) {

    char buffer[ENVLINT_BUFFERSIZE];
    FILE *stream = NULL;

    if (!item) {
        fputs("Null cJSON object ...\r\n", stderr);
        return -1;
    }

    while(item) {
        printf("Checking for %s ... ", cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_NAME)->valuestring);
        memset(buffer, 0x00, ENVLINT_BUFFERSIZE);
        snprintf(buffer, ENVLINT_BUFFERSIZE, "%s > /dev/null 2>&1", cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_COMMAND)->valuestring);
        if (system(buffer)) {
            printf("\033[1m\033[31mno \033[0m\r\n");
            if (!ignore_failure) {
                if (!strcmp(cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_FORCE)->valuestring, "yes")) { 
                    fputs("Abort!\r\n", stderr);
                    return -1;
                }
            }
        }
        else {
            if (!strcmp(cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_RETURN)->valuestring, "null")) {
                printf("yes\r\n");
            }
            else {
                stream = popen(cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_RETURN)->valuestring, "r");
                memset(buffer, 0x00, ENVLINT_BUFFERSIZE);
                fgets(buffer, sizeof(buffer), stream);
                printf("%s", buffer);
                pclose(stream);
            }

            /* check feature */
            if (envlint_check_featrues(cJSON_GetObjectItem(item, ENVLINT_JSON_KEY_FEATURE), ignore_failure)) {
                return -1;
            }
        }
        item = item->next;
    };

    return 0;
}

/*****************************************************************************/
/** 
 * \author      Weilun Fong
 * \date        2021/04/22
 * \brief       obtain json content from specified file via file path
 * \param[in]   *path: path string of target file
 * \return      content (if return NULL, it means failure)
 * \ingroup     envlint
 * \remarks     
******************************************************************************/
char* envlint_get_json_file_content(char *path) {

    char *dat = NULL;
    FILE *fp  = NULL;
    long len  = 0;

    fp = fopen(path, "rb");
    if (!fp) {
        fputs("Can not find specified file.\r\n", stderr);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    dat = malloc(len + 1);
    fseek(fp, 0, SEEK_SET);
    fread(dat, 1, len, fp);
    dat[len] = '\0';

    fclose(fp);
    return dat;
}

/*****************************************************************************/
/** 
 * \author      Weilun Fong
 * \date        2021/04/23
 * \brief       print usage
 * \param[in]   none
 * \return      0
 * \ingroup     envlint
 * \remarks     
******************************************************************************/
int envlint_usage(void) {

    printf("Environment Lint - Check your runtime environment\r\n\
%s\r\n\
Usage: envlint -c config\r\n\
\r\n\
Functional options:\r\n\
 -c, --config=CONF           Specify config file(.json).\r\n\
 --ignore-all-failure        Ignore all failure, even if value of <force> is \"yes\" \r\n\
                             in config file. \r\n\
\r\n\
Other options:\r\n\
 -h, --help                  Show usage, then exit.\r\n\
 -v, --version               Show version information, then exit.\r\n\
\r\n\
Originally written by Weilun Fong<wlf@zhishan-iot.tk>\r\n", CJSONEMBED);

    return 0;
}

/*****************************************************************************/
/** 
 * \author      Weilun Fong
 * \date        2021/04/23
 * \brief       print version information
 * \param[in]   none
 * \return      0
 * \ingroup     envlint
 * \remarks     
******************************************************************************/
int envlint_version(void) {

    printf("envlint v%d.%d.%d\r\n\
build with cJSON %d.%d.%d\r\n", \
            ENVLINT_MAJOR_VERSION, \
            ENVLINT_MINOR_VERSION, \
            ENVLINT_PATCH_VERSION, \
	    CJSON_VERSION_MAJOR, \
	    CJSON_VERSION_MINOR, \
	    CJSON_VERSION_PATCH);
    return 0;
}
