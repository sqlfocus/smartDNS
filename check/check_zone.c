#include <stdio.h>
#include "check_main.h"
#include "util_glb.h"
#include "log_glb.h"
#include "zone.h"

static void setup(void)
{
    log_init();
}

static void teardown(void)
{
    ;           /* do nothing */
}

START_TEST (test_get_token_handler)
{
    /* 仅仅为了测试, 使用GCC的拓展, 内部定义 */
    int test_func_p(void *glb_vars, char *val)
    {
        return RET_ERR;
    }
    int an_test_func_p(void *glb_vars, char *val)
    {
        return RET_ERR;
    }
    CFG_TYPE tmp_test_arr[] = {
        {"test", test_func_p},
        {"*", an_test_func_p},
        {NULL, (token_handler)NULL},
    };
    token_handler tmp_hdler;


    /* right thing */
    tmp_hdler = get_token_handler(tmp_test_arr, "test");
    ck_assert_int_eq((tmp_hdler - test_func_p), 0);

    tmp_hdler = get_token_handler(tmp_test_arr, "other");
    ck_assert_int_eq((tmp_hdler - an_test_func_p), 0);

    /* NOT right thing */
    tmp_hdler = get_token_handler(tmp_test_arr, "");
    ck_assert_int_eq(tmp_hdler, 0);
}
END_TEST

START_TEST (test_get_a_token)
{
    /* snprintf()按照指定长度形成字符串, 并自动在结尾处插入'\0'字符,
     * <NOTE>指定的长度包括最后'\0' */
    /* 由token长度和LINE长度界定的边界不需要在此测试, 由代码中调用
     * 此函数的逻辑保证 */
    char tmp_line[LINE_LEN_MAX] = {0};
    char *tmp_token;
    int tmp_len;
    int tmp_ret;

    /*** right thing ***/
    /* keyword */
    snprintf(tmp_line, LINE_LEN_MAX, "hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token-tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);


    /* 空格 + keyword */
    snprintf(tmp_line, LINE_LEN_MAX, " hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line - 1), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);


    /* keyword + 空格 */
    snprintf(tmp_line, LINE_LEN_MAX, "hello ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* keyword + ; */
    snprintf(tmp_line, LINE_LEN_MAX, "hello;");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL);
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* keyword + { */
    snprintf(tmp_line, LINE_LEN_MAX, "hello {");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL);
    ck_assert_int_eq(tmp_ret, RET_BRACE);

    /* \t + } + ; + 空格 */
    snprintf(tmp_line, LINE_LEN_MAX, "\t}; ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL); 
    ck_assert_int_eq(tmp_ret, RET_BRACE);

    /* keyword + ( */
    snprintf(tmp_line, LINE_LEN_MAX, "hello (");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hello"));
    ck_assert_int_eq((tmp_token - tmp_line), 0);
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL);
    ck_assert_int_eq(tmp_ret, RET_BRACE);

    /* \t + } + ; + 空格 */
    snprintf(tmp_line, LINE_LEN_MAX, "\t); ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL); 
    ck_assert_int_eq(tmp_ret, RET_BRACE);

    /* \" (magic 1 present ") */
    snprintf(tmp_line, LINE_LEN_MAX, "\"keys.conf\"");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("keys.conf"));
    ck_assert_int_eq((tmp_token - tmp_line - 1), 0);
    ck_assert_int_eq(tmp_ret, RET_DQUOTE);


    /* ; */
    snprintf(tmp_line, LINE_LEN_MAX, " ; hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* // */
    snprintf(tmp_line, LINE_LEN_MAX, "  // hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* # */
    snprintf(tmp_line, LINE_LEN_MAX, "\t # hello world");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL); 
    ck_assert_int_eq(tmp_ret, RET_COMMENT);

    /* '/\*' */
    snprintf(tmp_line, LINE_LEN_MAX, "/* hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL); 
    ck_assert_int_eq(tmp_ret, RET_MULTI_COMMENT);

    /* '*\/' */
    snprintf(tmp_line, LINE_LEN_MAX, "hi  */ hello");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, strlen("hi"));
    ck_assert_int_eq((tmp_token - tmp_line), 0); 
    ck_assert_int_eq(tmp_ret, RET_OK);
    tmp_ret = get_a_token(tmp_token + tmp_len, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL); 
    ck_assert_int_eq(tmp_ret, RET_MULTI_COMMENT);

    /* 换行符 */
    snprintf(tmp_line, LINE_LEN_MAX, " \n ");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, NULL); 
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* 空串 */
    tmp_line[0] = 0;
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_len, 0);
    ck_assert_int_eq(tmp_token, 0); 
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* NOT right thing */

    /* ' */
    snprintf(tmp_line, LINE_LEN_MAX, "'keys.conf'");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);
    snprintf(tmp_line, LINE_LEN_MAX, "\'keys.conf\'");
    tmp_ret = get_a_token(tmp_line, &tmp_token, &tmp_len);
    ck_assert_int_eq(tmp_ret, RET_ERR);
}
END_TEST

