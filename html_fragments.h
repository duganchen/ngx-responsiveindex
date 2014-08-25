#ifndef BOOTSTRAPINDEX_HTML_FRAGMENTS_H
#define BOOTSTRAPINDEX_HTML_FRAGMENTS_H

#include <ngx_config.h>
#include <ngx_core.h>

#define DOCTYPE "<!DOCTYPE html>"
#define HTML_PRE_LANG "<html lang=\""
#define TAG_END "\">"
#define HTML_POST_LANG TAG_END
#define HEAD_START "<head>"
#define CHARSET "<meta charset=\"utf-8\">"
#define VIEWPORT "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
#define LINK_PRE_HREF "<link rel=\"stylesheet\" href=\""
#define LINK_POST_HREF TAG_END
#define TITLE_START "<title>"
#define TITLE_END "</title>"
#define STYLE_START "<style>"
#define STYLE_END "</style>"
#define HEAD_END "</head>"
#define BODY_START "<body>"
#define BODY_END "</body>"
#define DIV_START "<div class=\""
#define DIV_END "</div>"
#define CONTAINER "container-fluid"
#define ROW "row"
#define COL "col-md-12"
#define H1_START "<h1>"
#define H1_END "</h1>"
#define TABLE "table-responsive hidden-xs hidden-sm"
#define TABLE_START "<table class=\"table table-striped table-condensed\">"
#define THEAD_START "<thead>"
#define TR_START "<tr>"
#define TH_START "<th>"
#define TH_END "</th>"
#define TR_END "</tr>"
#define THEAD_END "</thead>"
#define TBODY_START "<tbody>"
#define TD_START "<td>"
#define TD_END "</td>"
#define TBODY_END "</tbody>"
#define TABLE_END "</table>"
#define UL_START "<ul class=\"list-group visible-xs visible-sm\">"
#define LI_START "<li class=\"list-group-item\">"
#define LI_END "</li>"
#define UL_END "</ul>"
#define HTML_END "</html>"

#define A_PRE_HREF "<a href=\""
#define A_END "</a>"

#define EN "en"
#define BOOTSTRAPCDN "//maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap.min.css"

/* Blocks of the HTML to be written in order, with runtime data in between. */

static u_char to_lang[] =
DOCTYPE CRLF
HTML_PRE_LANG;

static u_char to_stylesheet[] =
HTML_POST_LANG CRLF
HEAD_START CRLF
CHARSET CRLF
VIEWPORT CRLF
LINK_PRE_HREF
;

static u_char to_title[] =
TAG_END CRLF
STYLE_START CRLF
"body {" CRLF
"    word-wrap: break-word;" CRLF
"}" CRLF
"a {" CRLF
"    display: block;" CRLF
"    width: 100%;" CRLF
"    height: 100%;" CRLF
"}" CRLF
STYLE_END CRLF
TITLE_START
;


static u_char to_h1[] =
TITLE_END CRLF
HEAD_END CRLF
BODY_START CRLF
DIV_START CONTAINER TAG_END CRLF
DIV_START ROW TAG_END CRLF
DIV_START COL TAG_END CRLF
H1_START
;

static u_char to_table_body[] =
H1_END CRLF
DIV_START TABLE TAG_END CRLF
TABLE_START CRLF
THEAD_START CRLF
TR_START
TH_START "File Name" TH_END
TH_START "Date" TH_END
TH_START "File Size" TH_END
TR_END CRLF
THEAD_END CRLF
TBODY_START CRLF
TR_START
TD_START A_PRE_HREF ".." TAG_END ".." A_END TD_END
TD_START TD_END
TD_START TD_END
TR_END CRLF
;

static u_char to_td_href[] =
TR_START
TD_START
A_PRE_HREF
;

static u_char to_td_date[] =
A_END
TD_END
TD_START
;

static u_char to_td_size[] =
TD_END
TD_START
;

static u_char end_row[] =
TD_END
TR_END CRLF
;

static u_char to_list[] =
TBODY_END CRLF
TABLE_END CRLF
DIV_END CRLF
UL_START CRLF
;

static u_char to_item_href[] =
LI_START
A_PRE_HREF
;

static u_char to_item_end[] =
A_END
LI_END CRLF
;

static u_char to_html_end[] =
UL_END CRLF
DIV_END CRLF
DIV_END CRLF
DIV_END CRLF
BODY_END CRLF
HTML_END CRLF
;

#endif

