
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */



#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#include "html_fragments.h"

typedef struct {
	ngx_str_t	name;
	size_t		utf_len;
	size_t		escape;
	size_t		escape_html;

	unsigned	dir:1;

	time_t		mtime;
	off_t		size;
} ngx_http_bootstrapindex_entry_t;


typedef struct {
	ngx_flag_t	enable;
	ngx_flag_t	localtime;
	ngx_flag_t	exact_size;
} ngx_http_bootstrapindex_loc_conf_t;


#define NGX_HTTP_AUTOINDEX_PREALLOCATE	255

#define NGX_HTTP_AUTOINDEX_NAME_LEN	255


static int ngx_libc_cdecl ngx_http_bootstrapindex_cmp_entries(const void *one,
	const void *two);
static ngx_int_t ngx_http_bootstrapindex_error(ngx_http_request_t *r,
	ngx_dir_t *dir, ngx_str_t *name);
static ngx_int_t ngx_http_bootstrapindex_init(ngx_conf_t *cf);
static void *ngx_http_bootstrapindex_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_bootstrapindex_merge_loc_conf(ngx_conf_t *cf,
	void *parent, void *child);

static void ngx_http_bootstrapindex_cpy_uri(ngx_buf_t *, ngx_http_bootstrapindex_entry_t *);
static void ngx_http_bootstrapindex_cpy_name(ngx_buf_t *, ngx_http_bootstrapindex_entry_t *);
static void ngx_http_bootstrapindex_cpy_size(ngx_buf_t *, ngx_http_bootstrapindex_entry_t *,
	ngx_http_bootstrapindex_loc_conf_t  *);


static ngx_command_t  ngx_http_bootstrapindex_commands[] = {

	{
		ngx_string("bootstrapindex"),
		NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
		ngx_conf_set_flag_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_bootstrapindex_loc_conf_t, enable),
		NULL
	},

	{
		ngx_string("bootstrapindex_localtime"),
		NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
		ngx_conf_set_flag_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_bootstrapindex_loc_conf_t, localtime),
		NULL
	},

	{
		ngx_string("bootstrapindex_exact_size"),
		NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
		ngx_conf_set_flag_slot,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_bootstrapindex_loc_conf_t, exact_size),
		NULL
	},

	ngx_null_command
};


static ngx_http_module_t ngx_http_bootstrapindex_module_ctx = {
	NULL,										/* preconfiguration */
	ngx_http_bootstrapindex_init,				/* postconfiguration */

	NULL,										/* create main configuration */
	NULL,										/* init main configuration */

	NULL,										/* create server configuration */
	NULL,										/* merge server configuration */

	ngx_http_bootstrapindex_create_loc_conf,	/* create location configuration */
	ngx_http_bootstrapindex_merge_loc_conf		/* merge location configuration */
};


ngx_module_t ngx_http_bootstrapindex_module = {
	NGX_MODULE_V1,
	&ngx_http_bootstrapindex_module_ctx,	/* module context */
	ngx_http_bootstrapindex_commands,		/* module directives */
	NGX_HTTP_MODULE,						/* module type */
	NULL,									/* init master */
	NULL,									/* init module */
	NULL,									/* init process */
	NULL,									/* init thread */
	NULL,									/* exit thread */
	NULL,									/* exit process */
	NULL,									/* exit master */
	NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_bootstrapindex_handler(ngx_http_request_t *r)
{
	u_char						*last, *filename;
	size_t						len, escape_html, allocated, root;
	ngx_tm_t					tm;
	ngx_err_t					err;
	ngx_buf_t					*b;
	ngx_int_t					rc;
	ngx_str_t					path;
	ngx_dir_t					dir;
	ngx_uint_t					i, level, utf8;
	ngx_pool_t					*pool;
	ngx_time_t					*tp;
	ngx_chain_t					out;
	ngx_array_t					entries;
	ngx_http_bootstrapindex_entry_t	 *entry;
	ngx_http_bootstrapindex_loc_conf_t *alcf;

	static char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
							"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	if (r->uri.data[r->uri.len - 1] != '/') {
		return NGX_DECLINED;
	}

	if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
		return NGX_DECLINED;
	}

	alcf = ngx_http_get_module_loc_conf(r, ngx_http_bootstrapindex_module);

	if (!alcf->enable) {
		return NGX_DECLINED;
	}

	/* NGX_DIR_MASK_LEN is lesser than NGX_HTTP_AUTOINDEX_PREALLOCATE */

	last = ngx_http_map_uri_to_path(r, &path, &root,
									NGX_HTTP_AUTOINDEX_PREALLOCATE);
	if (last == NULL) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	allocated = path.len;
	path.len = last - path.data;
	if (path.len > 1) {
		path.len--;
	}
	path.data[path.len] = '\0';

	ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
				"http bootstrapindex: \"%s\"", path.data);

	if (ngx_open_dir(&path, &dir) == NGX_ERROR) {
		err = ngx_errno;

		if (err == NGX_ENOENT
			|| err == NGX_ENOTDIR
			|| err == NGX_ENAMETOOLONG)
		{
			level = NGX_LOG_ERR;
			rc = NGX_HTTP_NOT_FOUND;

		} else if (err == NGX_EACCES) {
			level = NGX_LOG_ERR;
			rc = NGX_HTTP_FORBIDDEN;

		} else {
			level = NGX_LOG_CRIT;
			rc = NGX_HTTP_INTERNAL_SERVER_ERROR;
		}

		ngx_log_error(level, r->connection->log, err,
					  ngx_open_dir_n " \"%s\" failed", path.data);

		return rc;
	}

