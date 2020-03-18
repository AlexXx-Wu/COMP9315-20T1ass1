/*
 * src/tutorial/complex.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/
//check
//去空格
//show
#include <stdio.h>
#include "postgres.h"
#include <stdbool.h>
#include <string.h>
#include <regex.h>
#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */
#include "access/hash.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

static int check_input(const char *match_string){
    const char *pattern = "((^[A-Z])(([‘|-])|([A-Za-z]))+(([ ])?([A-Z])(([‘|-])|([A-Za-z]))+)*,([ ])?([A-Z])(([‘|-])|([A-Za-z]))+(([ ])?([A-Z])(([‘|-])|([A-Za-z]))+)*)$";
    bool result = 0;
    regex_t regex;
    int regexInit;
    regexInit = regcomp(&regex, pattern, REG_EXTENDED);
    if (regexInit == 0){
        int match_result = regexec(&regex, match_string, 0, NULL, 0);
        if(match_result == 0){
            result = 1;
        }
    }
    regfree(&regex);
    return result;
            //return Regex.IsMatch(str, "((^[A-Z])(([‘|-])|([A-Za-z]))+(([ ])?([A-Z])(([‘|-])|([A-Za-z]))+)*,([ ])?([A-Z])(([‘|-])|([A-Za-z]))+(([ ])?([A-Z])(([‘|-])|([A-Za-z]))+)*)$");
}



typedef struct Person
{
    int length;
    //variable
    char pname[1];
}PersonName;

int pname_compare(PersonName *a, PersonName *b);

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/


PG_FUNCTION_INFO_V1(pname_in);

Datum
pname_in(PG_FUNCTION_ARGS)
{
    // insert into students (name) values("Smith,John")
    char	   *str = PG_GETARG_CSTRING(0);


    if (strlen(str) < 2)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                        errmsg("invalid input syntax for type %s: \"%s\"",
                               "pname", str)));
    if (check_input(str) == false){
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                        errmsg("invalid input syntax for type %s: \"%s\"",
                               "pname", str)));
    }
    //还需添加
    //check ,Aa 正则表达式
    //首字母必须大写
    //(^[A~Z](a~z)+(([ -])?([A~Z](a~z)+
    PersonName    *result;
    int length = strlen(str) + 1;

    result = (PersonName *) palloc(VARHDRSZ + length);
    SET_VARSIZE(result, VARHDRSZ + length);
    //赋值result
    snprintf(result->pname, length, "%s", str);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(pname_out);

Datum
pname_out(PG_FUNCTION_ARGS)
{
    PersonName    *personName = (PersonName *) PG_GETARG_POINTER(0);
    char	   *result;

    result = psprintf("(%s)", personName->pname);
    PG_RETURN_CSTRING(result);
}

/*****************************************************************************
 * Binary Input/Output functions
 *
 * These are optional.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(pname_recv);

Datum
pname_recv(PG_FUNCTION_ARGS)
{
    StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
    PersonName    *result;

    const char * person_name = pq_getmsgstring(buf);
    int length = strlen(person_name) + 1;

    result = (PersonName *) palloc(VARHDRSZ + length);
    SET_VARSIZE(result, VARHDRSZ + length);
    //赋值result
    snprintf(result->pname, length, "%s", person_name);
    PG_RETURN_POINTER(result);
    /***
	result = (PersonName *) palloc(sizeof(PersonName));
	result->x = pq_getmsgfloat8(buf);
	result->y = pq_getmsgfloat8(buf);
	PG_RETURN_POINTER(result);
     ***/
}

PG_FUNCTION_INFO_V1(pname_send);