START_TEST (test_get_a_token_str)
{
    char tmp_line[LINE_LEN_MAX];
    char token[TOKEN_NAME_LEN_MAX];
    char *tmp_ch_p;
    int tmp_ret;

    /* keyword */
    snprintf(tmp_line, sizeof(tmp_line), "hello");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(token, "hello");
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* "keyword" */
    snprintf(tmp_line, sizeof(tmp_line), "\"hello world\"");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(token, "hello world");
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* 空格 + keyword */
    snprintf(tmp_line, sizeof(tmp_line), " hello");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(token, "hello");

    /* keyword + 空格 */
    snprintf(tmp_line, sizeof(tmp_line), "hello ");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_OK);
    ck_assert_str_eq(token, "hello");
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* ; */
    snprintf(tmp_line, sizeof(tmp_line), ";");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* # */
    snprintf(tmp_line, sizeof(tmp_line), "#");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* } */
    snprintf(tmp_line, sizeof(tmp_line), "}");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* { */
    snprintf(tmp_line, sizeof(tmp_line), "{");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* ( */
    snprintf(tmp_line, sizeof(tmp_line), "(");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* ) */
    snprintf(tmp_line, sizeof(tmp_line), ")");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* /\* */
    snprintf(tmp_line, sizeof(tmp_line), "/*");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* *\/ */
    snprintf(tmp_line, sizeof(tmp_line), "*/");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* // */
    snprintf(tmp_line, sizeof(tmp_line), "//");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* \n */
    snprintf(tmp_line, sizeof(tmp_line), "\n");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* \r */
    snprintf(tmp_line, sizeof(tmp_line), "\r");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* \r\n */
    snprintf(tmp_line, sizeof(tmp_line), "\r\n");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ENDLINE);

    /* ' */
    snprintf(tmp_line, sizeof(tmp_line), "'");
    tmp_ch_p = tmp_line;
    tmp_ret = get_a_token_str(&tmp_ch_p, token, NULL);
    ck_assert_int_eq(tmp_ret, RET_ERR);
}
END_TEST

START_TEST (test_get_arr_index_by_type)
{
    int tmp_ret;

    tmp_ret = get_arr_index_by_type(TYPE_A);
    ck_assert_int_le(tmp_ret, RR_TYPE_MAX);

    tmp_ret = get_arr_index_by_type(TYPE_MAX);
    ck_assert_int_eq(tmp_ret, RR_TYPE_MAX);
}
END_TEST

Suite * cfg_glb_suite(void)                                               
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("zone/zone.c");

    tc_core = tcase_create("get_token_handler");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_token_handler);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("get_a_token");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_a_token);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("get_a_token_str");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_a_token_str);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("get_arr_index_by_type");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_arr_index_by_type);
    suite_add_tcase(s, tc_core);

    return s;
}

REGISTER_SUITE(cfg_glb_suite);
