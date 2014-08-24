#define DOCTYPE "<DOCTYPE html>" CRLF
#define HTML_PRE_LANG "<html lang=\""
#define TAG_END "\">"
#define HTML_POST_LANG TAG_END CRLF
#define HEAD_START "<head>" CRLF
#define CHARSET "<meta charset=\"utf-8\">" CRLF
#define VIEWPORT "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">" CRLF
#define LINK_PRE_HREF "<link rel=\"stylesheet\" href=\""
#define LINK_POST_HREF TAG_END CRLF
#define TITLE_START "<title>"
#define TITLE_END "</title>" CRLF
#define STYLE_START "<style>" CRLF
#define STYLE_END "</style>" CRLF
#define HEAD_END "</head>" CRLF
#define BODY_START "<body>" CRLF
#define BODY_END "</body>" CRLF
#define DIV_START(class) "<div class=\"" #class "\">" CRLF
#define DIV_END "</div>" CRLF
#define CONTAINER "container-fluid"
#define ROW "row"
#define COL "col-md-12"
#define H1_START "<h1>"
#define H1_END "</h1>"
#define TABLE "table-responsive hidden-xs hidden-sm"
#define TABLE_START "<table class=\"table table-striped table-condensed\">" CRLF
#define THEAD_START "<thead>" CRLF
#define TR_START "<tr>" CRLF
#define TH(text) "<th>" #text "</th>"
#define TR_END "</tr>" CRLF
#define THEAD_END "</thead>" CRLF
#define TBODY_START "<tbody>" CRLF
#define TD_START "<td>"
#define TD_END "</td>"
#define TBODY_END "</tbody>" CRLF
#define TABLE_END "</table>" CRLF
#define UL_START "<ul class=\"list-group visible-xs visible-sm\">" CRLF
#define LI_START "<li class=\"list-group-item\">"
#define LI_END "</li>" CRLF
#define UL_END "</ul>" CRLF
#define HTML_END "</html>"

#define A_PRE_HREF "<a href=\""
#define A_END "</a>"

static u_char css[] =
"body {" CRLF
"    word-wrap: break-word;" CRLF
"}" CRLF
"a {" CRLF
"    display: block;" CRLF
"    width: 100%;" CRLF
"    height: 100%;" CRLF
"}" CRLF
;

/* Blocks of the HTML to be written in order, with runtime data in between. */

static u_char to_lang[] =
DOCTYPE
HTML_PRE_LANG;

static u_char to_stylesheet[] =
HTML_POST_LANG
HEAD_START
CHARSET
VIEWPORT
LINK_PRE_HREF
;


static u_char to_style[] =
TAG_END
STYLE_START
;

static u_char to_title[] =
STYLE_END
TITLE_START
;


static u_char to_h1[] =
TITLE_END
HEAD_END
BODY_START
DIV_START(CONTAINER)
DIV_START(ROW)
H1_START
;

static u_char to_table_body[] =
H1_END
DIV_START(TABLE)
TABLE_START
THEAD_START
TR_START
TH("File Name")
TH("Date")
TH("File Size")
TR_END
THEAD_END
TBODY_START
TR_START
TD_START A_PRE_HREF ".." TAG_END ".." A_END TD_END
TD_START TD_END
TD_START TD_END
TR_END
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
TR_END
;

static u_char to_list[] =
TBODY_END
TABLE_END
DIV_END
UL_START
;

static u_char to_list_href[] =
LI_START
A_PRE_HREF
;

static u_char to_item_end[] =
A_END
LI_END
;

static u_char to_html_end[] =
UL_END
DIV_END
DIV_END
DIV_END
BODY_END
HTML_END
;