Datum
pname_send(PG_FUNCTION_ARGS)
{
    PersonName    *personName = (PersonName *) PG_GETARG_POINTER(0);
    StringInfoData buf;
    pq_begintypsend(&buf);
    pq_sendstring(&buf, personName->pname);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * New Operators
 *
 * A practical Complex datatype would provide much more than this, of course.
 *****************************************************************************/
PG_FUNCTION_INFO_V1(pname_compare);

Datum
pname_compare(PG_FUNCTION_ARGS)
{
    int a_comma = 0, b_comma = 0;
    for (int i = 0; i < strlen(a->pname); i++){
        if (a->pname[i] == ','){
            a_comma = i;
            break;
        }
    }
    /***
     * second
     * char * a_given_name = strchr(a->pname, ',');
     * a_comma = strlen(a->pname) - strlen(a_given_name);
     */
    char * a_given_name = strchr(a->pname, ',');
    char * b_given_name = strchr(b->pname, ',');
    int result;
    b_comma = strlen(b->pname) - strlen(b_given_name);

    a->pname[a_comma] = '\0';
    b->pname[b_comma] = '\0';
    result = strcmp(a->pname, b->pname);

    a->pname[a_comma] = ',';
    b->pname[b_comma] = ',';
    if (result == 0){
        if (a->pname[a_comma + 1] == ' '){
            a_given_name++;
        }
        if (b->pname[a_comma + 1] == ' '){
            b_given_name++;
        }
        result = strcmp(a_given_name, b_given_name);
    }
    return result;
}

PG_FUNCTION_INFO_V1(pname_lt);

Datum
pname_lt(PG_FUNCTION_ARGS)
{
    PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
    PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(pname_compare(a, b) < 0);
}

PG_FUNCTION_INFO_V1(pname_le);

Datum
pname_le(PG_FUNCTION_ARGS)
{
    PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
    PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(pname_compare(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(pname_equal);

Datum
pname_equal(PG_FUNCTION_ARGS)
{
    PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
    PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(pname_compare(a, b) == 0);
}

PG_FUNCTION_INFO_V1(pname_ge);

Datum
pname_ge(PG_FUNCTION_ARGS)
{
    PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
    PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(pname_compare(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(pname_gt);

Datum
pname_gt(PG_FUNCTION_ARGS)
{
    PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
    PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(pname_compare(a, b) > 0);
}
/***
PG_FUNCTION_INFO_V1(complex_abs_cmp);

Datum
complex_abs_cmp(PG_FUNCTION_ARGS)
{
	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(complex_abs_cmp_internal(a, b));
}
***/

PG_FUNCTION_INFO_V1(family);

Datum
family(PG_FUNCTION_ARGS)
{
    PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
    int a_family = 0;
    char result;
    for (int i = 0; i < strlen(a->pname); i++){
        if (a->pname[i] == ','){
            a_family = i;
            break;
        }
    }
    a->pname[a_family] = '\0';
    result = psprintf("%s", a->pname);
    a->pname[a_family] = ',';
    PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(given);

Datum
given(PG_FUNCTION_ARGS)
{
    PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
    char * a_given_name;
    char result;
    a_given_name = strchr(a->pname, ',') + 1;
    if (*(a_given_name) == ' '){
        a_given_name++;
    }
    result = psprintf('%s', a_given_name);

    PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(pname_hash);

Datum
pname_hash(PG_FUNCTION_ARGS)
{
    PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
    /***char * a_given_name;
    char result;
    a_given_name = strchr(a->pname, ',') + 1;
    if (*(a_given_name) == ' '){
        a_given_name++;
    }***/
    // hash any
    int hash_code = DatumGetUInt32(hash_any((const unsigned char *) a->pname, strlen(a->pname)));;
    PG_RETURN_INT32(hash_code);
}

PG_FUNCTION_INFO_V1(show);

Datum
show(PG_FUNCTION_ARGS){
        PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
        int a_family = 0;
        char result;
        for (int i = 0; i < strlen(a->pname); i++){
            if (a->pname[i] == ','){
                a_family = i;
                break;
            }
        }
        a->pname[a_family] = '\0';
        result = psprintf("%s", a->pname);
        a->pname[a_family] = ',';
        char * a_given_name;
        char result2;
        a_given_name = strchr(a->pname, ',') + 1;
        if (*(a_given_name) == ' '){
            a_given_name++;
        }

        int given_space = strchr(a_given_name , ' ');
        a_given_name[given_space] = '\0';
        result2 = psprintf('%s', a_given_name);
        a_given_name[given_space] = ' ';

        PG_RETURN_TEXT_P(cstring_to_text(result + " " + result2));
}