#if (NGX_SUPPRESS_WARN)

	/* MSVC thinks 'entries' may be used without having been initialized */
	ngx_memzero(&entries, sizeof(ngx_array_t));

#endif

	/* TODO: pool should be temporary pool */
	pool = r->pool;

	if (ngx_array_init(&entries, pool, 40, sizeof(ngx_http_bootstrapindex_entry_t))
		!= NGX_OK)
	{
		return ngx_http_bootstrapindex_error(r, &dir, &path);
	}

	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_type_len = sizeof("text/html") - 1;
	ngx_str_set(&r->headers_out.content_type, "text/html");
	r->headers_out.content_type_lowcase = NULL;

	rc = ngx_http_send_header(r);

	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
		if (ngx_close_dir(&dir) == NGX_ERROR) {
			ngx_log_error(NGX_LOG_ALERT, r->connection->log, ngx_errno,
						ngx_close_dir_n " \"%V\" failed", &path);
		}

		return rc;
	}

	filename = path.data;
	filename[path.len] = '/';

	if (r->headers_out.charset.len == 5
		&& ngx_strncasecmp(r->headers_out.charset.data, (u_char *) "utf-8", 5)
		== 0)
	{
		utf8 = 1;

	} else {
		utf8 = 0;
	}

	for ( ;; ) {
		ngx_set_errno(0);

		if (ngx_read_dir(&dir) == NGX_ERROR) {
			err = ngx_errno;

			if (err != NGX_ENOMOREFILES) {
				ngx_log_error(NGX_LOG_CRIT, r->connection->log, err,
							  ngx_read_dir_n " \"%V\" failed", &path);
				return ngx_http_bootstrapindex_error(r, &dir, &path);
			}

			break;
		}

		ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
					"http bootstrapindex file: \"%s\"", ngx_de_name(&dir));

		len = ngx_de_namelen(&dir);

		if (ngx_de_name(&dir)[0] == '.') {
			continue;
		}

		if (!dir.valid_info) {

			/* 1 byte for '/' and 1 byte for terminating '\0' */

			if (path.len + 1 + len + 1 > allocated) {
				allocated = path.len + 1 + len + 1
									+NGX_HTTP_AUTOINDEX_PREALLOCATE;

				filename = ngx_pnalloc(pool, allocated);
				if (filename == NULL) {
					return ngx_http_bootstrapindex_error(r, &dir, &path);
				}

				last = ngx_cpystrn(filename, path.data, path.len + 1);
				*last++ = '/';
			}

			ngx_cpystrn(last, ngx_de_name(&dir), len + 1);

			if (ngx_de_info(filename, &dir) == NGX_FILE_ERROR) {
				err = ngx_errno;

				if (err != NGX_ENOENT && err != NGX_ELOOP) {
					ngx_log_error(NGX_LOG_CRIT, r->connection->log, err,
								ngx_de_info_n " \"%s\" failed", filename);

					if (err == NGX_EACCES) {
						continue;
					}

					return ngx_http_bootstrapindex_error(r, &dir, &path);
				}

				if (ngx_de_link_info(filename, &dir) == NGX_FILE_ERROR) {
					ngx_log_error(NGX_LOG_CRIT, r->connection->log, ngx_errno,
								ngx_de_link_info_n " \"%s\" failed",
								filename);
					return ngx_http_bootstrapindex_error(r, &dir, &path);
				}
			}
		}

		entry = ngx_array_push(&entries);
		if (entry == NULL) {
			return ngx_http_bootstrapindex_error(r, &dir, &path);
		}

		entry->name.len = len;

		entry->name.data = ngx_pnalloc(pool, len + 1);
		if (entry->name.data == NULL) {
			return ngx_http_bootstrapindex_error(r, &dir, &path);
		}

		ngx_cpystrn(entry->name.data, ngx_de_name(&dir), len + 1);

		entry->escape = 2 * ngx_escape_uri(NULL, ngx_de_name(&dir), len,
										NGX_ESCAPE_URI_COMPONENT);

		entry->escape_html = ngx_escape_html(NULL, entry->name.data,
											 entry->name.len);

		if (utf8) {
			entry->utf_len = ngx_utf8_length(entry->name.data, entry->name.len);
		} else {
			entry->utf_len = len;
		}

		entry->dir = ngx_de_is_dir(&dir);
		entry->mtime = ngx_de_mtime(&dir);
		entry->size = ngx_de_size(&dir);
	}

	if (ngx_close_dir(&dir) == NGX_ERROR) {
		ngx_log_error(NGX_LOG_ALERT, r->connection->log, ngx_errno,
					ngx_close_dir_n " \"%V\" failed", &path);
	}

	escape_html = ngx_escape_html(NULL, r->uri.data, r->uri.len);

	len = r->uri.len + escape_html
		+ r->uri.len + escape_html +
		sizeof(to_lang) - 1
		+ sizeof(EN) - 1
		+ sizeof(to_stylesheet) - 1
		+ sizeof(BOOTSTRAPCDN) - 1
		+ sizeof(to_title) - 1
		+ sizeof(to_h1) - 1
		+ sizeof(to_table_body) - 1
		;


	entry = entries.elts;
	for (i = 0; i < entries.nelts; i++) {
		len += entry[i].name.len + entry[i].escape
			+ 1										/* 1 is for "/" */
			+ sizeof(to_td_href) - 1
			+ sizeof(TAG_END) - 1
			+ entry[i].name.len - entry[i].utf_len
			+ entry[i].escape_html
			+ NGX_HTTP_AUTOINDEX_NAME_LEN + sizeof("&gt;") - 2
			+ sizeof(to_td_date) - 1
			+ sizeof("28-Sep-1970 12:00") - 1
			+ sizeof(to_td_size) - 1
			+ 20									/* the file size */
			+ sizeof(end_row) - 1
			;
		}

	len += sizeof(to_list) - 1;

	for (i = 0; i < entries.nelts; i++) {
		len += entry[i].name.len + entry[i].escape
			+ sizeof(to_item_href) - 1
			+ sizeof(TAG_END) - 1
			+ entry[i].name.len - entry[i].utf_len
			+ entry[i].escape_html
			+ NGX_HTTP_AUTOINDEX_NAME_LEN + sizeof("&gt;") - 2
			+ 1										/* 1 is for "/" */
			+ sizeof("\">") - 1
			+ sizeof(to_item_end) - 1
			;
		}


	len += sizeof(to_html_end) - 1;

	b = ngx_create_temp_buf(r->pool, len);
	if (b == NULL) {
		return NGX_ERROR;
	}

	if (entries.nelts > 1) {
		ngx_qsort(entry, (size_t) entries.nelts,
				  sizeof(ngx_http_bootstrapindex_entry_t),
				  ngx_http_bootstrapindex_cmp_entries);
	}

	b->last = ngx_cpymem(b->last, to_lang, sizeof(to_lang) - 1);

	/*The language attribute. This is eventually intended to be a
	*set by a directive.
	*/
	b->last = ngx_cpymem(b->last, EN, sizeof(EN) - 1);

	b->last = ngx_cpymem(b->last, to_stylesheet, sizeof(to_stylesheet) - 1);

	/* The URL to load the stylesheet from. This is also intended to be
	* a configuration directive. */
	b->last = ngx_cpymem(b->last, BOOTSTRAPCDN, sizeof(BOOTSTRAPCDN) - 1);

	b->last = ngx_cpymem(b->last, to_title, sizeof(to_title) - 1);

	if (escape_html) {
		b->last = (u_char *) ngx_escape_html(b->last, r->uri.data, r->uri.len);
		b->last = ngx_cpymem(b->last, to_h1, sizeof(to_h1) - 1);
		b->last = (u_char *) ngx_escape_html(b->last, r->uri.data, r->uri.len);
	} else {
		b->last = ngx_cpymem(b->last, r->uri.data, r->uri.len);
		b->last = ngx_cpymem(b->last, to_h1, sizeof(to_h1) - 1);
		b->last = ngx_cpymem(b->last, r->uri.data, r->uri.len);
	}

	b->last = ngx_cpymem(b->last, to_table_body, sizeof(to_table_body) - 1);

	tp = ngx_timeofday();

	for (i = 0; i < entries.nelts; i++) {

		b->last = ngx_cpymem(b->last, to_td_href, sizeof(to_td_href) - 1);

		ngx_http_bootstrapindex_cpy_uri(b, &entry[i]);

		b->last = ngx_cpymem(b->last, TAG_END, sizeof(TAG_END) - 1);

		ngx_http_bootstrapindex_cpy_name(b, &entry[i]);

		b->last = ngx_cpymem(b->last, to_td_date, sizeof(to_td_date) - 1);

		ngx_gmtime(entry[i].mtime + tp->gmtoff * 60 * alcf->localtime, &tm);


		b->last = ngx_sprintf(b->last, "%02d-%s-%d %02d:%02d ",
				tm.ngx_tm_mday,
				months[tm.ngx_tm_mon - 1],
				tm.ngx_tm_year,
				tm.ngx_tm_hour,
				tm.ngx_tm_min);

		b->last = ngx_cpymem(b->last, to_td_size, sizeof(to_td_size) - 1);

		ngx_http_bootstrapindex_cpy_size(b, &entry[i], alcf);

		b->last = ngx_cpymem(b->last, end_row, sizeof(end_row) - 1);
	}

	b->last = ngx_cpymem(b->last, to_list, sizeof(to_list) - 1);

	for (i = 0; i < entries.nelts; i++) {
		b->last = ngx_cpymem(b->last, to_item_href, sizeof(to_item_href) - 1);

		ngx_http_bootstrapindex_cpy_uri(b, &entry[i]);

		b->last = ngx_cpymem(b->last, TAG_END, sizeof(TAG_END) - 1);

		ngx_http_bootstrapindex_cpy_name(b, &entry[i]);

		b->last = ngx_cpymem(b->last, to_item_end, sizeof(to_item_end) - 1);

	}

	b->last = ngx_cpymem(b->last, to_html_end, sizeof(to_html_end) - 1);

	/* TODO: free temporary pool */

	if (r == r->main) {
		b->last_buf = 1;
	}

	b->last_in_chain = 1;

	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r, &out);
}


