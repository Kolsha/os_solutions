#pragma once
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define DEFAULT_PORT "80"
struct parsed_url 
{
    char *uri;					/* mandatory */
    char *scheme;               /* mandatory */
    char *host;                 /* mandatory */
    char *port;                 /* optional */
    char *path;                 /* optional */
    char *query;                /* optional */
    char *fragment;             /* optional */
};


void parsed_url_free(struct parsed_url *purl)
{
    if ( NULL != purl )
    {
        if ( NULL != purl->scheme ) free(purl->scheme);
        if ( NULL != purl->host ) free(purl->host);
        if ( NULL != purl->port ) free(purl->port);
        if ( NULL != purl->path )  free(purl->path);
        if ( NULL != purl->query ) free(purl->query);
        if ( NULL != purl->fragment ) free(purl->fragment);

        free(purl);
    }
}

int is_scheme_char(int c)
{
    return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}

/*
    Parses a specified URL and returns the structure named 'parsed_url'
    Implented according to:
    RFC 1738 - http://www.ietf.org/rfc/rfc1738.txt
    RFC 3986 -  http://www.ietf.org/rfc/rfc3986.txt
*/
struct parsed_url *parse_url(const char *url)
{

    /* Define variable */
    struct parsed_url *purl;
    const char *tmpstr;
    const char *curstr;
    int len;
    int i;
    int bracket_flag;

    /* Allocate the parsed url storage */
    purl = (struct parsed_url*)malloc(sizeof(struct parsed_url));
    if ( NULL == purl )
    {
        return NULL;
    }
    purl->scheme = NULL;
    purl->host = NULL;
    purl->port = NULL;
    purl->path = NULL;
    purl->query = NULL;
    purl->fragment = NULL;

    curstr = url;

    /*
     * <scheme>:<scheme-specific-part>
     * <scheme> := [a-z\+\-\.]+
     *             upper case = lower case for resiliency
     */
    /* Read scheme */
    tmpstr = strchr(curstr, ':');
    if ( NULL == tmpstr )
    {
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);

        return NULL;
    }

    /* Get the scheme length */
    len = tmpstr - curstr;

    /* Check restrictions */
    for ( i = 0; i < len; i++ )
    {
        if (is_scheme_char(curstr[i]) == 0)
        {
            /* Invalid format */
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
    }
    /* Copy the scheme to the storage */
    purl->scheme = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->scheme )
    {
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);

        return NULL;
    }

    (void)strncpy(purl->scheme, curstr, len);
    purl->scheme[len] = '\0';

    /* Make the character to lower if it is upper case. */
    for ( i = 0; i < len; i++ )
    {
        purl->scheme[i] = tolower(purl->scheme[i]);
    }

    /* Skip ':' */
    tmpstr++;
    curstr = tmpstr;

    /*
     * //<user>:<password>@<host>:<port>/<url-path>
     * Any ":", "@" and "/" must be encoded.
     */
    /* Eat "//" */
    for ( i = 0; i < 2; i++ )
    {
        if ( '/' != *curstr )
        {
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        curstr++;
    }


    if ( '[' == *curstr )
    {
        bracket_flag = 1;
    }
    else
    {
        bracket_flag = 0;
    }
    /* Proceed on by delimiters with reading host */
    tmpstr = curstr;
    while ( '\0' != *tmpstr ) {
        if ( bracket_flag && ']' == *tmpstr )
        {
            /* End of IPv6 address. */
            tmpstr++;
            break;
        }
        else if ( !bracket_flag && (':' == *tmpstr || '/' == *tmpstr) )
        {
            /* Port number is specified. */
            break;
        }
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->host = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->host || len <= 0 )
    {
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    (void)strncpy(purl->host, curstr, len);
    purl->host[len] = '\0';
    curstr = tmpstr;

    /* Is port number specified? */
    if ( ':' == *curstr )
    {
        curstr++;
        /* Read port number */
        tmpstr = curstr;
        while ( '\0' != *tmpstr && '/' != *tmpstr )
        {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->port = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->port )
        {
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->port, curstr, len);
        purl->port[len] = '\0';
        curstr = tmpstr;
    }
    else
    {
        len = strlen(DEFAULT_PORT);
        purl->port = (char*)malloc(sizeof(char) * (len + 1));
        (void)strncpy(purl->port, DEFAULT_PORT, len);
    }


    /* Set uri */
    purl->uri = (char*)url;

    /* End of the string */
    if ( '\0' == *curstr )
    {
        return purl;
    }

    /* Skip '/' */
    if ( '/' != *curstr )
    {
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    curstr++;

    /* Parse path */
    tmpstr = curstr;
    while ( '\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr )
    {
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->path = (char*)malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->path )
    {
        parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
        return NULL;
    }
    (void)strncpy(purl->path, curstr, len);
    purl->path[len] = '\0';
    curstr = tmpstr;

    /* Is query specified? */
    if ( '?' == *curstr )
    {
        /* Skip '?' */
        curstr++;
        /* Read query */
        tmpstr = curstr;
        while ( '\0' != *tmpstr && '#' != *tmpstr )
        {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->query = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->query )
        {
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->query, curstr, len);
        purl->query[len] = '\0';
        curstr = tmpstr;
    }

    /* Is fragment specified? */
    if ( '#' == *curstr )
    {
        /* Skip '#' */
        curstr++;
        /* Read fragment */
        tmpstr = curstr;
        while ( '\0' != *tmpstr )
        {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->fragment = (char*)malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->fragment )
        {
            parsed_url_free(purl); fprintf(stderr, "Error on line %d (%s)\n", __LINE__, __FILE__);
            return NULL;
        }
        (void)strncpy(purl->fragment, curstr, len);
        purl->fragment[len] = '\0';
        curstr = tmpstr;
    }
    return purl;
}
