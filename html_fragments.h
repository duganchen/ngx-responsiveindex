#ifndef RESPONSIVEINDEX_HTML_FRAGMENTS_H
#define RESPONSIVEINDEX_HTML_FRAGMENTS_H

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
#define DIV_START(class) "<div class=\"" class TAG_END
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


static ngx_str_t en = ngx_string("en");
static ngx_str_t bootstrapcdn = ngx_string("//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css");


/* Blocks of the HTML to be written in order, with runtime data in between. */

static ngx_str_t tag_end = ngx_string(TAG_END);


static ngx_str_t to_lang = ngx_string(
	DOCTYPE "\n"
	HTML_PRE_LANG
);


static ngx_str_t to_stylesheet = ngx_string(
	HTML_POST_LANG "\n"
	HEAD_START "\n"
	CHARSET "\n"
	VIEWPORT "\n"
	LINK_PRE_HREF
);


static ngx_str_t to_title = ngx_string(
	TAG_END "\n"
	STYLE_START "\n"
	"body {" "\n"
	"    word-wrap: break-word;" "\n"
	"}" "\n"
	"a {" "\n"
	"    display: block;" "\n"
	"    width: 100%;" "\n"
	"    height: 100%;" "\n"
	"}" "\n"
	STYLE_END "\n"
	TITLE_START
);


static ngx_str_t to_h1 = ngx_string(
	TITLE_END "\n"
	HEAD_END "\n"
	BODY_START "\n"
	DIV_START(CONTAINER) "\n"
	DIV_START(ROW) "\n"
	DIV_START(COL) "\n"
	H1_START
);


static ngx_str_t to_table_body = ngx_string(
	H1_END "\n"
	DIV_START(TABLE) "\n"
	TABLE_START "\n"
	THEAD_START "\n"
	TR_START
	TH_START "File Name" TH_END
	TH_START "Date" TH_END
	TH_START "File Size" TH_END
	TR_END "\n"
	THEAD_END "\n"
	TBODY_START "\n"
	TR_START
	TD_START A_PRE_HREF ".." TAG_END ".." A_END TD_END
	TD_START TD_END
	TD_START TD_END
	TR_END "\n"
);


static ngx_str_t to_td_href = ngx_string(
	TR_START
	TD_START
	A_PRE_HREF
);


static ngx_str_t to_td_date = ngx_string(
	A_END
	TD_END
	TD_START
);


static ngx_str_t to_td_size = ngx_string(
	TD_END
	TD_START
);


static ngx_str_t end_row = ngx_string(
	TD_END
	TR_END "\n");


static ngx_str_t to_list = ngx_string(
	TBODY_END "\n"
	TABLE_END "\n"
	DIV_END "\n"
	UL_START "\n"
	LI_START
	A_PRE_HREF
	".."
	TAG_END
	".."
	A_END
	LI_END "\n"
);


static ngx_str_t to_item_href = ngx_string(
	LI_START
	A_PRE_HREF
);


static ngx_str_t to_item_end = ngx_string(
	A_END
	LI_END "\n");


static ngx_str_t to_html_end = ngx_string(
	UL_END "\n"
	DIV_END "\n"
	DIV_END "\n"
	DIV_END "\n"
	BODY_END "\n"
	HTML_END "\n"
);


#endif