static int ngx_libc_cdecl
ngx_http_bootstrapindex_cmp_entries(const void *one, const void *two)
{
	ngx_http_bootstrapindex_entry_t *first = (ngx_http_bootstrapindex_entry_t *) one;
	ngx_http_bootstrapindex_entry_t *second = (ngx_http_bootstrapindex_entry_t *) two;

	if (first->dir && !second->dir) {
		/* move the directories to the start */
		return -1;
	}

	if (!first->dir && second->dir) {
		/* move the directories to the start */
		return 1;
	}

	return (int) ngx_strcmp(first->name.data, second->name.data);
}


static ngx_int_t
ngx_http_bootstrapindex_error(ngx_http_request_t *r, ngx_dir_t *dir, ngx_str_t *name)
{
	if (ngx_close_dir(dir) == NGX_ERROR) {
		ngx_log_error(NGX_LOG_ALERT, r->connection->log, ngx_errno,
					  ngx_close_dir_n " \"%V\" failed", name);
	}

	return r->header_sent ? NGX_ERROR : NGX_HTTP_INTERNAL_SERVER_ERROR;
}


static void *
ngx_http_bootstrapindex_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_bootstrapindex_loc_conf_t  *conf;

	conf = ngx_palloc(cf->pool, sizeof(ngx_http_bootstrapindex_loc_conf_t));
	if (conf == NULL) {
		return NULL;
	}

	conf->enable = NGX_CONF_UNSET;
	conf->localtime = NGX_CONF_UNSET;
	conf->exact_size = NGX_CONF_UNSET;

	return conf;
}


static char *
ngx_http_bootstrapindex_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
	ngx_http_bootstrapindex_loc_conf_t *prev = parent;
	ngx_http_bootstrapindex_loc_conf_t *conf = child;

	ngx_conf_merge_value(conf->enable, prev->enable, 0);
	ngx_conf_merge_value(conf->localtime, prev->localtime, 0);
	ngx_conf_merge_value(conf->exact_size, prev->exact_size, 1);

	return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_bootstrapindex_init(ngx_conf_t *cf)
{
	ngx_http_handler_pt		*h;
	ngx_http_core_main_conf_t  *cmcf;

	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

	h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
	if (h == NULL) {
		return NGX_ERROR;
	}

	*h = ngx_http_bootstrapindex_handler;

	return NGX_OK;
}


static void
ngx_http_bootstrapindex_cpy_uri(ngx_buf_t *b, ngx_http_bootstrapindex_entry_t *entry)
{
	if (entry->escape) {
		ngx_escape_uri(b->last, entry->name.data, entry->name.len, NGX_ESCAPE_URI_COMPONENT);
		b->last += entry->name.len + entry->escape;
	} else {
		b->last = ngx_cpymem(b->last, entry->name.data, entry->name.len);
	}

	if (entry->dir) {
		*b->last++ = '/';
	}
}


static void
ngx_http_bootstrapindex_cpy_name(ngx_buf_t *b, ngx_http_bootstrapindex_entry_t *entry) {

	size_t char_len;
	size_t len;
	u_char *last;

	len = entry->utf_len;

	if (entry->name.len != len) {

		if (len > NGX_HTTP_AUTOINDEX_NAME_LEN) {
			char_len = NGX_HTTP_AUTOINDEX_NAME_LEN - 3 + 1;
		} else {
			char_len = NGX_HTTP_AUTOINDEX_NAME_LEN + 1;
		}

		last = b->last;
		b->last = ngx_utf8_cpystrn(b->last, entry->name.data, char_len, entry->name.len + 1);

		if (entry->escape_html) {
			b->last = (u_char *) ngx_escape_html(last, entry->name.data, b->last - last);
		}

		last = b->last;

	}else {

		if (entry->escape_html) {

			if (len > NGX_HTTP_AUTOINDEX_NAME_LEN) {
				char_len = NGX_HTTP_AUTOINDEX_NAME_LEN - 3;
			} else {
				char_len = len;
			}

			b->last = (u_char *) ngx_escape_html(b->last, entry->name.data, char_len);
			last = b->last;

		} else {
			b->last = ngx_cpystrn(b->last, entry->name.data, NGX_HTTP_AUTOINDEX_NAME_LEN + 1);
			last = b->last - 3;
		}
	}

	if (len > NGX_HTTP_AUTOINDEX_NAME_LEN) {
		b->last = ngx_cpymem(last, "..&gt;", sizeof("..&gt;") - 1);
	} else {

		if (entry->dir && NGX_HTTP_AUTOINDEX_NAME_LEN - len > 0) {
			*b->last++ = '/';
			len++;
		}
		if (NGX_HTTP_AUTOINDEX_NAME_LEN - len > 0) {
			ngx_memset(b->last, ' ', NGX_HTTP_AUTOINDEX_NAME_LEN - len);
			b->last += NGX_HTTP_AUTOINDEX_NAME_LEN - len;
		}
	}
}


static void
ngx_http_bootstrapindex_cpy_size(ngx_buf_t *b, ngx_http_bootstrapindex_entry_t *entry,
		ngx_http_bootstrapindex_loc_conf_t  *alcf) {

	off_t length;
	u_char scale;
	ngx_int_t size;

	if (alcf->exact_size) {
		if (entry->dir) {
			b->last = ngx_cpymem(b->last,  "-", sizeof("-") - 1);
		} else {
			b->last = ngx_sprintf(b->last, "%19O", entry->size);
		}
	} else {
		if (entry->dir) {
			b->last = ngx_cpymem(b->last,  "-", sizeof("-") - 1);
		} else {
			length = entry->size;
			if (length > 1024 * 1024 * 1024 - 1) {
				size = (ngx_int_t) (length / (1024 * 1024 * 1024));
				if ((length % (1024 * 1024 * 1024)) > (1024 * 1024 * 1024 / 2 - 1)) {
					size++;
				}
				scale = 'G';
			} else if (length > 1024 * 1024 - 1) {
				size = (ngx_int_t) (length / (1024 * 1024));
				if ((length % (1024 * 1024)) > (1024 * 1024 / 2 - 1)) {
					size++;
				}
				scale = 'M';
			} else if (length > 9999) {
				size = (ngx_int_t) (length / 1024);
				if (length % 1024 > 511) {
					size++;
				}
				scale = 'K';
			} else {
				size = (ngx_int_t) length;
				scale = '\0';
			} if (scale) {
				b->last = ngx_sprintf(b->last, "%6i%c", size, scale);
			} else {
				b->last = ngx_sprintf(b->last, "%6i", size);
			}
		}
	}
}
